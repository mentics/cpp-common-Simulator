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
		ONLY_MOVE(SignalCollection);

		struct SignalCollectionItem {
			TimeType created;
			TimeType deleted = FOREVER;
			T value;
			SignalCollectionItem(TimeType created, T&& v) : created(created), value(std::move(v)) {}
		};

		std::list<SignalCollectionItem> vals;
	public:
		SignalCollection() {}

		void forEach(TimeType at, std::function<void(T*)> f)
		{
			for (SignalCollectionItem& i : vals) {
				if (i.created <= at && at < i.deleted) {
					f(&i.value);
				}
				else {
					continue;
				}
			}
		}

		void add(T&& value, TimeType now)
		{
			// TODO: vals.remove_if deleted < SignalValue::oldest
			vals.emplace_back(now, std::move(value));
		}

		void removeOld(TimeType upTo)
		{
			vals.remove_if([upTo](const SignalCollectionItem& a) {return a.deleted < upTo; });
		}

		void reset(TimeType resetTime)
		{
			vals.remove_if([resetTime](const SignalCollectionItem& a) {return a.created > resetTime; });
			for (SignalCollectionItem& i : vals) {
				if (i.deleted >= resetTime) {
					i.deleted = FOREVER;
					if (i.deleted > resetTime) i.value.reset(resetTime);
				}
			}
		}

		};

		template <typename T, typename TimeType = TimePoint>
		class SignalUnique
		{
			ONLY_MOVE(SignalUnique);

			struct ValueAtTimeUnique {
				ValueAtTimeUnique(nn::nn_unique_ptr<T>&& value, TimeType at) : value(std::move(value)), at(at) {}
				nn::nn_unique_ptr<T> value;
				TimeType at;
			};

			std::deque<ValueAtTimeUnique> values;

		public:

			SignalUnique (nn::nn_unique_ptr<T>&& v)
			{
				values.emplace_back(std::move(v), 0);
			}

			T* get(TimeType at)
			{
				for (std::deque<ValueAtTimeUnique>::reverse_iterator i = values.rbegin(); i != values.rend(); ++i)
				{
					if (i->at <= at) {
						return i->value.get();
					}
				}
			}

			void add(nn::nn_unique_ptr<T>&& val, TimeType t) 
			{
				if (values.size() > 1 && values[1].at <= SignalValue::oldest) values.pop_front();
				values.emplace_back(std::move(val), t);
			}

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
		class SignalValue
		{
			ONLY_MOVE(SignalValue);

			struct ValueAtTime {
				ValueAtTime(T value, TimeType at) : value(value), at(at) {}
				T value;
				TimeType at;
			};

			std::deque<ValueAtTime> values;

		public:
			static TimeType oldest;

			SignalValue(T v) {
				values.emplace_back(v, 0);
			}

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

			void add(T val, TimeType t)
			{
				if (values.size() > 1 && values[1].at <= oldest) values.pop_front();
				values.emplace_back(val , t );
			}

			const T& get(TimeType at)
			{
				for (std::deque<ValueAtTime>::reverse_iterator i = values.rbegin(); i != values.rend(); ++i)
				{
					if (i->at <= at) {
						return i->value;
					}
				}
			}
		};

		template <typename TimeType >
		std::function<bool()> constantValue(TimeType t, TimeType now)
		{
			return [t,now]() {return now = > value};
		}

		template <typename TimeType >
		std::function<double()> CappedLinearValue(double startGameTime, double initial, double rate, double max, TimeType now)
		{
			return [startGameTime, initial, rate, max, now]() {n = initial + rate * (now - startGameTime); return n > max ? max : n; };
		}

		template<typename T, typename TimeType>
		TimeType SignalValue<T, TimeType>::oldest = 0;
}