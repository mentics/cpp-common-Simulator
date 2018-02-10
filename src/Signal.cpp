#include "stdafx.h"
#include "Signal.h"

namespace MenticsGame {
	
	
	template<typename A>
	inline A Signal<A>::At(double time)
	{
		return EventFor(time)(time);
	}

};