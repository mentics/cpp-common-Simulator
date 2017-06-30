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


namespace mentics { namespace scheduler {

using namespace std::chrono_literals;
	//using namespace logging::trivial;
	//namespace logging = boost::log;
	//namespace src = boost::log::sources;
	//namespace sinks = boost::log::sinks;

struct TestTimeProvider : public SchedulerTimeProvider<double> {
	double maxTime() {
		return 0;
	}
	double realTimeUntil(double t) {
		return 0;
	}
};

class TestEvent : public Event<double> {
public:
	double timeToRun() {
		return 0;
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
		SchedulerModel<double> model;
		Scheduler<double> sched(&model, (SchedulerTimeProvider<double>*)&timeProvider);
		std::this_thread::sleep_for(0.5s);
		sched.stop();
	}

};

}}