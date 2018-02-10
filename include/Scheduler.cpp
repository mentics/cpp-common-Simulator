//#include "stdafx.h"

#include "Scheduler.h"
#include <spdlog\spdlog.h>

namespace MenticsGame {

using namespace std::chrono_literals;



template <typename TimeType, typename Model>
TimeType SchedulerModel<TimeType,Model>::processIncoming() {
	const auto m_log = spdlog::stdout_logger_st("unique name");
	m_log->trace("SchedulerModel::processIncoming");
	// TODO: assert scheduler thread?
	TimeType minTime = FOREVER;
	EventUniquePtr<TimeType,Model> ev = uniquePtr<EventZero<TimeType,Model>>(); // TODO: simplify this?
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



// This is the method run by the Scheduler thread.
// It loops forever and processes the events on the processing queue up to 
template <typename TimeType, typename Model>
void Scheduler<TimeType,Model>::run() {
	const auto m_log = spdlog::stdout_logger_st("unique name");
	m_log->trace("Scheduler::run");

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
			Event<TimeType,Model>* ev = schedModel->first(maxTime);
			if (ev != NULL) {
				nextTime = ev->timeToRun;
				if (nextTime < now) {
					m_log->warn("Event time prior to now, event processing can't keep up.");
				}
				if (nextTime < processedTime) {
					m_log->error("Back in time processing");
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
			m_log->trace("Sleeping for {0} ns", sleepTime.count());
			{
				std::unique_lock<std::mutex> lock(mtx);
				wait.wait_for(lock, sleepTime);
			}
		}
		else {
			break;
		}
		m_log->error("looping again...");
	}
}

template <typename TimeType, typename Model>
void Scheduler<TimeType, Model>::reset(TimeType resetToTime) {
	// TODO: reset queues to what they were at resetToTime
	schedModel->reset(resetToTime);
	model->reset(resetToTime);
}


template <typename TimeType, typename Model>
void Scheduler<TimeType,Model>::stop() {
	const auto m_log = spdlog::stdout_logger_st("unique name 1");
	shouldStop = true;
	m_log->error("notifying...");
	wait.notify_all();
	if (theThread.joinable()) {
		m_log->trace("joining...");
		theThread.join();
	}
}

template<typename TimeType, typename Model>
inline bool Event<TimeType, Model>::compare(const EventUniquePtr<TimeType, Model>& ev1, const EventUniquePtr<TimeType, Model>& ev2) {
	return ev1->timeToRun > ev2->timeToRun;  
}

template <typename TimeType, typename Model>
SchedulerModel<TimeType, Model>::~SchedulerModel() {
	m_log->error("SchedulerModel destructor");
}

// Runs on outside thread
template <typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::schedule(EventUniquePtr<TimeType, Model> ev) {
	m_log->trace("Scheduling event");
	incoming.enqueue(std::move(ev));
}

template <typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::addOutEvent(OutEventUniquePtr<TimeType> outEvent) {
	outgoing.push_back(std::move(outEvent));
}


template <typename TimeType, typename Model>
Scheduler<TimeType, Model>::Scheduler(std::string name, SchedulerModelPtr<TimeType, Model> schedModel, SchedulerTimeProviderPtr<TimeType> timeProvider, nn::nn<Model*> model) :
	CanLog(name),
	schedModel(schedModel), timeProvider(timeProvider), model(model),
	theThread(&Scheduler<TimeType, Model>::run, this),
	processedTime(0) {}

template <typename TimeType, typename Model>
Scheduler<TimeType, Model>::~Scheduler() {
	m_log->error("Scheduler destructor");
	stop();
}

template <typename TimeType, typename Model>
void Scheduler<TimeType, Model>::schedule(EventUniquePtr<TimeType, Model> ev) {
	schedModel->schedule(std::move(ev));
	m_log->trace("notifying...");
	wakeUp();
}

template <typename TimeType, typename Model>
void Scheduler<TimeType, Model>::wakeUp() {
	wait.notify_all();
}

// Only used for testing. TODO: untested
template <typename TimeType, typename Model>
void Scheduler<TimeType, Model>::WaitUntilProcessed(TimeType until) {
	do {
		Thread.Sleep(100);
	} while (processedTime < until);
}

}
