#include "stdafx-scheduler.h"
#include "SchedulerModel.h"

using namespace MenticsGame;

template<typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::schedule(EventUniquePtr<TimeType, Model>&& ev) {
    mlog->trace("Scheduling event");
    incoming.enqueue(std::move(ev));
}

template<typename TimeType, typename Model>
TimeType SchedulerModel<TimeType, Model>::processIncoming() {
    mlog->trace("SchedulerModel::processIncoming");
    // TODO: assert scheduler thread?
    EventUniquePtr<TimeType, Model> ev = uniquePtr<EventZero<TimeType, Model>>(); // TODO: simplify this?
    while (incoming.try_dequeue(ev)) {
        processing.push(std::move(ev));
    }
    return !processing.empty() ? processing.top()->timeToRun : (TimeType)FOREVER;
}

template<typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::reset(TimeType toTime) {
    // TODO
    // TODO: delete items off outgoing > toTime

    //untested
    for (int i = 0; i < outgoing.size(); i++) {
        if (outgoing[i]->occursAt > toTime) {
            outgoing.pop_back();
        }
    }
}

template<typename TimeType, typename Model>
Event<TimeType, Model>* SchedulerModel<TimeType, Model>::first(TimeType maxTime) {
    if (!processing.empty()) {
        Event<TimeType, Model>* top = processing.top().get();
        const TimeType nextTime = top->timeToRun;
        if (nextTime <= maxTime) {
            return top;
        }
    }
    return nullptr;
}

template<typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::completeFirst() {
    forReset.push_back(processing.pop());
}

template<typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) {
    outgoing.push_back(std::move(outEvent));
}

template<typename TimeType, typename Model>
void SchedulerModel<TimeType, Model>::consumeOutgoing(const std::function<void(OutEventPtr<TimeType>)> handler, const TimeType upToTime) {
    while (!outgoing.empty()) {
        OutEventPtr<TimeType> ev = NN_CHECK_ASSERT(outgoing.front().get());
        if (ev->occursAt <= upToTime) {
            mlog->error("inside consumeoutgoing");
            handler(ev);
            outgoing.pop_front();

            SignalValue<Model, TimeType>::oldest = upToTime - maxTimeAhead;
            // Finally after travelling through 3 queues, the event's eventful life has come to an end.
            // delete ev; <- it's deleted by unique_ptr
        } else {
            break;
        }
    }
}
