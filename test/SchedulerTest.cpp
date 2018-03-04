#include "stdafx.h"

#include <thread>

#include "MenticsCommonTest.h"
#include "Scheduler.h" // This should be the only place that includes this at this level

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono_literals;

namespace MenticsGame { namespace scheduler {

typedef uint64_t TimeType;

struct TestModel {
	void reset(TimeType resetToTime) {}
};

struct TestTimeProvider : public SchedulerTimeProvider<TimeType> {
	TimeType max = 2000;
	chrono::nanoseconds until = chrono::nanoseconds(500);

	TimeType now() {
		return 0;
	}

	TimeType maxTimeAhead() {
		return max;
	}
	chrono::nanoseconds realTimeUntil(TimeType t) {
		return until;
	}
};

class TestEvent : public Event< TestModel, TimeType> {
public:
	TestEvent(const TimeType created, const TimeType runAt) : Event(created, runAt) {}

	void run(SchedulatorPtr<TestModel,TimeType> sched, nn::nn<TestModel*> model) {
		//BOOST_LOG_SEV(sched->lg, boost::log::trivial::trace) << "TestEvent for " << runAt;
	}

};

TEST_CLASS(SchedulerTest)
{
public:
	TEST_CLASS_INITIALIZE(BeforeClass) {
		setupLog();
	}

	TEST_METHOD(TestScheduler) {
		
		TestTimeProvider timeProvider;
		SchedulerModel<TimeType,TestModel> schedModel("SchedulerModel");
		TestModel model;
//		Scheduler<TestModel, TimeType> sched("Scheduler", nn::nn_addr(schedModel), nn::nn_addr(timeProvider), nn::nn_addr(model));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimeType>, TestEvent>(1, 1));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimeType>, TestEvent>(2, 2));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimeType>, TestEvent>(3, 3));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimeType>,TestEvent>(4, 4));
		std::this_thread::sleep_for(100ms);
		TimeType t = 1; 
		
		//schedModel.consumeOutgoing([&t, &sched](auto ev, 5) {
		//	log->trace("checking {0}", ev->occursAt);
		//
		//	Assert::AreEqual(t, ev->occursAt);
		//	t++;
		//});
		//sched.stop();
	}

};

}}