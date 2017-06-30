#pragma once

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
//#include <boost/heap/priority_queue.hpp>
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>


namespace mentics { namespace scheduler {


template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType maxTime() = 0;
	virtual TimeType realTimeUntil(TimeType t) = 0;
};


template <typename TimeType>
class Event {
public:
	virtual TimeType timeToRun() = 0;
};


//template <typename TimeType>
//class EventComparator
//{
//public:
//	bool operator() (const std::unique_ptr<Event<TimeType>>& e1, const std::unique_ptr<Event<TimeType>> e2)
//	{
//		return e1->timeToRun() < e2->timeToRun();
//	}
//};


template <typename TimeType>
class SchedulerModel {
private:
	boost::lockfree::queue<Event<TimeType>*> incomingQueue;
	std::vector<Event<TimeType>*> processingQueue;
	//boost::heap::priority_queue<std::unique_ptr<Event<TimeType>>, boost::heap::compare<EventComparator<TimeType>>> processingQueue;
	//std::vector<std::unique_ptr<Event<TimeType>>> processingQueue;

public:
	SchedulerModel() : incomingQueue(1024) {}

	// Runs on outside thread
	void schedule(Event<TimeType>* ev) {
		incomingQueue.push(ev);
	}

	void processIncoming();
	TimeType processFirst(TimeType maxTime);
};


template <typename TimeType>
class Scheduler {
private:
	boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg;

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
		//lock(std::mutex()),
		model(model), timeProvider(timeProvider),
		theThread(&Scheduler<TimeType>::run, this) {}

	void run();

	void stop();
};


}}