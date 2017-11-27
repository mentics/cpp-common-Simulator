#pragma once

#include "SignalBase.h"
#include <vector>
#include <cmath>
//#include <type_traits>

namespace MenticsGame {

	/// The value of this Signal has changing values inside it at certain times.
	/// @param <C> type of the value in this Signal
	template<typename C>
	class CompositeSignal : public SignalBase<C> {
	public:
		CompositeSignal(C initial) : SignalBase<C>(initial) {
		}

		virtual std::vector<double> GetTimes(double fromTime, double toTime) {
			std::vector<double> result;
			int find = LatestIndex;
			while (Times[find] > fromTime && Times[find] < toTime) {
				assert(!std::isnan(Times[find]));
				result.push_back(Times[find]);
				find--;
				if (find < 0) {
					find = Times.size() - 1;
				}
			}
			return result;
		}
	};
}
