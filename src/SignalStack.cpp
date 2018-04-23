#include "stdafx.h"
#include "SignalStack.h"




template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::push(ItemType i, TimeType at) {
	m_stack.push({ i,at,0});
}

template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::pop(TimeType at) {
	m_stack.top().deletedAt = at;
	deleted.push_back(m_stack.top());
	m_stack.pop();
}

template<typename ItemType, typename TimeType>
ItemType SignalStack<ItemType, TimeType>::peek() {
	return m_stack.top().item;
}

template<typename ItemType, typename TimeType>
bool SignalStack<ItemType, TimeType>::empty() {
	return m_stack.empty();
}

template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::reset(TimeType resetAt)
{
	while ((!m_stack.empty()) && resetAt <= m_stack.top().time)
	{
		m_stack.pop();
	}
	while ((!deleted.empty()) && resetAt <= deleted.front().deletedAt && resetAt > deleted.front().time)
	{
		m_stack.push(deleted.front());
		deleted.pop_front();
	}
}

template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::removeOldest(TimeType upTo)
{
	if (m_stack.empty())return;

	TimeType t = m_stack.top().time;

	while(t < upTo){
		m_stack.pop();

		if(!m_stack.empty())
		t = m_stack.top().time;
		else break;

	}
	
}



