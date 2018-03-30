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

		void run(SchedulatorPtr<TestModel, TimePoint> sched, nn::nn<TestModel*> model) {
			mlog->error("TestEvent for {0}", timeToRun);
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
			
			TestTimeProvider timeProvider;
			SchedulerModel<TestModel, TimePoint> schedModel("SchedulerModel");
			TestModel model;


			Scheduler<TestModel, TimePoint> sched("Scheduler", nn::nn_addr(schedModel), nn::nn_addr(timeProvider), nn::nn_addr(model));
			TimePoint t1 = timeProvider.now() + 1000;
			mlog->info("time to run ev 1 : {0}", t1);

			TimePoint t2 = timeProvider.now() + 3000000000;
			mlog->info("time to run ev 1 : {0}", t2);

			TestEvent t = TestEvent(timeProvider.now(), t1); 

			//sched.schedule((TestEventUniquePtr)std::make_unique(t));   
			//schedModel.schedule(uniquePtrC<TestEvent>(TestEvent(timeProvider.now(), t2))); 
			//timeProvider
			std::this_thread::sleep_for(9000ms);
			//TimePoint t = 1;



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
