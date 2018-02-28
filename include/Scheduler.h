#pragma once

#include "concurrentqueue.h"

#include "MenticsCommonTest.h"
#include "MenticsCommon.h"
#include "PriorityQueue.h"

namespace chrono = std::chrono;

namespace MenticsGame {

template <typename Model, typename TimeType = TimePoint> struct Event;
PTRS2(Event, TimeType, Model)
template <typename TimeType = TimePoint> struct OutEvent;
PTRS1(OutEvent, TimeType)


template <typename Model, typename TimeType = TimePoint>
class Schedulator {
public:
	virtual void schedule(EventUniquePtr<Model, TimeType> ev) = 0;
	virtual void addOutEvent(OutEventUniquePtr<TimeType> outEvent) = 0;
};
PTRS2(Schedulator, Model, TimeType)


template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType now() = 0;
	virtual TimeType maxTimeAhead() = 0;
	virtual chrono::nanoseconds realTimeUntil(TimeType t) = 0;
};
PTRS1(SchedulerTimeProvider, TimeType)


template <typename Model, typename TimeType = TimePoint>
struct Event {
	const TimeType created;
	const TimeType timeToRun;

	Event(const TimeType created, const TimeType timeToRun) : created(created), timeToRun(timeToRun) {
	}

	static bool compare(const EventUniquePtr<Model,TimeType>& ev1, const EventUniquePtr<Model, TimeType>& ev2) {
		//return ev1->timeToRun > ev2->timeToRun;
		return true;
	}

	virtual void run(SchedulatorPtr<Model, TimeType> sched, nn::nn<Model*> model) = 0;
};

template <typename Model, typename TimeType = TimePoint>
class EventZero : public Event<Model, TimeType> {
public:
	EventZero() : Event(0, 0) {}
	void run(SchedulatorPtr<Model, TimeType> sched, nn::nn<Model*> model) {};
};


template <typename TimeType = TimePoint>
struct OutEvent {
	const TimeType occursAt;

	OutEvent(const TimeType occursAt) : created(occursAt) {}
};


template <typename Model, typename TimeType = TimePoint>
class SchedulerModel : public Schedulator<Model, TimeType> {
private:
	moodycamel::ConcurrentQueue<EventUniquePtr<Model, TimeType>> incoming;
	PriorityQueue<EventUniquePtr<Model, TimeType>, decltype(&Event<Model, TimeType>::compare)> processing;
	std::deque<EventUniquePtr<Model, TimeType>> forReset;
	std::deque<OutEventUniquePtr<TimeType>> outgoing;

public:
	void schedule(EventUniquePtr<TimeType, Model> ev) {};
	
	
	SchedulerModel(std::string name) : 
		incoming(1024), processing(&Event<Model, TimeType>::compare) {}
	~SchedulerModel() {
		log->error("SchedulerModel destructor");
	}

	

	// Runs on outside thread
	void schedule(EventUniquePtr<Model, TimeType> ev) {
		log->trace("Scheduling event");
		incoming.enqueue(std::move(ev));
	}

	// Returns minimum timeToRun of ingested events
	TimeType processIncoming();
	void reset(TimeType toTime);
	Event<Model, TimeType>* first(TimeType maxTime);
	void completeFirst();

	void consumeOutgoing(std::function<void(OutEventPtr<TimeType>)> handler, TimeType upToTime);

	void addOutEvent(OutEventUniquePtr<TimeType> outEvent) {
		outgoing.push_back(std::move(outEvent));
	}
};
PTRS2(SchedulerModel, Model, TimeType)


template <typename Model, typename TimeType = TimePoint>
class Scheduler  {
public:
	Scheduler(std::string name, SchedulerModelPtr<Model, TimeType> schedModel, SchedulerTimeProviderPtr<TimeType> timeProvider, nn::nn<Model*> model) :
		schedModel(schedModel), timeProvider(timeProvider), model(model),
		theThread(&Scheduler<Model, TimeType>::run, this),
		processedTime(0) {}

	~Scheduler() {
		log->error("Scheduler destructor");
		stop();
	}

	void run();

	void schedule(EventUniquePtr<TimeType,Model> ev) {
		schedModel->schedule(std::move(ev));
		log->trace("notifying...");
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
	SchedulerModelPtr<Model, TimeType> schedModel;
	SchedulerTimeProviderPtr<TimeType> timeProvider;
	nn::nn<Model*> model;

	std::thread theThread;
	bool shouldStop = false;
	//std::unique_lock<std::mutex> lock;
	std::mutex mtx;
	std::condition_variable wait;

	TimeType processedTime;
};
PTRS2(Scheduler, Model, TimeType)

}
