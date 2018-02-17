
#include "MenticsCommon.h"
#include "readerwriterqueue.h"

template <typename T> class queue;
template <typename T> class vector;



using namespace moodycamel;

namespace MenticsGame {
	
		//template<typename T> using Action = std::function<void(T&)>;

		template<typename T,typename TimeType>
		struct Change {
			Change(Action<T> action, TimeType const& time) : action(action), time(time) {}
			TimeType time;
			Action<T> action;
		};
		/*
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
			timeCurrent = change.time;
			change.action(stateCurrent);
			buffer.enqueue(change);
		}

		template<typename T, typename TimeType>
		void Resettable<T, TimeType>::moveOldest(TimeType const& time)
		{
			assert(time >= timeOldest);
			if (time > timeCurrent) {
				// TODO: should we set time to timeCurrent? should we report/throw error?
				return;
			}

			for (auto value = buffer.peek(); value->time <= time; value = buffer.peek()) {
				value->action(stateOldest);
				buffer.pop();
			}
		}


		template<typename T, typename TimeType>
		void Resettable<T, TimeType>::reset(TimeType const& time)
		{
			if (time > timeCurrent) {
				return;
			}
			stateCurrent = stateOldest; // copy old state to current
			std::vector<Change<T, TimeType>> temp; // TODO: optimize this
			for (auto value = buffer.peek(); value->time <= time; value = buffer.peek()) {
				value->action(stateCurrent);
				temp.push_back(*value);
				// TODO: we need a way to walk this without popping them off the buffer.
				buffer.pop();
			}
			// buffer.clear(); // TODO: we need to clear it one way or another
			for (auto change : temp) {
				buffer.enqueue(change);
			}

			timeCurrent = time;
		}
	}
	*/


#include <queue>
	class Action
	{
		virtual void apply() = 0;
	};

	template <typename TimeType, typename T, typename Collection, typename AddFunc, typename DelFunc>
	class ChangeValue : Action
	{
		TimeType at;
		T value;
		T* ptr;
	public:
		ChangeValue(TimeType at, T ptr, T existingValue) : at(at), ptr(ptr), value(existingValue) {}
		void apply()
		{
			*ptr = value;
		}
	};

	template <typename TimeType, typename T, typename Collection, typename AddFunc, typename DelFunc>
	class DeleteItem : Action
	{
		TimeType at;
		T value;
		Collection ptr;
	public:
		DeleteItem(TimeType at, Collection collection, T newItem) : at(at), ptr(collection), value(newItem) {}
		void apply()
		{
			*ptr.DelFunc(value);
		}
	};


	template <typename TimeType, typename T, typename Collection, typename AddFunc, typename DelFunc>
	class AddItem : Action
	{
		TimeType at;
		T value;
		Collection ptr;
	public:
		AddItem(TimeType at, Collection ptr, T existingValue) : at(at), ptr(ptr), value(existingValue) {}
		void apply()
		{
			*ptr.AddFunc(value);
		}
	};

	template <typename TimeType, typename T, typename Collection, typename AddFunc, typename DelFunc>
	class Resettable
	{
		queue<Action> undoActions;
	public:
		void changeValue(TimeType at, T* ptr, const T& value)
		{
			ChangeValue<TimeType, T, typename Collection, AddFunc, DelFunc> cv(at, ptr, *ptr);
			undoActions.emplace(cv);
			*ptr = value;
		}

		void addItem(TimeType at, T collection, T newItem)
		{
			DeleteItem<TimeType, T, typename Collection, AddFunc, DelFunc> di(at, collection, newItem);
			undoActions.emplace(di);
			collection.Func(newItem);
		}

		void deleteItem(TimeType at, T collection, T doomedItem)
		{
			AddItem<TimeType, T, typename Collection, AddFunc, DelFunc> ai(at, collection, doomedItem);
			undoActions.emplace(ai);
			collection.DelFunc(doomedItem);
		}

		void reset(TimeType to)
		{
			while (to > undoActions.front().at)
			{
				undoActions.front().apply();
				undoActions.pop();
			}
		}
	};

}