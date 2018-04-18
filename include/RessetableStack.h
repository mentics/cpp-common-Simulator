#pragma once
#include "MenticsCommon.h"
#include <stack>

template<typename ItemType, typename TimeType = TimePoint>
class RessetableStack
{
	struct ValatTime { ItemType item; TimeType time; };
	std::stack<ValatTime> m_stack;
public:
	RessetableStack();
	void push(ItemType i, TimeType at);
	void pop();
	ItemType peek();
	void reset(TimeType resetAt);
	~RessetableStack();
};

