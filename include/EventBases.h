#pragma once

#include "MenticsCommon.h"

namespace MenticsGame {

	enum EventType{EventQuipCreated};

	template <typename TimeType>
	struct OutEvent {
		const TimeType occursAt;
		EventType type;
		OutEvent(const TimeType occursAt, EventType t) : occursAt(occursAt), type(t) {}
	};
	PTRS1(OutEvent, TimeType)

}
