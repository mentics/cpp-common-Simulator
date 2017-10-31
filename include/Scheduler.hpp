//#include "stdafx.h"

#include "Scheduler.h"

namespace MenticsGame {

using namespace std::chrono_literals;
namespace src = boost::log::sources;
namespace lvl = boost::log::trivial;


template <typename TimeType, typename Model>
TimeType SchedulerModel<TimeType,Model>::processIncoming() {
	LOG(lvl::trace) << "SchedulerModel::processIncoming";
	// TODO: assert scheduler thread?
	TimeType minTime = FOREVER;
	EventUniquePtr<TimeType,Model> ev = uniquePtr<EventZero<TimeType,Model>>();
	while (incoming.try_dequeue(ev)) {
		TimeType evTime = ev->timeToRun;
		if (evTime < minTime) {
			minTime = evTime;
		}
		processing.push(std::move(ev));
	}
	//std::sort(processingQueue.begin(), processingQueue.end(), &Event<TimeType>::compare);
	return minTime;
}


template <typename TimeType, typename Model>
void SchedulerModel<TimeType,Model>::reset(TimeType toTime) {
	// TODO
	// TODO: delete items off outgoing > toTime
}


template <typename TimeType, typename Model>
Event<TimeType,Model>* SchedulerModel<TimeType,Model>::first(TimeType maxTime) {
	if (!processing.empty()) {
		Event<TimeType,Model>* top = processing.top().get();
		const TimeType nextTime = top->timeToRun;
		if (nextTime <= maxTime) {
			return top;
		}
	}
	return nullptr;
}


template <typename TimeType, typename Model>
void SchedulerModel<TimeType,Model>::completeFirst() {
	forReset.push_back(processing.pop());
}


template <typename TimeType, typename Model>
void SchedulerModel<TimeType,Model>::consumeOutgoing(TimeType upToTime, std::function<void(OutEventPtr<TimeType>)> handler) {
	while (!outgoing.empty()) {
		OutEventPtr<TimeType> ev = NN_CHECK_ASSERT(outgoing.front().get());
		if (ev->occursAt <= upToTime) {
			handler(ev);
			outgoing.pop_front();
			// Finally after travelling through 3 queues, the event's eventful life has come to an end.
			// delete ev; <- it's deleted by unique_ptr
		} else {
			break;
		}
	}
}



template <typename TimeType, typename Model>
void Scheduler<TimeType,Model>::run() {
	LOG(lvl::trace) << "Scheduler::run";

	while (true) {
		TimeType nextTime = 0;
		TimeType maxTime = 0;
		do {
			TimeType minTimeToRun = schedModel->processIncoming();
			if (minTimeToRun < processedTime) {
				schedModel->reset(minTimeToRun);
			}
			maxTime = processedTime + timeProvider->maxTimeAhead();
			Event<TimeType,Model>* ev = schedModel->first(maxTime);
			if (ev != NULL) {
				nextTime = ev->timeToRun;
				if (nextTime < processedTime) {
					LOG(lvl::error) << "Back in time processing";
				}
				processedTime = nextTime;
				ev->run(schedModel, model);
				schedModel->completeFirst();
				// TODO: memory leak: do we delete the event? what if it rescheduled itself?
				// problem is: if it rescheduled but we need to have it consumed by front end?
			} else {
				nextTime = FOREVER;
			}
		} while (nextTime < maxTime);

		if (!shouldStop) {
			TimeType sleepTime = timeProvider->realTimeUntil(nextTime);
			LOG(boost::log::trivial::trace) << "Sleeping for " << sleepTime;
			{
				std::unique_lock<std::mutex> lock(mtx);
				wait.wait_for(lock, 1000000s + sleepTime * 1ms);
			}
		}
		else {
			break;
		}
		LOG(boost::log::trivial::error) << "looping again...";
	}
}


template <typename TimeType, typename Model>
void Scheduler<TimeType,Model>::stop() {
	shouldStop = true;
	LOG(boost::log::trivial::error) << "notifying...";
	wait.notify_all();
	if (theThread.joinable()) {
		LOG(boost::log::trivial::error) << "joining...";
		theThread.join();
	}
}

}
