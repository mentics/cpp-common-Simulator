
#include "MenticsCommon.h"
#include "readerwriterqueue.h"


using namespace moodycamel;

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
		friend ResettableTest;
	protected:
		ReaderWriterQueue<Change<T, TimeType>> buffer;
		TimeType timeCurrent, timeOldest;
		T stateOldest, stateCurrent;
	};

	template<typename T,typename TimeType>
	void Resettable<T, TimeType>::apply(Change<T, TimeType> const& change)
	{
		assert(change.time >= timeCurrent);
		change.action(stateCurrent);
		buffer.enqueue(change);
		timeCurrent = change.time;
	}

	template<typename T, typename TimeType>
	void Resettable<T, TimeType>::moveOldest(TimeType const& time)
	{
		assert(time < timeOldest);
		if (time > timeCurrent) {
			return;
		}

		while(buffer.peek() != nullptr)
		{
			if (buffer.peek()->time >= time) {
				buffer.peek()->action(stateOldest);
				buffer.pop();
			}
		}
	}


	template<typename T, typename TimeType>
	void Resettable<T, TimeType>::reset(TimeType const& time)
	{
		assert(time < buffer.peek()->time);
		if (time > timeCurrent) {
			return;
		}
		stateCurrent = stateOldest;
		while(buffer.peek() != nullptr)
		{
			if (buffer.peek()->time <= time) {
				buffer.peek()->action(stateCurrent);
				buffer.pop();
			}
		}
		timeCurrent = buffer.peek()->time;
		
	}
}