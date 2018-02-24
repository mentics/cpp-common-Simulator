#pragma once
#include "SignalBase.h"
#include "SignalFunctions.h"


namespace MenticsGame {
	/// Signal represents a time varying value that we can query at a specific time.
	/// 
	/// Signals are a concept in reactive programming. I don't know if we should consider using a full blown reactive library
	/// (I'm not sure they would support level of temporal requirements we have anyway), or whether we'll need all that. I
	/// don't expect to do much composition, so I think this simple thing should be sufficient.
	/// @param <A>
	///            type of the value in this Signal

	template<typename A>
	class Signal : public SignalBase<SignalFunction<A>> {
	public:
		Signal() {}
		Signal(SignalFunction<A> initial) : SignalBase<SignalFunction<A>>(initial) { }

		virtual A At(double time) {
			return EventFor(time)(time);
		}
	};
}
