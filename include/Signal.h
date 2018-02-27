#pragma once
#include "MenticsCommon.h"
#include <deque>
#include <functional>

namespace MenticsGame
{
	template <typename T, typename TimeType = TimePoint>
	class Signal
	{
		struct ValueAtTime { T value; TimeType at; };
		std::deque<ValueAtTime> Values;
	public:
		void add(T val);
		void undo();
		void removeOldest(TimeType upTo);

	};

	template <typename T, typename TimeType = TimePoint>
	class FunctionSignal : public Signal<std::function<T>, TimeType>
	{
	public:
		T get(TimeType at);
	};


	template <typename T, typename TimeType = TimePoint>
	class ValueSignal : public Signal<T, TimeType>
	{
	public:
		T get(TimeType at);
	};
	
	
	

}

