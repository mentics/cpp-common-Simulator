
#include "MenticsCommon.h"
#include "readerwriterqueue.h"
#include <queue>

template <typename T> class queue;
template <typename T> class vector;



using namespace moodycamel;

namespace MenticsGame {
	/*
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

	template <typename T>
	int addVal(std::vector<T>* a, T v)
	{
		a->push_back(v);
		return a->size() - 1;
	}

	template<typename T>
	T deleteVal(std::vector<T>* a, int v) 
	{
		auto tmp = a->at(v);
		a->erase(a->begin() + v);
		return tmp;
	}
	
	template <typename TimeType>
	class Action
	{
	public:
		Action(TimeType t) : at(t) {}
		TimeType at;
		virtual void apply() = 0;
	};

	template <typename TimeType, typename CollectionT, typename T>
	class ChangeValue : public Action<TimeType>
	{
		T value;
		T* ptr;
	public:
		ChangeValue(TimeType at, T ptr, T existingValue) : Action(at), ptr(ptr), value(existingValue) {}
		void apply()
		{
			*ptr = value;
		}
	};

	template <typename TimeType, typename CollectionT, typename Key>
	class DeleteItem : public Action<TimeType>
	{
		Key delKey;
		CollectionT* ptr;
	public:
		DeleteItem(TimeType at, CollectionT* collection, Key delKey) : Action(at), ptr(collection), delKey(delKey) {}
		void apply()
		{
			deleteVal(ptr, delKey);
		}
	};


	template <typename TimeType, typename CollectionT, typename T>
	class AddItem : public Action<TimeType>
	{
		TimeType at;
		T value;
		CollectionT* ptr;
	public:
		AddItem(TimeType at, CollectionT* ptr, T existingValue) : Action(at), ptr(ptr), value(existingValue) {}
		void apply()
		{
			addVal(ptr, value);
		}
	};

	
	template <typename TimeType>
	class Resettable
	{
		std::queue<std::unique_ptr<Action<TimeType>>> undoActions;
	public:
		  
		template <typename T> 
		void changeValue(TimeType at, T* ptr, const T value)
		{
			using temp_args = ChangeItem<TimeType, std::vector<T>, T>;
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, ptr, value);
			undoActions.push(std::move(p));
			*ptr = value;
		}

		template <typename T> 
		void addItem(TimeType at, std::vector<T>* collection, T newItem)
		{
			using temp_args = DeleteItem<TimeType, std::vector<T>, int>;
			int key = addVal(collection, newItem);
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, collection, key); 
			undoActions.push(std::move(p)); 
			addVal(collection, newItem);
		}

		template <typename T, typename K>
		void deleteItem(TimeType at, std::vector<T>* collection, K key)
		{
			using temp_args = AddItem<TimeType, std::vector<T>, T>;

			T obj = deleteVal(collection, key);
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, collection, obj);
			undoActions.push(std::move(p));
			
		}

		void reset(TimeType to)
		{
			while (to > undoActions.front().get()->at)
			{
				undoActions.front().get()->apply();
				undoActions.pop();
			}
		}
	};

	

}