#include "stdafx.h"
#include "Scheduler.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace mentics { namespace scheduler {


using namespace std::chrono_literals;
namespace src = boost::log::sources;
namespace level = boost::log::trivial;

template<>
uint32_t FOREVER<uint32_t>() {
	return std::numeric_limits<uint32_t>::max();
}


template <typename TimeType>
TimeType SchedulerModel<TimeType>::processIncoming() {
	BOOST_LOG_SEV(lg, level::trace) << "SchedulerModel::processIncoming";
	// TODO: assert scheduler thread?
	TimeType minTime = FOREVER<TimeType>();
	while (!incoming.empty()) {
		Event<TimeType>* ev;
		incoming.pop(ev);
		TimeType evTime = ev->timeToRun();
		if (evTime < minTime) {
			minTime = evTime;
		}
		processing.push(ev);
	}
	//std::sort(processingQueue.begin(), processingQueue.end(), &Event<TimeType>::compare);
	return minTime;
}


template <typename TimeType>
void SchedulerModel<TimeType>::reset(TimeType toTime) {
	// TODO
	// TODO: delete items off outgoing > toTime
}


template <typename TimeType>
Event<TimeType>* SchedulerModel<TimeType>::first(TimeType maxTime) {
	if (!processing.empty()) {
		Event<TimeType>* top = processing.top();
		TimeType nextTime = top->timeToRun();
		if (nextTime <= maxTime) {
			return top;
		}
	}
	return NULL;
}


template <typename TimeType>
void SchedulerModel<TimeType>::completeFirst() {
	outgoing.push_back(processing.top());
	processing.pop();
}


template <typename TimeType>
void SchedulerModel<TimeType>::consumeOutgoing(TimeType upToTime, std::function<void(Event<TimeType>*)> handler) {
	while (!outgoing.empty()) {
		Event<TimeType>* ev = outgoing.front();
		if (ev->timeToRun() <= upToTime) {
			handler(ev);
			outgoing.pop_front();
			// Finally after travelling through 3 queues, the event's eventful life has come to an end.
			delete ev;
		} else {
			break;
		}
	}
}



template <typename TimeType>
void Scheduler<TimeType>::run() {
	BOOST_LOG_SEV(lg, level::trace) << "Scheduler::run";

	while (true) {
		TimeType nextTime;
		TimeType maxTime;
		do {
			TimeType minTimeToRun = model->processIncoming();
			if (minTimeToRun < processedTime) {
				model->reset(minTimeToRun);
			}
			maxTime = processedTime + timeProvider->maxTimeAhead();
			Event<TimeType>* ev = model->first(maxTime);
			if (ev != NULL) {
				nextTime = ev->timeToRun();
				if (nextTime < processedTime) {
					BOOST_LOG_SEV(lg, level::error) << "Back in time processing";
				}
				processedTime = nextTime;
				ev->run(model);
				model->completeFirst();
				// TODO: memory leak: do we delete the event? what if it rescheduled itself?
				// problem is: if it rescheduled but we need to have it consumed by front end?
			} else {
				nextTime = FOREVER<TimeType>();
			}
		} while (nextTime < maxTime);

		if (!shouldStop) {
			TimeType sleepTime = timeProvider->realTimeUntil(nextTime);
			BOOST_LOG_SEV(lg, boost::log::trivial::trace) << "Sleeping for " << sleepTime;
			{
				std::unique_lock<std::mutex> lock(mtx);
				wait.wait_for(lock, 1000000s + sleepTime * 1ms);
			}
		}
		else {
			break;
		}
		BOOST_LOG_SEV(lg, boost::log::trivial::error) << "looping again...";
	}
}


template <typename TimeType>
void Scheduler<TimeType>::stop() {
	shouldStop = true;
	BOOST_LOG_SEV(lg, boost::log::trivial::error) << "notifying...";
	wait.notify_all();
	BOOST_LOG_SEV(lg, boost::log::trivial::error) << "joining...";
	theThread.join();
}


template class SchedulerModel<uint32_t>;
template class Scheduler<uint32_t>;


}}
