#pragma once

#include <string>
#include <vector>
#include <cmath>
#include "MenticsCommon.h"
//#include "stringbuilder.h"

#define MAX_EVENTS 5000

namespace MenticsGame {

	class SignalTest;

	/// TODO: lots of opportunity to optimize

	template<typename C>
	class SignalBase : public CanLog {
	protected:
		std::vector<double> Times;
	// C# TO C++ CONVERTER TODO TASK: This event cannot be converted to C++:
	// public event Affect2<double, C> Inserted;

	private:
		friend SignalTest;
		/// times are when that indexed event starts to be applicable. It ends at the next event.
		std::vector<unsigned char> serializedTimes;
		std::vector<unsigned char> serializedEvents;
		int LatestIndex = 0;
	public:
		std::vector<std::unique_ptr<C>> Events;

		SignalBase(std::unique_ptr<C> initial) : CanLog(std::string("SignalBase")), Times(MAX_EVENTS, std::numeric_limits<double>::quiet_NaN()), Events(MAX_EVENTS) {
			// TODO Is assert required?
			// assert(initial != nullptr);
			// There must be an initial because there must always have been a value.
			Events[0] = std::move(initial);
			Times[0] = 0.0;
		}
		
		virtual void InsertEvent(double time, std::unique_ptr<C> ev) {
			assert(time >= 0);
			assert(ev != nullptr); // ok since we use unique_ptr
			// TODO: this is exact same code as in CompositeSignal--remove dupe code
			// NOTE: I'm not sure if this is exactly right to remove anything past the new event, but I think it is.
			while (Times[LatestIndex] > time) {
				// C must implement operator << if we want to use it like below
				//LOG(boost::log::trivial::debug) << "Throwing away event for " << Times[LatestIndex] << ": " << Events[LatestIndex];
				Times[LatestIndex] = std::numeric_limits<double>::quiet_NaN();
				// TODO LZU ask if ok to use nullptr (C# code use default(C))
				//Events[LatestIndex] = std::make_unique<C>();
				Events[LatestIndex] = nullptr;
				LatestIndex = (LatestIndex == 0) ? Times.size() - 1 : LatestIndex - 1;
				// TODO: do we need to do something to invalidate any events related to these we're discarding?
			}
			// If exactly the same time, replace the old one
			if (Times[LatestIndex] != time) {
				LatestIndex = (LatestIndex == Times.size() - 1) ? 0 : LatestIndex + 1;
			}
			if (LatestIndex >= MAX_EVENTS - 2) {
				// TODO LZU
				//LOG(boost::log::trivial::error) << "** OVERFLOW ** too many events in " << ToString();
			}
			Events[LatestIndex] = std::move(ev);
			Times[LatestIndex] = time;
			// TODO LZU
			//if (Inserted != nullptr) {
			//	for (auto listener : Inserted->listeners()) {
			//		listener(time, ev);
			//	}
			//}
		}

		virtual std::unique_ptr<C>& EventFor(double time) {
			int find = LatestIndex;
			// TODO: corner case (insert >50 events in short time) could cause this to be infinite loop
			while (!std::isnan(Times[find]) && time < Times[find]) {
				find--;
				if (find < 0) {
					find = Times.size() - 1;
				}
				if (find == LatestIndex) { //in case we returned to starting point - use first one
					find = 0;
					break;
				}
			}
			if (std::isnan(Times[find])) {
				find++;
				if (find >= Times.size()) {
					find = 0;
				}
			}
			// TODO: shouldn't need this when it's bug free
			if (Events[find] == nullptr) {
				LOG(boost::log::trivial::error) << "Null Events[" << find + L"] for index=" << find;
				// TODO LZU
				//LOG(boost::log::trivial::error) << "Null Events[" << find + L"] for index=" << find << " in " << this->ToString();
			}
			assert(Events[find] != nullptr);
			return Events[find];
		}

		// TODO
		//std::string ToString() {
		//	std::stringstream ss("SignalBase.Events[");
		//	for (int i = LatestIndex; i >= 0; i--) {
		//		if (Events[i] == nullptr) {
		//			ss << i << "-was null, ";
		//		}
		//		else {
		//			ss << i << "- :{ time=" << Times[i] << ", Event=" << Events[i] << "}, ";
		//		}
		//	}
		//	ss << "]";
		//	return ss.str();
		//}

		/// Note that it currently is inclusive on both ends: it will include events at exactly fromTime or toTime.
		//std::vector<C> SignalBae::EventsForRange(double fromTime, double toTime) {
		//	std::vector<C> result;
		//	int find = LatestIndex;
		//	// TODO: corner case (insert >50 events in short time) could cause this to be infinite loop
		//	while (Times[find] != 0 && fromTime <= Times[find]) {
		//		if (Times[find] <= toTime) {
		//			result.push_back(Events[find]);
		//		}
		//		find--;
		//		if (find < 0) {
		//			find = Times.size() - 1;
		//		}
		//	}
		//	return result;
		//}

		//C SignalBase::Reset(double resetTime) {
		//	// TODO: > or >= ?
		//	while (Times[LatestIndex] > resetTime) {
		//		Times[LatestIndex] = double::NaN;
		//		Events[LatestIndex] = C();
		//		LatestIndex = (LatestIndex == 0) ? Times.size() - 1 : LatestIndex - 1;
		//		// TODO: do we need to do something to invalidate any events related to these we're discarding?
		//	}
		//	auto retVal = Events[LatestIndex];
		//	if (retVal == nullptr) {
		//		DebugUtil::LogError(L"SignalBase reset not working. Reset Time is too small");
		//	}
		//	return Events[LatestIndex];
		//}

		//void SignalBase::Serialize() {
		//	if (C::typeid == TempSortedList<ISchedulable*>::typeid) {
		//		for (auto item : Events) {
		//			if (item != nullptr) {
		//				(dynamic_cast<TempSortedList<ISchedulable*>*>(item))->Serialize();
		//			}
		//		}
		//	}
		//	serializedTimes = SerializeTimes();
		//	serializedEvents = SerializeEvents();
		//}

		//double LatestTime() { return Times[LatestIndex]; }

	//private:
	//	std::vector<unsigned char> SerializeTimes() {
	//		auto list = Times.ToList();
	//		return SaveManager::Serialize(list);
	//	}
	//	std::vector<double> DeserializeTimes() {
	//		return SaveManager::Deserialize<std::vector<double>>(serializedTimes)->ToArray();
	//	}

	//	std::vector<unsigned char> SerializeEvents() {
	//		auto list = Events.ToList();
	//		auto list2 = list.Where([&] (void *x) {
	//			return x != nullptr;
	////C# TO C++ CONVERTER TODO TASK: The following lambda expression could not be converted:
	//		})->Select(x => KeyValuePair<int, C>(list.find(x), x))->ToList();
	//		return SaveManager::Serialize(list2);
	//	}
	//	std::vector<C> DeserializeEvents() {
	//		auto list = SaveManager::Deserialize<std::vector<KeyValuePair<int, C>*>>(serializedEvents);
	//		std::vector<C> array_Renamed(MAX_EVENTS);
	//		for (auto item : list) {
	//			array_Renamed[item->Key] = item->Value;
	//		}
	//		return array_Renamed;
	//	}
	//public:
	//	void Deserialize() {
	//		Times = DeserializeTimes();
	//		Events = DeserializeEvents();
	//	}
	};
}
