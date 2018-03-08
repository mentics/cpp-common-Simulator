#include "stdafx.h"

#include <thread>

#include "MenticsCommonTest.h"
#include "Scheduler.h" // This should be the only place that includes this at this level
#include "Scheduler.cpp"  

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono_literals;

namespace MenticsGame {

struct TestModel {
	void reset(TimePoint resetToTime) {}
};

struct TestTimeProvider : public SchedulerTimeProvider<TimePoint> {
	TimePoint max = 2000;
	chrono::nanoseconds until = chrono::nanoseconds(500);

	TimePoint now() {
		return 0;
	}

	TimePoint maxTimeAhead() {
		return max;
	}
	chrono::nanoseconds realTimeUntil(TimePoint t) {
		return until;
	}
};

class TestEvent : public Event< TestModel, TimePoint> {
public:
	TestEvent(const TimePoint created, const TimePoint runAt) : Event(created, runAt) {}

	void run(SchedulatorPtr<TestModel,TimePoint> sched, nn::nn<TestModel*> model) {
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
		SchedulerModel<TestModel,TimePoint> schedModel("SchedulerModel");
		TestModel model;
		Scheduler<TestModel,TimePoint> sched("Scheduler", nn::nn_addr(schedModel), nn::nn_addr(timeProvider), nn::nn_addr(model));
		sched.schedule(uniquePtrC<Event<TestModel, TimePoint>, TestEvent>(1, 1));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimePoint>, TestEvent>(1, 1));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimePoint>, TestEvent>(2, 2));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimePoint>, TestEvent>(3, 3));
		schedModel.schedule(uniquePtrC<Event<TestModel, TimePoint>,TestEvent>(4, 4));
		std::this_thread::sleep_for(100ms);
		TimePoint t = 1; 
		
		//schedModel.consumeOutgoing([&t, &sched](auto ev, 5) {
		//	log->trace("checking {0}", ev->occursAt);
		//
		//	Assert::AreEqual(t, ev->occursAt);
		//	t++;
		//});
		//sched.stop();
	}

};

}