#pragma once
#include "MenticsCommon.h"
#include <stack>

template<typename ItemType, typename TimeType = TimePoint>
class SignalStack {
    struct ValueAtTime {
        ItemType item; TimeType time; TimeType deletedAt;
    };
    std::list<ValueAtTime> deleted;
    std::stack<ValueAtTime> m_stack;

public:
    void push(ItemType i, TimeType at);
    void pop(TimeType at);
    ItemType peek();
    bool empty();
    void reset(TimeType resetAt);
    void removeOldest(TimeType upTo);
};

