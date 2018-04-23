#pragma once
#include "MenticsCommon.h"
#include "EventBases.h"
#include "SchedulerModel.h"

namespace chrono = std::chrono;

namespace MenticsGame {

////
// An interface to provide time to the Scheduler.
template <typename TimeType>
class SchedulerTimeProvider {
public:
    virtual TimeType now() = 0;
    virtual chrono::nanoseconds realTimeUntil(TimeType t) = 0;
};
PTRS1(SchedulerTimeProvider, TimeType)


////
// A Scheduler that runs in it's own thread and processes Events.
template <typename TimeType, typename Model>
class Scheduler : public Schedulator<TimeType, Model> {
    SchedulerModelPtr<TimeType, Model> schedModel;
    SchedulerTimeProviderPtr<TimeType> timeProvider;
    nn::nn<Model*> model;

    std::thread theThread;
    bool shouldStop = false;
    //std::unique_lock<std::mutex> lock;
    std::mutex mtx;
    std::condition_variable wait;

    TimeType processedTime;

public:
    friend class SchedulerTest;
    Scheduler(SchedulerModelPtr<TimeType, Model> schedModel, SchedulerTimeProviderPtr<TimeType> timeProvider, nn::nn<Model*> model)
        : schedModel(schedModel), timeProvider(timeProvider), model(model),
        theThread(&Scheduler<TimeType, Model>::run, this),
        processedTime(0) {}


    ~Scheduler() {
        mlog->error("Scheduler destructor");
        stop();
    }

    void run();

    void schedule(TimeType afterDuration, EventUniquePtr<TimeType, Model>&& ev);

    void wakeUp() {
        wait.notify_all();
    }

    void reset(TimeType resetToTime);

    void stop();

    // Only used for testing. TODO: untested
    void WaitUntilProcessed(TimeType until) {
        do {
            std::this_thread::sleep_for(chrono::microseconds(100));
        } while (processedTime < until && !schedModel->processing.empty());
    }

    void addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) {
        schedModel->addOutEvent(std::move(outEvent));
    }
    void consumeOutgoing(const std::function<void(OutEventPtr<TimeType>)> handler, const TimeType upToTime) {
        // TODO: schedModel->consumeOutgoing(handler, upToTime);
    }

    //TimeType getProcessedTime() { return processedTime; }

    void listen(const UpdateKey key, std::function<void(SchedulatorP const)> handler) override {
        // TODO
    }
};
PTRS2(Scheduler, Model, TimeType)


}
