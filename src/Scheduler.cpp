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
	while (!incomingQueue.empty()) {
		Event<TimeType>* ev;
		incomingQueue.pop(ev);
		TimeType evTime = ev->timeToRun();
		if (evTime < minTime) {
			minTime = evTime;
		}
		processingQueue.push(ev);
	}
	//std::sort(processingQueue.begin(), processingQueue.end(), &Event<TimeType>::compare);
	return minTime;
}

template <typename TimeType>
Event<TimeType>* SchedulerModel<TimeType>::first(TimeType maxTime) {
	if (!processingQueue.empty()) {
		Event<TimeType>* top = processingQueue.top();
		TimeType nextTime = top->timeToRun();
		if (nextTime <= maxTime) {
			processingQueue.pop();
			return top;
		}
	}
	return NULL;
}

//template <typename TimeType>
//TimeType SchedulerModel<TimeType>::processFirst(TimeType maxTime) {
//	BOOST_LOG_SEV(lg, level::trace) << "SchedulerModel::processFirst";
//	// TODO: assert scheduler thread?
//
//	TimeType nextTime = FOREVER();
//	if (!processingQueue.empty()) {
//		Event<TimeType>* top = processingQueue.top();
//		nextTime = top->timeToRun();
//		if (nextTime < processedTime) {
//			// TODO: Error: back in time processing that should never happen
//		}
//		TimeType maxTime = processedTime + maxTimeAhead;
//		if (nextTime <= maxTime) {
//			processingQueue.pop();
//			processedTime = nextTime;
//			top->run(this);
//		}
//	}
//
//	return nextTime;
//}

template <typename TimeType>
void Scheduler<TimeType>::run() {
	BOOST_LOG_SEV(lg, level::trace) << "Scheduler::run";

	while (true) {
		TimeType nextTime;
		TimeType maxTime;
		do {
			TimeType minTimeToRun = model->processIncoming();
			if (minTimeToRun < processedTime) {
				// TODO: reset time to minTimeToRun
			}
			maxTime = processedTime + timeProvider->maxTimeAhead();
			Event<TimeType>* ev = model->first(maxTime);
			// TODO: what's next time if it's null? need to wait until then
			if (ev != NULL) {
				nextTime = ev->timeToRun();
				if (nextTime < processedTime) {
					BOOST_LOG_SEV(lg, level::error) << "Back in time processing";
				}
				processedTime = nextTime;
				ev->run(model);
				// TODO: memory leak: do we delete the event? what if it rescheduled itself?
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
