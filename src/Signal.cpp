#include "stdafx.h"
#include "Signal.h"


namespace MenticsGame
{
	template<typename T, typename TimeType>
	TimeType ValueSignal<T, TimeType>::oldest = 0;
}