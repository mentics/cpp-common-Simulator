#pragma once

#include <functional>

#include "Signal.h"

namespace MenticsGame {

	template<typename A>
	using SignalFunction = std::function<A(double)>;
	using SignalFunction2 = std::function<double(double, double)>;

	/// Convenience functions for Signals.
	class SignalFunctions {
	public:
		template<typename A>
		static SignalFunction<A> ConstantFunction(A value) {
			return [&](double time) -> A {
				return value;
			};
		}

		static SignalFunction2 IncreasingFunction(double startGameTime, double initial, double rate, double max) {
			return [&](double currentEnergy, double time) -> double {
				double n = initial + rate * (time - startGameTime);
				return n > max ? max : n;
			};
		}

		static SignalFunction2 TransferEnergyFunction(double startGameTime, double endGameTime, double rate) {
			return [&](double currentEnergy, double gameTime) -> double {
				if (gameTime >= startGameTime) {
					if (gameTime > endGameTime) {
						gameTime = endGameTime;
					}
					return currentEnergy + rate * (gameTime - startGameTime);
				} else {
					return currentEnergy;
				}
			};
		}
	};
}