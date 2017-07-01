#include "stdafx.h"
#include "CppUnitTest.h"

#include <thread>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "Scheduler.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono_literals;

namespace mentics { namespace scheduler {

typedef uint32_t TimeType;

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
		BOOST_LOG_SEV(sched->lg, boost::log::trivial::trace) << "TestEvent for " << runAt;
	}
};

TEST_CLASS(SchedulerTest)
{
public:
	TEST_CLASS_INITIALIZE(BeforeClass) {
		auto sink = boost::log::add_file_log("unit-test.log");
		sink->locked_backend()->auto_flush(true);
		boost::log::core::get()->set_filter
		(
			boost::log::trivial::severity >= boost::log::trivial::trace
		);
		boost::log::add_common_attributes();
	}

	TEST_METHOD(TestScheduler)
	{
		TestTimeProvider timeProvider;
		SchedulerModel<TimeType> model(1000000000);
		Scheduler<TimeType> sched(&model, (SchedulerTimeProvider<TimeType>*)&timeProvider);
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