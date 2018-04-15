#pragma once

#include "MenticsCommon.h"

namespace MenticsGame {

template <typename TimeType>
struct OutEvent {
	const TimeType occursAt;

	OutEvent(const TimeType occursAt) : occursAt(occursAt) {}
};
PTRS1(OutEvent, TimeType)

}
