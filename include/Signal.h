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
		std::deque<ValueAtTime> values;
	public:
		void reset(TimeType resetTime)
		{
			while (resetTime < values.back())values.pop_back();
		}

		void add(T val, TimeType t)
		{
			values.push_back(ValueAtTime{ val , t });
		}

		T get(TimeType at)
		{
			for (std::deque<ValueAtTime>::reverse_iterator i = values.rbegin();
				i != values.rend(); ++i)
			{
				if (i->at <= at){
					return i->value;
				}
			}
		}

		void removeOldest(TimeType upTo)
		{
			for (auto time = values.front().at; time < upTo; time = values.front().at) {
				values.pop_front();
			}
		}

	};

	template <typename T, typename TimeType = TimePoint>
	class FunctionSignal : public Signal<std::function<T()>, TimeType>
	{
	public:
		void add(std::function<T()> val, TimeType t)
		{
			Values.push_back(ValueAtTime{ val , t });
		}

		void getValue(TimeType at)
		{
			get(at)(at);
		}


		static bool constantValue(TimeType t, TimeType now)
		{
			return now = > value;
		}

		static double CappedLinearValue(double startGameTime, double initial, double rate, double max, TimeType now)
		{
			double n = initial + rate * (now - startGameTime);
			return n > max ? max : n;

		}
	};


	
	
	

}

