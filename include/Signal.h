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
		void undo()
		{
			Values.pop_back();
		}


		void removeOldest(TimeType upTo)
		{
			for (auto time = Values.front().at; time < upTo; time = Values.front().at) {
				Values.pop_front();
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

		T get(TimeType at)
		{
			ValueAtTime val{0,0};
			TimeType old = 0;
			for (std::deque<ValueAtTime>::iterator i = Values.begin(); i != Values.end(); i++)
			{
				if (i->at == at) { val = *i; break; }
				else if (i->at > old && i->at < at)
				{
					old = i->at;
					val = *i;
				}
			}

			return val.value();
		}
	};


	template <typename T, typename TimeType = TimePoint>
	class ValueSignal : public Signal<T, TimeType>
	{
	public:
		void add(T val, TimeType t)
		{
			Values.push_back(ValueAtTime{ val , t });
		}

		T get(TimeType at)
		{
			ValueAtTime val{0,0};
			TimeType old = 0;
			for (std::deque<ValueAtTime>::iterator i = Values.begin(); i != Values.end(); i++)
			{
				if (i->at == at) { val = *i; break; }
				else if (i->at > old && i->at < at)
				{
					old = i->at;
					val = *i;
				}
			}

			return val.value;
		
		}

	};
	
	
	

}

