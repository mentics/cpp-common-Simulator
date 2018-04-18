#include "stdafx.h"

#include <thread>

#include "MenticsCommonTest.h"
#include "Scheduler.h"
#include "nn.hpp"

#include "Scheduler.cpp"  // This should be the only place that includes this at this level

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

bool ran = false;
TestTimeProvider timeProvider;

class TestEvent : public Event<TestModel, TimePoint> {
public:
	//TestEvent(const TimePoint created, const TimePoint runAt) : Event(created, runAt) {}

	void run(SchedulatorPtr<TestModel, TimePoint> sched, nn::nn<TestModel*> model) {
		mlog->error("TestEvent for {0}", timeToRun);
		if (timeToRun + 0.5 > timeProvider.now()) mlog->error(" run at runAt");
		OutEvent<TimePoint> e(0, EventQuipCreated); // it should not be EventQuipCreated but for now

		sched->addOutEvent(uniquePtrC<OutEvent<TimePoint>, OutEvent<TimePoint>>(100));

		system("color 00");
		ran = true;
		//exit(0);
	}
};
PTRS(TestEvent);


TEST_CLASS(SchedulerTest)
{
public:
	TEST_CLASS_INITIALIZE(BeforeClass) {
	}

	TEST_METHOD(TestScheduler) {
		setupLog();
		mlog->error("TestEvent for");
		SchedulerModel<TestModel, TimePoint> schedModel;
		TestModel model;
		Scheduler<TestModel, TimePoint> sched(nn::nn_addr(schedModel), nn::nn_addr(timeProvider), nn::nn_addr(model));

		for (int i = 0; i < 50; i++) {
			sched.schedule(i, uniquePtrC<TestEvent, TestEvent>());
		}

		sched.reset(0);

		int counter = 0;
		int counter2 = 0;

		sched.WaitUntilProcessed(100);

		schedModel.consumeOutgoing([&](OutEventPtr<TimePoint> e) {counter++; }, 100);

		schedModel.consumeOutgoing([&](OutEventPtr<TimePoint> e) {counter2++; }, 100); // see if the queue is empty

		mlog->error("counter = {0}", counter); // should be 50
		mlog->error("counter2 = {0}", counter2); // should be 0

		if (!ran || counter == 0 || counter2 > 0)
		{
			Assert::Fail();
		}

		
		//schedModel.consumeOutgoing([&t, &sched](auto ev, 5) {
		//	log->trace("checking {0}", ev->occursAt);
		//
		//	Assert::AreEqual(t, ev->occursAt);
		//	t++;
		//});
		//sched.stop();
	}

	TEST_METHOD(TestSchedulerModel)
	{

	}

};

}
