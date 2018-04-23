#pragma once
#include "MenticsCommon.h"
#include "concurrentqueue.h"
#include "PriorityQueue.h"
#include "EventBases.h"

namespace MenticsGame {

template<typename TimeType, typename Model>
class Scheduler;

template <typename TimeType, typename Model>
class SchedulerModel {
    TimeType maxTimeAhead = 2000000000;
    moodycamel::ConcurrentQueue<EventUniquePtr<TimeType, Model>> incoming;
    PriorityQueue<EventUniquePtr<TimeType, Model>, decltype(&Event<TimeType, Model>::compare)> processing;
    std::deque<EventUniquePtr<TimeType, Model>> forReset;
    std::deque<OutEventUniquePtr<TimeType>> outgoing;

protected:
    void schedule(EventUniquePtr<TimeType, Model>&& ev);

public:
    friend class Scheduler<TimeType, Model>;
    SchedulerModel() : incoming(1024), processing(&Event<TimeType, Model>::compare) {}
    ~SchedulerModel() {
        mlog->error("SchedulerModel destructor");
    }

    // Returns minimum timeToRun of ingested events
    TimeType processIncoming();
    void reset(TimeType toTime);
    Event<TimeType, Model>* first(TimeType maxTime);
    void completeFirst();

    void addOutEvent(OutEventUniquePtr<TimeType>&& outEvent);
    void consumeOutgoing(const std::function<void(OutEventPtr<TimeType>)> handler, const TimeType upToTime);
};
PTRS2(SchedulerModel, Model, TimeType)

}
