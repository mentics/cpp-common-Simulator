
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
	void changeVal(std::vector<T>* a, T v)
	{
		a->push_back(v);
	}

	template<typename T>
	void deleteVal(std::vector<T>* a, int v) 
	{
		a->erase(a->begin() + v);
	}
	
	template <typename TimeType>
	class Action
	{
	public:
		TimeType at;
		virtual void apply() = 0;
	};

	template <typename TimeType, typename CollectionT, typename T>
	class ChangeValue : public Action<TimeType>
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

	template <typename TimeType, typename CollectionT, typename T>
	class DeleteItem : public Action<TimeType>
	{
		TimeType at;
		T value;
		CollectionT* ptr;
	public:
		DeleteItem(TimeType at, CollectionT* collection, T newItem) : at(at), ptr(collection), value(newItem) {}
		void apply()
		{
			ptr->erase(ptr->begin() + value);
		}
	};


	template <typename TimeType, typename CollectionT, typename T>
	class AddItem : public Action<TimeType>
	{
		TimeType at;
		T value;
		CollectionT* ptr;
	public:
		AddItem(TimeType at, CollectionT* ptr, T existingValue) : at(at), ptr(ptr), value(existingValue) {}
		void apply()
		{
			changeVal(ptr, value);
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
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, collection, collection->size() - 1); 
			undoActions.push(std::move(p)); 
			changeVal(collection, newItem);
		}

		template <typename T, typename K>
		void deleteItem(TimeType at, std::vector<T>* collection, K key)
		{
			using temp_args = AddItem<TimeType, std::vector<T>, T>;
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, collection, collection->at(key));
			undoActions.push(std::move(p));
			deleteVal(collection, key);
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