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
	TimeType max;
	TimeType until;

	TimeType maxTime() {
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
		SchedulerModel<TimeType> model;
		Scheduler<TimeType> sched(&model, (SchedulerTimeProvider<TimeType>*)&timeProvider);
		model.schedule(new TestEvent(1));
		sched.stop();
	}

};

}}