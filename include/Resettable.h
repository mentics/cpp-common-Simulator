#pragma once

#include "MenticsCommon.h"
#include "readerwriterqueue.h"
#include <deque>


template <typename T> class queue;
template <typename T> class vector;



using namespace moodycamel;

namespace MenticsGame {
	

	template <typename T>
	size_t addVal(std::vector<T>* a, T v)
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
		ChangeValue(TimeType at, T* ptr, T existingValue) : Action(at), ptr(ptr), value(existingValue) {}
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
		std::deque<std::unique_ptr<Action<TimeType>>> undoActions;
	public:
		  
		template <typename T> 
		void changeValue(TimeType at, T* ptr, const T value)
		{
			using temp_args = ChangeValue<TimeType, T*, T>;
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, ptr, *ptr);
			undoActions.push_back(std::move(p));
			*ptr = value;
		}

		template <typename T> 
		void addItem(TimeType at, std::vector<T>* collection, T newItem)
		{
			using temp_args = DeleteItem<TimeType, std::vector<T>, size_t>;
			size_t key = addVal(collection, newItem);
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, collection, key); 
			undoActions.push_back(std::move(p)); 
		}

		template <typename T, typename K>
		void deleteItem(TimeType at, std::vector<T>* collection, K key)
		{
			using temp_args = AddItem<TimeType, std::vector<T>, T>;

			T obj = deleteVal(collection, key);
			std::unique_ptr<temp_args> p = std::make_unique<temp_args>(at, collection, obj);
			undoActions.push_back(std::move(p));
		}

		void reset(TimeType to)
		{
			while (  (!undoActions.empty()) && to <= undoActions.back()->at)
			{
				undoActions.back()->apply();
				undoActions.pop_back();
			}
		}
	};

	

}