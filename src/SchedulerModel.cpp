#include "stdafx.h"
#include "SchedulerModel.h"

using namespace MenticsGame;

template <typename Model, typename TimeType>
void SchedulerModel<Model, TimeType>::schedule(EventUniquePtr<Model, TimeType>&& ev) {
	mlog->trace("Scheduling event");
	incoming.enqueue(std::move(ev));
}

template <typename Model, typename TimeType>
TimeType SchedulerModel<Model, TimeType>::processIncoming() {
	mlog->trace("SchedulerModel::processIncoming");
	// TODO: assert scheduler thread?
	EventUniquePtr<Model, TimeType> ev = uniquePtr<EventZero<Model, TimeType>>(); // TODO: simplify this?
	while (incoming.try_dequeue(ev)) {
		processing.push(std::move(ev));
	}
	return !processing.empty() ? processing.top()->timeToRun : FOREVER;
}

template <typename Model, typename TimeType>
void SchedulerModel<Model, TimeType>::reset(TimeType toTime) {
	// TODO
	// TODO: delete items off outgoing > toTime
	
	//untested
	for(int i = 0; i < outgoing.size(); i++ )
	{
		if (outgoing[i]->occursAt > toTime) 
		{
			outgoing.pop_back(); 
		}
	}
}

template <typename Model, typename TimeType>
Event<Model, TimeType>* SchedulerModel<Model, TimeType>::first(TimeType maxTime) {
	if (!processing.empty()) {
		Event<Model, TimeType>* top = processing.top().get();
		const TimeType nextTime = top->timeToRun;
		if (nextTime <= maxTime) {
			return top;
		}
	}
	return nullptr;
}

template <typename Model, typename TimeType>
void SchedulerModel<Model, TimeType>::completeFirst() {
	forReset.push_back(processing.pop());
}

template <typename Model, typename TimeType>
void SchedulerModel<Model, TimeType>::addOutEvent(OutEventUniquePtr<TimeType>&& outEvent) {
	outgoing.push_back(std::move(outEvent));
}

template <typename Model, typename TimeType>
void SchedulerModel<Model, TimeType>::consumeOutgoing(std::function<void(OutEventPtr<TimeType>)> handler, TimeType upToTime) {
	while (!outgoing.empty()) {
		OutEventPtr<TimeType> ev = NN_CHECK_ASSERT(outgoing.front().get());
		if (ev->occursAt <= upToTime) {
			mlog->error("inside consumeoutgoing");
			handler(ev);
			outgoing.pop_front();

			SignalValue<Model, TimeType>::oldest = upToTime - maxTimeAhead;
			// Finally after travelling through 3 queues, the event's eventful life has come to an end.
			// delete ev; <- it's deleted by unique_ptr
		}
		else {
			break;
		}
	}
}
