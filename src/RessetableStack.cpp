#include "stdafx.h"
#include "RessetableStack.h"




template<typename ItemType, typename TimeType>
void RessetableStack<ItemType, TimeType>::push(ItemType i, TimeType at)
{
	m_stack.push({ i,at });
}

template<typename ItemType, typename TimeType>
void RessetableStack<ItemType, TimeType>::pop()
{
	m_stack.pop();
}

template<typename ItemType, typename TimeType>
ItemType RessetableStack<ItemType, TimeType>::peek()
{
	return m_stack.top();
}

template<typename ItemType, typename TimeType>
void RessetableStack<ItemType, TimeType>::reset(TimeType resetAt)
{
	while ((!m_stack.empty()) && resetAt <= m_stack.back()->time)
	{
		undoActions.pop_back();
	}
}



