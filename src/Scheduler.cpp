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

template <typename TimeType>
void SchedulerModel<TimeType>::processIncoming() {
	BOOST_LOG_SEV(lg, level::trace) << "SchedulerModel::processIncoming";
	// TODO: assert scheduler thread?
	while (!incomingQueue.empty()) {
		//std::unique_ptr<Event<TimeType>> ev;
		Event<TimeType>* ev;
		incomingQueue.pop(ev);
		//processingQueue.push_back(ev);
		processingQueue.push(ev);
	}
	//std::sort(processingQueue.begin(), processingQueue.end(), &Event<TimeType>::compare);
}

template <typename TimeType>
TimeType SchedulerModel<TimeType>::processFirst(TimeType maxTime) {
	BOOST_LOG_SEV(lg, level::trace) << "SchedulerModel::processFirst";
	// TODO: assert scheduler thread?

	if (!processingQueue.empty()) {
		Event<TimeType>* top = processingQueue.top();
		TimeType nextRunTime = top->timeToRun();
		if (nextRunTime <= maxTime) {
			processingQueue.pop();
			top->run(this);
		}
	}

	return 0;
}

template <typename TimeType>
void Scheduler<TimeType>::run() {
	BOOST_LOG_SEV(lg, boost::log::trivial::trace) << "Scheduler::run";

	while (true) {
		TimeType nextTime;
		TimeType maxTime;
		do {
			maxTime = timeProvider->maxTime();
			model->processIncoming();
			nextTime = model->processFirst(maxTime);
			if (nextTime < processedTime) {
				// TODO: Error: back in time processing
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
