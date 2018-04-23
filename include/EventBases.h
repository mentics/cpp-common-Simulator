#pragma once
#include <functional>
#include "MenticsCommon.h"

namespace MenticsGame {

enum EventType {
    EventQuipCreated
};

typedef uint64_t EntityId;

enum AgentUpdateType {
    deleted, changeTraj
};
struct UpdateKey {
    EntityId agentId;
    AgentUpdateType updateType;
};


////
// After Events are processed, they can emit OutEvents that are handled later by the frontend.
// In other words, Events are processed at processing time and OutEvents are processed at real time.
template <typename TimeType>
struct OutEvent {
    const TimeType occursAt;
    const EventType type;
    OutEvent(const TimeType occursAt, const EventType t) : occursAt(occursAt), type(t) {}
};
PTRS1S(OutEvent, TimeType)


template<typename TimeType, typename Model> struct Event;
PTRS2S(Event, TimeType, Model)
////
// An interface other parts of the code use to interact with the scheduler.
PTRS2(Schedulator, TimeType, Model)
#define SchedulatorP SchedulatorPtr<TimeType,Model>
#define SchedulatorUP SchedulatorUniquePtr<TimeType,Model>
template<typename TimeType, typename Model>
class Schedulator {
public:
    virtual void schedule(const TimeType afterDuration, EventUniquePtr<TimeType, Model>&& ev) = 0;
    virtual void addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) = 0;
    virtual void listen(const UpdateKey k, std::function<void(SchedulatorP const)> handler) = 0;
};


////
// An Event that can be put on the Scheduler.
template<typename TimeType, typename Model>
struct Event {
    // NOTE: these should be const, but they're set by scheduler inside schedule() method
    TimeType created;
    TimeType timeToRun;

    //Event(const TimeType created, const TimeType timeToRun) : created(created), timeToRun(timeToRun) {
    //}

    static bool compare(const EventUniquePtr<TimeType, Model>& ev1, const EventUniquePtr<TimeType, Model>& ev2) {
        return ev1->timeToRun > ev2->timeToRun;
    }
    virtual void run(SchedulatorPtr<TimeType, Model> sched, nn::nn<Model*> model) = 0;
};


////
// In case we need an empty Event. This probably should be removed at some point if we can.
template<typename TimeType, typename Model>
struct EventZero : public Event<TimeType, Model> {
public:
    EventZero() : Event() {}
    void run(SchedulatorPtr<TimeType, Model> sched, nn::nn<Model*> model) {};
};

}
