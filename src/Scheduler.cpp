#include "stdafx.h"
#include "Scheduler.h"

namespace MenticsGame
{

template <typename Model, typename TimeType>
void SchedulerModel<Model, TimeType>::schedule(EventUniquePtr<Model, TimeType>&& ev) {
	mlog->trace("Scheduling event");
	incoming.enqueue(std::move(ev));
}


template <typename Model, typename TimeType>
TimeType SchedulerModel<Model, TimeType>::processIncoming() {
	mlog->trace("SchedulerModel::processIncoming");
	// TODO: assert scheduler thread?
	EventUniquePtr<Model, TimeType> ev = uniquePtr<EventZero<Model, TimeType>>(); // TODO: simplify this?
	while (incoming.try_dequeue(ev)) {
		processing.push(std::move(ev));
	}
	return !processing.empty() ? processing.top()->timeToRun : FOREVER;
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
void SchedulerModel<Model, TimeType>::addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) {
	outgoing.push_back(std::move(outEvent));
}

template <typename Model, typename TimeType = TimePoint>
void SchedulerModel<Model, TimeType>::consumeOutgoing(std::function<void(OutEventPtr<TimeType>)> handler, TimeType upToTime) {
	while (!outgoing.empty()) {
		OutEventPtr<TimeType> ev = NN_CHECK_ASSERT(outgoing.front().get());
		if (ev->occursAt <= upToTime) {
			handler(ev);
			outgoing.pop_front();
			Signal::oldest = upToTime - maxTimeAhead();
			// Finally after travelling through 3 queues, the event's eventful life has come to an end.
			// delete ev; <- it's deleted by unique_ptr
		}
		else {
			break;
		}
	}
}



// This is the method run by the Scheduler thread.
// It loops forever and processes the events on the processing queue up to 
template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model, TimeType>::run() {
	mlog->trace("Scheduler::run");

	while (true) {
		TimeType nextTime = 0;
		TimeType maxTime = 0;
		do {
			TimeType minTimeToRun = schedModel->processIncoming();
			if (minTimeToRun < processedTime) {
				reset(minTimeToRun);
			}
			TimeType now = timeProvider->now();
			maxTime = now + schedModel->maxTimeAhead;
			Event<Model, TimeType>* ev = schedModel->first(maxTime);
			if (ev != NULL) {
				nextTime = ev->timeToRun;
				if (nextTime < now) {
					mlog->warn("Event time prior to now, event processing can't keep up.");
				}
				if (nextTime < processedTime) {
					mlog->error("Back in time processing");
				}
				processedTime = nextTime;
				ev->run(schedModel, model);
				schedModel->completeFirst();
			} else {
				nextTime = minTimeToRun;
			}
		} while (nextTime < maxTime);

		if (!shouldStop) {
			const chrono::nanoseconds sleepTime = timeProvider->realTimeUntil(nextTime);
			mlog->trace("Sleeping for {0} ns", sleepTime.count());
			{
				std::unique_lock<std::mutex> lock(mtx);
				wait.wait_for(lock, sleepTime);
			}
		}
		else {
			break;
		}
		mlog->error("looping again...");
	}
}

template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model, TimeType>::schedule(EventUniquePtr<Model, TimeType>&& ev) {
	schedModel->schedule(std::move(ev));
	mlog->trace("notifying...");
	wakeUp();
}

template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model, TimeType>::reset(TimeType resetToTime) {
	// TODO: reset queues to what they were at resetToTime
	schedModel->reset(resetToTime);
	model->reset(resetToTime);
}


template <typename Model, typename TimeType = TimePoint>
void Scheduler<Model, TimeType>::stop() {
	shouldStop = true;
	mlog->error("notifying...");
	wait.notify_all();
	if (theThread.joinable()) {
		mlog->trace("joining...");
		theThread.join();
	}
}

}