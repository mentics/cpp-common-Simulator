#include "stdafx-scheduler.h"
#include "Scheduler.h"
#include "Signal.h"

namespace MenticsGame {

// This is the method run by the Scheduler thread.
// It loops forever and processes the events on the processing queue up to 
template<typename TimeType, typename Model>
void Scheduler<TimeType, Model>::run() {
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
            Event<TimeType, Model>* ev = schedModel->first(maxTime);
            if (ev != NULL) {
                nextTime = ev->timeToRun;
                if (nextTime < now) {
                    mlog->warn("Event time prior to now, event processing can't keep up.");
                }
                if (nextTime < processedTime) {
                    mlog->error("Back in time processing");
                }
                processedTime = nextTime;
                ev->run(nn::nn_addr(*this), model);
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
        } else {
            break;
        }
        mlog->error("looping again...");
    }
}

template<typename TimeType, typename Model>
void Scheduler<TimeType, Model>::schedule(TimeType afterDuration, EventUniquePtr<TimeType, Model>&& ev) {
    TimeType n = timeProvider->now();
    ev->created = n;
    ev->timeToRun = n + afterDuration;
    schedModel->schedule(std::move(ev));
    mlog->trace("notifying...");
    wakeUp();
}

template<typename TimeType, typename Model>
void Scheduler<TimeType, Model>::reset(TimeType resetToTime) {
    // TODO: reset queues to what they were at resetToTime
    schedModel->reset(resetToTime);
    model->reset(resetToTime);
}


template<typename TimeType, typename Model>
void Scheduler<TimeType, Model>::stop() {
    shouldStop = true;
    mlog->error("notifying...");
    wait.notify_all();
    if (theThread.joinable()) {
        mlog->trace("joining...");
        theThread.join();
    }
}

}
