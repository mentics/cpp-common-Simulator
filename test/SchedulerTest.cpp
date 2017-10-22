#include "stdafx.h"

#include <thread>

#include "MenticsCommonTest.h"
#include "Scheduler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono_literals;

namespace mentics { namespace scheduler {

typedef uint64_t TimeType;

struct TestTimeProvider : public SchedulerTimeProvider<TimeType> {
	TimeType max = 2000;
	TimeType until = 500;

	TimeType maxTimeAhead() {
		return max;
	}
	TimeType realTimeUntil(TimeType t) {
		return until;
	}
};

class TestEvent : public Event<TimeType> {
private:
	TimeType runAt;
public:
	TestEvent(TimeType runAt) : runAt(runAt) {}

	TimeType timeToRun() {
		return runAt;
	}

	void run(Schedulator<TimeType>* sched) {
		//BOOST_LOG_SEV(sched->lg, boost::log::trivial::trace) << "TestEvent for " << runAt;
	}
};

TEST_CLASS(SchedulerTest)
{
public:
	TEST_CLASS_INITIALIZE(BeforeClass) {
		mentics::test::setupLog();
	}

	TEST_METHOD(TestScheduler)
	{
		TestTimeProvider timeProvider;
		SchedulerModel<TimeType> model("SchedulerModel");
		Scheduler<TimeType> sched("Scheduler", &model, (SchedulerTimeProvider<TimeType>*)&timeProvider);
		model.schedule(new TestEvent(1));
		model.schedule(new TestEvent(2));
		model.schedule(new TestEvent(3));
		model.schedule(new TestEvent(4));
		std::this_thread::sleep_for(100ms);
		TimeType t = 1;
		model.consumeOutgoing(5, [&t, &sched](auto ev) {
			BOOST_LOG_SEV(sched.lg, boost::log::trivial::trace) << "checking " << ev->timeToRun();

			Assert::AreEqual(t, ev->timeToRun());
			t++;
		});
		sched.stop();
	}

};

}}