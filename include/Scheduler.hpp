//#include "stdafx.h"

#include "Scheduler.h"
#include <spdlog\spdlog.h>


using namespace std::chrono_literals;

namespace MenticsGame {


template <typename Model, typename TimeType = TimePoint>
TimeType SchedulerModel<Model, TimeType>::processIncoming() {
	log->trace("SchedulerModel::processIncoming");
	// TODO: assert scheduler thread?
	TimeType minTime = FOREVER;
	EventUniquePtr<Model, TimeType> ev = uniquePtr<EventZero<Model, TimeType>>(); // TODO: simplify this?
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


template <typename Model, typename TimeType = TimePoint>
void SchedulerModel<Model, TimeType>::reset(TimeType toTime) {
	// TODO
	// TODO: delete items off outgoing > toTime
}


template <typename Model, typename TimeType = TimePoint>
Event<Model, TimeType>* SchedulerModel<Model, TimeType>::first(TimeType maxTime) {
	if (!processing.empty()) {
		Event<Model, TimeType>* top = processing.top().get();
		const TimeType nextTime = top->timeToRun;
		if (nextTime <= maxTime) {
			return top;
		}
	}
	return nullptr;
}


template <typename Model, typename TimeType = TimePoint>
void SchedulerModel<Model, TimeType>::completeFirst() {
	forReset.push_back(processing.pop());
}


template <typename Model, typename TimeType = TimePoint>
void SchedulerModel<Model, TimeType>::consumeOutgoing(std::function<void(OutEventPtr<TimeType>)> handler, TimeType upToTime ) {
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



// This is the method run by the Scheduler thread.
// It loops forever and processes the events on the processing queue up to 
template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model, TimeType>::run() {
	log->trace("Scheduler::run");

	while (true) {
		TimeType nextTime = 0;
		TimeType maxTime = 0;
		do {
			TimeType minTimeToRun = schedModel->processIncoming();
			if (minTimeToRun < processedTime) {
				reset(minTimeToRun);
			}
			TimeType now = timeProvider->now();
			maxTime = now + timeProvider->maxTimeAhead();
			Event<Model, TimeType>* ev = schedModel->first(maxTime);
			if (ev != NULL) {
				nextTime = ev->timeToRun;
				if (nextTime < now) {
					log->warn("Event time prior to now, event processing can't keep up.");
				}
				if (nextTime < processedTime) {
					log->error("Back in time processing");
				}
				processedTime = nextTime;
				ev->run(schedModel, model);
				schedModel->completeFirst();
			} else {
				nextTime = FOREVER;
			}
		} while (nextTime < maxTime);

		if (!shouldStop) {
			const chrono::nanoseconds sleepTime = timeProvider->realTimeUntil(nextTime);
			log->trace("Sleeping for {0} ns", sleepTime.count());
			{
				std::unique_lock<std::mutex> lock(mtx);
				wait.wait_for(lock, sleepTime);
			}
		}
		else {
			break;
		}
		log->error("looping again...");
	}
}

template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model, TimeType>::reset(TimeType resetToTime) {
	// TODO: reset queues to what they were at resetToTime
	schedModel->reset(resetToTime);
	model->reset(resetToTime);
}


template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model,TimeType>::stop() {
	shouldStop = true;
	log->error("notifying...");
	wait.notify_all();
	if (theThread.joinable()) {
		log->trace("joining...");
		theThread.join();
	}
}

}
