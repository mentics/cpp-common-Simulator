#pragma once

#include <string>
#include <vector>
#include <cmath>
#include "MenticsCommon.h"

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
		std::vector<unsigned char> serializedTimes;
		std::vector<unsigned char> serializedEvents;
	public:
		int LatestIndex = 0;
		std::vector<C> Events;
		SignalBase(C const& initial);
		virtual void InsertEvent(double time, C const& ev);
		virtual C EventFor(double time);
		std::vector<C> EventsForRange(double fromTime, double toTime);
		C Reset(double resetTime);
		double LatestTime();


	};
}
