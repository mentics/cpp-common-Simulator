#pragma once

#include "concurrentqueue.h"
#include "MenticsCommon.h"
#include "PriorityQueue.h"
#include "EventBases.h"
#include "SchedulerModel.h"
#include "../src/SchedulerModel.cpp" 

namespace chrono = std::chrono;

namespace MenticsGame {

template <typename Model, typename TimeType> struct Event;
PTRS2(Event, Model, TimeType)

template <typename Model, typename TimeType>
class Schedulator {
public:
	virtual void schedule(TimeType afterDuration, EventUniquePtr<Model, TimeType>&& ev) = 0;
	virtual void addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) = 0;
};
PTRS2(Schedulator, Model, TimeType)


template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType now() = 0;
	virtual chrono::nanoseconds realTimeUntil(TimeType t) = 0;
};
PTRS1(SchedulerTimeProvider, TimeType)


template <typename Model, typename TimeType>
struct Event {
	// NOTE: these should be const, but they're set by scheduler inside schedule() method
	TimeType created;
	TimeType timeToRun;

	//Event(const TimeType created, const TimeType timeToRun) : created(created), timeToRun(timeToRun) {
	//}

	static bool compare(const EventUniquePtr<Model, TimeType>& ev1, const EventUniquePtr<Model, TimeType>& ev2) 
	{
		return ev1->timeToRun > ev2->timeToRun;
	}

	virtual void run(SchedulatorPtr<Model, TimeType> sched, nn::nn<Model*> model) = 0;
};


template <typename Model, typename TimeType>
class EventZero : public Event<Model, TimeType> {
public:
	EventZero() : Event() {}
	void run(SchedulatorPtr<Model, TimeType> sched, nn::nn<Model*> model) {};
};


template <typename Model, typename TimeType>
class Scheduler : public Schedulator<Model, TimeType> {
	SchedulerModelPtr<Model, TimeType> schedModel;
	SchedulerTimeProviderPtr<TimeType> timeProvider;
	nn::nn<Model*> model;

	std::thread theThread;
	bool shouldStop = false;
	//std::unique_lock<std::mutex> lock;
	std::mutex mtx;
	std::condition_variable wait;

	TimeType processedTime;

public:
	friend class SchedulerTest;  
	Scheduler(SchedulerModelPtr<Model, TimeType> schedModel, SchedulerTimeProviderPtr<TimeType> timeProvider, nn::nn<Model*> model)
			: schedModel(schedModel), timeProvider(timeProvider), model(model),
			  theThread(&Scheduler<Model, TimeType>::run, this),
			  processedTime(0) {}
	

	~Scheduler() {
		mlog->error("Scheduler destructor");
		stop();
	}

	void run();

	void schedule(TimeType afterDuration, EventUniquePtr<Model, TimeType>&& ev);

	void wakeUp() {
		wait.notify_all();
	}

	void reset(TimeType resetToTime);

	void stop();

	// Only used for testing. TODO: untested
	void WaitUntilProcessed(TimeType until) {
		do {
			std::this_thread::sleep_for(chrono::microseconds(100)); 
		} while (processedTime < until && !schedModel->processing.empty());
	}

	TimeType getPT() { return processedTime; }
	void addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) { schedModel->addOutEvent(std::move(outEvent)); }
	void consumeOutgoing(std::function<void(OutEventPtr<TimeType>)> handler, TimeType upToTime) { schedModel->consumeOutgoing(handler, upToTime); }
};
PTRS2(Scheduler, Model, TimeType)


}
