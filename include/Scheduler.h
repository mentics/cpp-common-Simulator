#pragma once

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
//#include <boost/heap/priority_queue.hpp>
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>


namespace mentics { namespace scheduler {

template <typename TimeType>
TimeType FOREVER();

template <typename TimeType>
class Event;

template <typename TimeType>
class Schedulator : public CanLog {
public:
	virtual void schedule(Event<TimeType>* ev) = 0;
};

class CanLog {
public:
	boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg;
};

template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType maxTimeAhead() = 0;
	virtual TimeType realTimeUntil(TimeType t) = 0;
};


template <typename TimeType>
class Event {
public:
	static bool compare(Event<TimeType>* ev1, Event<TimeType>* ev2) {
		return ev1->timeToRun() > ev2->timeToRun();
	}

	virtual TimeType timeToRun() = 0;
	virtual void run(Schedulator<TimeType>* sched) = 0;
};

template <typename TimeType>
class SchedulerModel : public Schedulator<TimeType> {
private:
	boost::lockfree::queue<Event<TimeType>*> incoming;
	std::priority_queue<Event<TimeType>*, std::vector<Event<TimeType>*>, decltype(&Event<TimeType>::compare)> processing;
	std::deque<Event<TimeType>*> outgoing;

public:
	SchedulerModel(TimeType forever) :
		incoming(1024), processing(&Event<TimeType>::compare) {}

	// Runs on outside thread
	void schedule(Event<TimeType>* ev) {
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
class Scheduler : public CanLog {
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
	Scheduler(SchedulerModel<TimeType>* model, SchedulerTimeProvider<TimeType>* timeProvider) :
		model(model), timeProvider(timeProvider),
		theThread(&Scheduler<TimeType>::run, this),
		processedTime(0) {}

	void run();

	void stop();
};


}}