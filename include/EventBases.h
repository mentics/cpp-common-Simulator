#pragma once

#include "MenticsCommon.h"

template <typename TimeType>
struct OutEvent {
	const TimeType occursAt;

	OutEvent(const TimeType occursAt) : occursAt(occursAt) {}
};
PTRS1(OutEvent, TimeType)
