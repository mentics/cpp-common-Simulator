#pragma once
#include "MenticsCommon.h"
#include <deque>
#include <functional>
#include <list>


namespace MenticsGame
{
	template <typename T, typename TimeType = TimePoint>
	class SignalCollection
	{
		struct SignalCollectionItem { TimeType created; TimeType deleted = FOREVER; T value; };
		std::list<SignalCollectionItem> vals;
	public:
		void forEach(TimeType at, std::function<void(T*)> f)
		{
			for (SignalCollectionItem i : vals) {
				if (i.created <= at && at < i.deleted) {
					f(&i.value);
				}
				else {
					continue;
				}
			}
		}

		void add(T value, TimeType now)
		{
			SignalCollectionItem i = { now, FOREVER, value };
			vals.push_back(i);
		}

		void removeOld(TimeType upTo)
		{
			
			vals.erase(std::remove_if(vals.begin(), vals.end(), [=](SignalCollectionItem a) {return a.deleted < upTo; }), vals.end());

		}

		void reset(TimeType resetTime)
		{
			vals.erase(std::remove_if(vals.begin(), vals.end(), [=](SignalCollectionItem a) {return a.created > resetTime; }), vals.end());
			for (SignalCollectionItem i : vals) {
				if (i.deleted >= resetTime) {
					i.deleted = FOREVER;
					if (i.deleted > resetTime) i.value.reset(resetTime);
				}
			}
		}

		};
		
		template <typename T, typename TimeType = TimePoint>
		class Signal
		{
			static TimeType oldest;
			struct ValueAtTime { T value; TimeType at; };
			
		public:
			std::deque<ValueAtTime> values;

			void reset(TimeType resetTime)
			{
				while (resetTime < values.back().at)values.pop_back();
			}

			void removeOldest(TimeType upTo)
			{
				for (auto time = values.front().at; time < upTo; time = values.front().at) {
					values.pop_front();
				}
			}
		};

		template <typename T, typename TimeType = TimePoint>
		class UnqSignal : public Signal<nn::nn_unique_ptr<T>, TimeType>
		{
		
			struct ValueAtTime { nn::nn_unique_ptr<T> value; TimeType at; };

		public:
			UnqSignal (nn::nn_unique_ptr<T> v)
			{
				values.emplace_back(v, 0);
			}

			T* get(TimeType at)
			{
				for (std::deque<ValueAtTime>::reverse_iterator i = values.rbegin(); i != values.rend(); ++i)
				{
					if (i->at <= at) {
						return i.value;
					}
				}
			}

			void add(nn::nn_unique_ptr<T>&& val, TimeType t) 
			{
				if (values.size() > 1 && values[1].at <= oldest) values.pop_front();
				values.emplace_back(val, t);
			}
		};

		template <typename T, typename TimeType = TimePoint>
		class ValueSignal : public Signal<T, TimeType>
		{
	
		public:
			ValueSignal(T v) {
				values.emplace_back(v, 0);
			}

			void add(T val, TimeType t)
			{
				if (values.size() > 1 && values[1].at <= oldest) values.pop_front();

				values.push_back(ValueAtTime{ val , t });
			}

			T get(TimeType at)
			{
				for (std::deque<ValueAtTime>::reverse_iterator i = values.rbegin();
					i != values.rend(); ++i)
				{
					if (i->at <= at) {
						return i->value;
					}
				}
			}
		};
		




		template <typename T, typename TimeType = TimePoint>
		class FunctionSignal : public Signal<std::function<T()>, TimeType>
		{
		public:
			void add(std::function<T()> val, TimeType t)
			{
				values.emplace_back(val, t);
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