#pragma once
#include "MenticsCommon.h"
#include <deque>
#include <functional>

namespace MenticsGame
{
	template <typename T, typename TimeType = TimePoint>
	class Signal
	{
	protected:
		struct ValueAtTime { T value; TimeType at; };
		std::deque<ValueAtTime> Values;
	public:
		void add(T val, TimeType t)
		{
			Values.push_back(ValueAtTime{ val , t });
		}

		void undo()
		{
			Values.pop_back();
		}


		void removeOldest(TimeType upTo)
		{
			for (auto time = Values.front().at; time < upTo; value = Values.front().at) {
				Values.pop_front();
			}
		}

	};

	template <typename T, typename TimeType = TimePoint>
	class FunctionSignal : public Signal<std::function<T>, TimeType>
	{
	public:
		T get(TimeType at)
		{
			auto val = std::find_if(Values.begin(), Values.end(), [=](ValueAtTime a) {return a.at == at ? true : false; });
			return val->value();
		}
	};


	template <typename T, typename TimeType = TimePoint>
	class ValueSignal : public Signal<T, TimeType>
	{
	public:
		T get(TimeType at)
		{
			auto val = std::find_if(Values.begin(), Values.end(), [=](ValueAtTime a) {return a.at == at ? true : false; });
			return val->value;
		}

	};
	
	
	

}

