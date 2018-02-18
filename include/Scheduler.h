#pragma once

#include "concurrentqueue.h"


#include "MenticsCommon.h"
#include "PriorityQueue.h"

namespace chrono = std::chrono;

namespace MenticsGame {

template <typename TimeType, typename Model> struct Event;
PTRS2(Event, TimeType, Model)
template <typename TimeType> struct OutEvent;
PTRS1(OutEvent, TimeType)


template <typename TimeType, typename Model>
class Schedulator {
public:
	virtual void schedule(EventUniquePtr<TimeType, Model> ev) = 0;
	virtual void addOutEvent(OutEventUniquePtr<TimeType> outEvent) = 0;
};
PTRS2(Schedulator, TimeType, Model)


template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType now() = 0;
	virtual TimeType maxTimeAhead() = 0;
	virtual chrono::nanoseconds realTimeUntil(TimeType t) = 0;
};
PTRS1(SchedulerTimeProvider, TimeType)


template <typename TimeType, typename Model>
struct Event {
	const TimeType created;
	const TimeType timeToRun;

	Event(const TimeType created, const TimeType timeToRun) : created(created), timeToRun(timeToRun) {
	}

	static bool compare(const EventUniquePtr<TimeType,Model>& ev1, const EventUniquePtr<TimeType,Model>& ev2) {
		return ev1->timeToRun > ev2->timeToRun;
	}

	virtual void run(SchedulatorPtr<TimeType,Model> sched, nn::nn<Model*> model) = 0;
};

template <typename TimeType, typename Model>
class EventZero : public Event<TimeType,Model> {
public:
	EventZero() : Event(0, 0) {}
	void run(SchedulatorPtr<TimeType,Model> sched, nn::nn<Model*> model) {};
};


template <typename TimeType>
struct OutEvent {
	const TimeType occursAt;

	OutEvent(const TimeType occursAt) : created(occursAt) {}
};


template <typename TimeType, typename Model>
class SchedulerModel : public Schedulator<TimeType, Model> {
private:
	moodycamel::ConcurrentQueue<EventUniquePtr<TimeType,Model>> incoming;
	PriorityQueue<EventUniquePtr<TimeType,Model>, decltype(&Event<TimeType,Model>::compare)> processing;
	std::deque<EventUniquePtr<TimeType,Model>> forReset;
	std::deque<OutEventUniquePtr<TimeType>> outgoing;

public:
	
	SchedulerModel(std::string name) : 
		incoming(1024), processing(&Event<TimeType,Model>::compare) {}
	~SchedulerModel() {
		m_log->error("SchedulerModel destructor");
	}

	// Runs on outside thread
	void schedule(EventUniquePtr<TimeType,Model> ev) {
		m_log->trace("Scheduling event");
		incoming.enqueue(std::move(ev));
	}

	// Returns minimum timeToRun of ingested events
	TimeType processIncoming();
	void reset(TimeType toTime);
	Event<TimeType,Model>* first(TimeType maxTime);
	void completeFirst();

	void consumeOutgoing(TimeType upToTime, std::function<void(OutEventPtr<TimeType>)> handler);

	void addOutEvent(OutEventUniquePtr<TimeType> outEvent) {
		outgoing.push_back(std::move(outEvent));
	}
};
PTRS2(SchedulerModel, TimeType, Model)


template <typename TimeType, typename Model>
class Scheduler  {
public:
	Scheduler(std::string name, SchedulerModelPtr<TimeType,Model> schedModel, SchedulerTimeProviderPtr<TimeType> timeProvider, nn::nn<Model*> model) :
		schedModel(schedModel), timeProvider(timeProvider), model(model),
		theThread(&Scheduler<TimeType,Model>::run, this),
		processedTime(0) {}

	~Scheduler() {
		m_log->error("Scheduler destructor");
		stop();
	}

	void run();

	void schedule(EventUniquePtr<TimeType,Model> ev) {
		schedModel->schedule(std::move(ev));
		m_log->trace("notifying...");
		wakeUp();
	}

	void wakeUp() {
		wait.notify_all();
	}

	void reset(TimeType resetToTime);

	void stop();

	// Only used for testing. TODO: untested
	void WaitUntilProcessed(TimeType until) {
		do {
			Thread.Sleep(100);
		} while (processedTime < until);
	}

private:
	SchedulerModelPtr<TimeType, Model> schedModel;
	SchedulerTimeProviderPtr<TimeType> timeProvider;
	nn::nn<Model*> model;

	std::thread theThread;
	bool shouldStop = false;
	//std::unique_lock<std::mutex> lock;
	std::mutex mtx;
	std::condition_variable wait;

	TimeType processedTime;
};
PTRS2(Scheduler, TimeType, Model)

}
