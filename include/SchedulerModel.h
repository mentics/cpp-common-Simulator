#pragma once
#include "MenticsCommon.h"
#include "concurrentqueue.h"
#include "PriorityQueue.h"
#include "EventBases.h"


namespace MenticsGame {


	template <typename Model, typename TimeType> struct Event;
	PTRS2(Event, Model, TimeType)

	template <typename Model, typename TimeType>
	class Scheduler;



	template <typename Model, typename TimeType>
	class SchedulerModel {
		TimeType maxTimeAhead = 2E9;
		moodycamel::ConcurrentQueue<EventUniquePtr<Model, TimeType>> incoming;
		PriorityQueue<EventUniquePtr<Model, TimeType>, decltype(&Event<Model, TimeType>::compare)> processing;
		std::deque<EventUniquePtr<Model, TimeType>> forReset;
		std::deque<OutEventUniquePtr<TimeType>> outgoing;

	protected:
		void schedule(EventUniquePtr<Model, TimeType>&& ev);
	public:
		friend class Scheduler<Model, TimeType>;
		SchedulerModel() : incoming(1024), processing(&Event<Model, TimeType>::compare) {}
		~SchedulerModel() {
			mlog->error("SchedulerModel destructor");
		}

	
		

		// Returns minimum timeToRun of ingested events
		TimeType processIncoming();
		void reset(TimeType toTime);
		Event<Model, TimeType>* first(TimeType maxTime);
		void completeFirst();

		void addOutEvent(OutEventUniquePtr<TimeType>&& outEvent);
		void consumeOutgoing(std::function<void(OutEventPtr<TimeType>)> handler, TimeType upToTime);
	};
	PTRS2(SchedulerModel, Model, TimeType)
}