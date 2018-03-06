#include "stdafx.h"
#include "Signal.h"


namespace MenticsGame
{
	template<typename T, typename TimeType = TimePoint>
	TimeType Signal<T, TimeType>::oldest = 0;
	
}