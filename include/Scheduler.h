#pragma once

#include "concurrentqueue.h"
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "MenticsCommon.h"
#include "PriorityQueue.h"

namespace mentics { namespace scheduler {

template <typename TimeType>
TimeType FOREVER();

template <typename TimeType>
class Event;

template <typename TimeType>
//using EventUniquePtr = std::unique_ptr<Event<TimeType>>;
using EventUniquePtr = nn::nn<std::unique_ptr<Event<TimeType>>>;

template <typename TimeType>
using EventPtr = nn::nn<Event<TimeType>*>;


template <typename TimeType>
class Schedulator {
public:
	virtual void schedule(EventUniquePtr<TimeType> ev) = 0;
};


template <typename TimeType>
struct SchedulerTimeProvider {
	virtual TimeType now() = 0;
	virtual TimeType maxTimeAhead() = 0;
	virtual TimeType realTimeUntil(TimeType t) = 0;
};


template <typename TimeType>
struct Event {
	const TimeType created;
	const TimeType timeToRun;

	Event(const TimeType created, const TimeType timeToRun) : created(created), timeToRun(timeToRun) {
	}

	static bool compare(const EventUniquePtr<TimeType>& ev1, const EventUniquePtr<TimeType>& ev2) {
		return ev1->timeToRun > ev2->timeToRun;
	}

	virtual void run(Schedulator<TimeType>* sched) = 0;
};
template <typename TimeType>
class EventZero : public Event<TimeType> {
public:
	EventZero() : Event(0, 0) {}
	virtual void run(Schedulator<TimeType>* sched) {};
};


template <typename TimeType>
struct OutEvent {
	const TimeType occursAt;

	OutEvent(const TimeType occursAt) : created(occursAt) {
	}
};
template <typename TimeType>
using OutEventUniquePtr = std::unique_ptr<OutEvent<TimeType>>;
//using OutEventUniquePtr = nn::nn<std::unique_ptr<OutEvent<TimeType>>>;


template <typename TimeType>
class SchedulerModel : public cmn::CanLog {
private:
//	boost::lockfree::queue<EventUniquePtr<TimeType>> incoming;
	moodycamel::ConcurrentQueue<EventUniquePtr<TimeType>> incoming;
	//std::priority_queue<EventUniquePtr<TimeType>, std::vector<EventUniquePtr<TimeType>>, decltype(&Event<TimeType>::compare)> processing;
	PriorityQueue<EventUniquePtr<TimeType>, decltype(&Event<TimeType>::compare)> processing;
	std::deque<EventUniquePtr<TimeType>> forReset;
	std::deque<OutEventUniquePtr<TimeType>> outgoing;

public:
	SchedulerModel(std::string name) : CanLog(name),
		incoming(1024), processing(&Event<TimeType>::compare) {}
	~SchedulerModel() {
		LOG(boost::log::trivial::error) << "SchedulerModel destructor";
	}

	// Runs on outside thread
	//void schedule(gsl::not_null<std::unique_ptr<Event<TimeType>> ev) {
	void schedule(EventUniquePtr<TimeType> ev) {
		LOG(boost::log::trivial::trace) << "Scheduling event";
		incoming.enqueue(std::move(ev));
	}

	// Returns minimum timeToRun of ingested events
	TimeType processIncoming();
	void reset(TimeType toTime);
	Event<TimeType>* first(TimeType maxTime);
	void completeFirst();
	void consumeOutgoing(TimeType upToTime, std::function<void(OutEvent<TimeType>*)> handler);
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
		stop();
	}

	void run();

	void schedule(EventUniquePtr<TimeType> ev) {
		model->schedule(std::move(ev));
		LOG(boost::log::trivial::trace) << "notifying...";
		wakeUp();
	}

	void wakeUp() {
		wait.notify_all();
	}

	void stop();
};


}}

namespace sched = mentics::scheduler;
