#include "stdafx.h"
#include "SignalBase.h"


namespace MenticsGame {


	template<typename C>
	SignalBase<C>::SignalBase(C const & initial) : CanLog(std::string("SignalBase")), Times(MAX_EVENTS, std::numeric_limits<double>::quiet_NaN()), Events(MAX_EVENTS) {
		// TODO Is assert required?
		// assert(initial != nullptr);
		// There must be an initial because there must always have been a value.
		Events[0] = initial;
		Times[0] = 0.0;
	}


	template<typename C>
	void SignalBase<C>::InsertEvent(double time, C const & ev) {
		assert(time >= 0);
		// assert(ev != nullptr); 
		// TODO: this is exact same code as in CompositeSignal--remove dupe code
		// NOTE: I'm not sure if this is exactly right to remove anything past the new event, but I think it is.
		while (Times[LatestIndex] > time) {
			// C must implement operator << if we want to use it like below
			//LOG(boost::log::trivial::debug) << "Throwing away event for " << Times[LatestIndex] << ": " << Events[LatestIndex];
			Times[LatestIndex] = std::numeric_limits<double>::quiet_NaN();
			// C# code use default(C)
			Events[LatestIndex] = C();
			LatestIndex = (LatestIndex == 0) ? Times.size() - 1 : LatestIndex - 1;
			// TODO: do we need to do something to invalidate any events related to these we're discarding?
		}
		// If exactly the same time, replace the old one
		if (Times[LatestIndex] != time) {
			LatestIndex = (LatestIndex == Times.size() - 1) ? 0 : LatestIndex + 1;
		}
		if (LatestIndex >= MAX_EVENTS - 2) {
			// TODO LZU output this class when ToString is ported
			m_log->error("** OVERFLOW ** too many events in ...");
		}
		Events[LatestIndex] = ev;
		Times[LatestIndex] = time;
	}


	template<typename C>
	C SignalBase<C>::EventFor(double time) {
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
		//if (Events[find] == nullptr) {
		//	// TODO LZU
		//	LOG(boost::log::trivial::error) << "Null Events[" << find + L"] for index=" << find << " in " << this->ToString();
		//}
		//assert(Events[find] != nullptr);
		return Events[find];
	}

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

	///// Note that it currently is inclusive on both ends: it will include events at exactly fromTime or toTime.

	template<typename C>
	std::vector<C> SignalBase<C>::EventsForRange(double fromTime, double toTime) {
		std::vector<C> result;
		int find = LatestIndex;
		// TODO: corner case (insert >50 events in short time) could cause this to be infinite loop
		while (Times[find] != 0 && fromTime <= Times[find]) {
			if (Times[find] <= toTime) {
				result.push_back(Events[find]);
			}
			find--;
			if (find < 0) {
				find = Times.size() - 1;
			}
		}
		return result;
	}

	template<typename C>
	C SignalBase<C>::Reset(double resetTime) {
		// TODO: > or >= ?
		while (Times[LatestIndex] > resetTime) {
			Times[LatestIndex] = std::numeric_limits<double>::quiet_NaN();
			Events[LatestIndex] = C();
			LatestIndex = (LatestIndex == 0) ? Times.size() - 1 : LatestIndex - 1;
			// TODO: do we need to do something to invalidate any events related to these we're discarding?
		}
		// TODO LZU how to check that an event is "null"?
		//auto retVal = Events[LatestIndex];
		//if (retVal == nullptr) {
		//	LOG(boost::log::trivial::error) << "SignalBase reset not working. Reset Time is too small";
		//}
		return Events[LatestIndex];
	}

	template<typename C>
	double SignalBase<C>::LatestTime() { return Times[LatestIndex]; }




	// TODO LZU port serialization?

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