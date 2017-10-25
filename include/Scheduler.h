#pragma once

#include <boost/lockfree/queue.hpp>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "MenticsCommon.h"

namespace mentics { namespace scheduler {

namespace cmn = mentics::common;

template <typename TimeType>
TimeType FOREVER();

template <typename TimeType>
class Event;

template <typename TimeType>
class Schedulator {
public:
	virtual void schedule(Event<TimeType>* ev) = 0;
};

template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType now() = 0;
	virtual TimeType maxTimeAhead() = 0;
	virtual TimeType realTimeUntil(TimeType t) = 0;
};


template <typename TimeType>
class Event {
public:
	const TimeType timeToRun;

	Event(TimeType timeToRun) : timeToRun(timeToRun) {
	}

	static bool compare(Event<TimeType>* ev1, Event<TimeType>* ev2) {
		return ev1->timeToRun > ev2->timeToRun;
	}

//	virtual TimeType timeToRun() = 0;
	virtual void run(Schedulator<TimeType>* sched) = 0;
};

template <typename TimeType>
class SchedulerModel : public cmn::CanLog {
private:
	boost::lockfree::queue<Event<TimeType>*> incoming;
	std::priority_queue<Event<TimeType>*, std::vector<Event<TimeType>*>, decltype(&Event<TimeType>::compare)> processing;
	std::deque<Event<TimeType>*> outgoing;

public:
	SchedulerModel(std::string name) : CanLog(name),
		incoming(1024), processing(&Event<TimeType>::compare) {}
	~SchedulerModel() {
		LOG(boost::log::trivial::error) << "SchedulerModel destructor";
	}

	// Runs on outside thread
	void schedule(Event<TimeType>* ev) {
		LOG(boost::log::trivial::trace) << "Scheduling event";
		incoming.push(ev);
	}

	// Returns minimum timeToRun of ingested events
	TimeType processIncoming();
	void reset(TimeType toTime);
	Event<TimeType>* first(TimeType maxTime);
	void completeFirst();
	void consumeOutgoing(TimeType upToTime, std::function<void(Event<TimeType>*)> handler);
};


template <typename TimeType>
class Scheduler : public cmn::CanLog, public Schedulator<TimeType> {
private:
	SchedulerModel<TimeType>* model;
	SchedulerTimeProvider<TimeType>* timeProvider;

	std::thread theThread;
	bool shouldStop = false;
	//std::unique_lock<std::mutex> lock;
	std::mutex mtx;
	std::condition_variable wait;

	TimeType processedTime;

public:
	Scheduler(std::string name, SchedulerModel<TimeType>* model, SchedulerTimeProvider<TimeType>* timeProvider) :
		CanLog(name),
		model(model), timeProvider(timeProvider),
		theThread(&Scheduler<TimeType>::run, this),
		processedTime(0) {}

	~Scheduler() {
		LOG(boost::log::trivial::error) << "Scheduler destructor";
	}

	void run();

	void schedule(Event<TimeType>* ev) {
		model->schedule(ev);
		LOG(boost::log::trivial::trace) << "notifying...";
		wait.notify_all(); // Wake up in case we're asleep
	}

	void stop();
};


}}