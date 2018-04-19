#include "stdafx.h"
#include "SignalStack.h"




template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::push(ItemType i, TimeType at)
{
	m_stack.push({ i,at });
}

template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::pop(TimeType at)
{
	m_stack.top().deletedAt = at;
	deleted.push_back(m_stack.top());
	m_stack.pop();
}

template<typename ItemType, typename TimeType>
ItemType SignalStack<ItemType, TimeType>::peek()
{
	return m_stack.top().item;
}

template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::reset(TimeType resetAt)
{
	

	while ((!deleted.empty()) && resetAt < deleted.back().deletedAt && resetAt > m_stack.top().time)
	{
		m_stack.push(deleted.back());
		deleted.pop_back();
	}
	while ((!m_stack.empty()) && resetAt <= m_stack.top().time)
	{
		m_stack.pop();
	}
}

template<typename ItemType, typename TimeType>
void SignalStack<ItemType, TimeType>::removeOldest(TimeType upTo, TimeType now)
{
	
	for (auto time = m_stack.top().time; time < upTo; time = m_stack.top().time) {
		m_stack.top().deletedAt = now;
		deleted.push(m_stack.top());
		m_stack.pop_front();
	}
	
}



