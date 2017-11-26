#include <boost/circular_buffer.hpp>
#include "MenticsCommon.h"

namespace MenticsGame {

	template<typename T> using Action = std::function<void(T&)>;

	template<typename T,typename TimeType>
	struct Change {
		Change(Action<T> action, TimeType const& time) : action(action), time(time) {}
		TimeType time;
		Action<T> action;
	};

	template<typename T, typename TimeType> using ChangeCallback = std::function<bool (Change<T,TimeType> const&)>;

	class ResettableTest;

	template<typename T, typename TimeType>
	class Resettable
	{
	public:
		Resettable(T t, int capacity) : stateOldest(t), stateCurrent(t), buffer(capacity) {}
		Resettable(T t, int capacity, TimeType time) : stateOldest(t), stateCurrent(t), buffer(capacity), timeCurrent(time) {}
		~Resettable() {}
		T getCurrentState() { return stateCurrent; }
		void apply(Change<T,TimeType> const& change);
		void moveOldest(TimeType const& time);
		void reset(TimeType const& time);
		void walk(ChangeCallback<T,TimeType> const& callback);
		friend ResettableTest;
	protected:
		boost::circular_buffer<Change<T,TimeType>> buffer;
		TimeType timeCurrent;
		T stateOldest, stateCurrent;
	};

	template<typename T,typename TimeType>
	void Resettable<T, TimeType>::apply(Change<T, TimeType> const& change)
	{
		assert(change.time > timeCurrent);
		if (buffer.full()) {
			buffer.back().action(stateOldest);
		}
		change.action(stateCurrent);
		buffer.push_back(change);
		timeCurrent = change.time;
	}

	template<typename T, typename TimeType>
	void Resettable<T, TimeType>::moveOldest(TimeType const& time)
	{
		if (time > timeCurrent) {
			return;
		}
		auto first = std::find_if(buffer.begin(), buffer.end(), [&](Change<T, TimeType> const& c) {
			if (c.time <= time) {
				c.action(stateOldest);
				return false;
			}
			return true;
		});
		buffer.erase(buffer.begin(), first);
	}

	template<typename T, typename TimeType>
	void Resettable<T, TimeType>::walk(ChangeCallback<T,TimeType> const& callback)
	{
		std::find_if(buffer.begin(), buffer.end(), callback);
	}

	template<typename T, typename TimeType>
	void Resettable<T, TimeType>::reset(TimeType const& time)
	{
		if (time > timeCurrent || time < buffer.front().time) {
			return;
		}
		stateCurrent = stateOldest;
		walk([&](Change<T, TimeType> const &change)->bool {
			if (change.time <= time) {
				change.action(stateCurrent);
				timeCurrent = change.time;
				return false;
			}
			return true;
		});
		stateOldest = stateCurrent;
		buffer.clear();
	}
}