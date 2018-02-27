#include "stdafx.h"
#include "Signal.h"


namespace MenticsGame
{
	template<typename T, typename TimeType>
	void Signal<T, TimeType>::add(T val)
	{
		Values.push_back(val);
		removeOldest(oldest);
	}

	template<typename T, typename TimeType>
	void Signal<T, TimeType>::undo()
	{
		Values.pop_back();
	}

	template<typename T, typename TimeType>
	void Signal<T, TimeType>::removeOldest(TimeType upTo)
	{
		for (auto time = Values.front().at; time < upTo ; value = Values.front().at) {
			Values.pop_front();
		}
	}


	template<typename T, typename TimeType>
	T FunctionSignal<T, TimeType>::get(TimeType at)
	{
		T val = std::find_if(Values.begin(), Values.end(), [](T a, T b) {return a.at == b.at ? true : false; }).value();
		return val;
	}

	template<typename T, typename TimeType>
	T ValueSignal<T, TimeType>::get(TimeType at)
	{
		T val = std::find_if(Values.begin(), Values.end(), [](T a, T b) {return a.at == b.at ? true : false; }).value;
		return val;
	}

}