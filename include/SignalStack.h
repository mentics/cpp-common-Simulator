#pragma once
#include "MenticsCommon.h"
#include <stack>

template<typename ItemType, typename TimeType = TimePoint>
class SignalStack
{
	struct ValatTime { ItemType item; TimeType time; TimeType deletedAt; };
	std::list<ValatTime> deleted;
	std::stack<ValatTime> m_stack;
public:
	
	void push(ItemType i, TimeType at);
	void pop(TimeType at); 
	ItemType peek();
	void reset(TimeType resetAt);
	void removeOldest(TimeType upTo, TimeType now);   

};

