#include "stdafx.h"
#include "SignalBase.h"
#include "CppUnitTest.h"
#include "MenticsCommonTest.h"
#include <iostream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MenticsGame {

	class EventKlass {
	public:
		EventKlass() : i(0) {};
		EventKlass(int i) : i(i) {}
		int i;
		friend std::ostream& operator<<(std::ostream &os, const EventKlass &e);
	};

	std::ostream& operator<<(std::ostream &os, const EventKlass &e) {
		os << e.i;
		return os;
	}

	TEST_CLASS(SignalTest)
	{
		boost::log::sources::severity_logger<boost::log::trivial::severity_level> lg;
		const std::string name = "SignalTest";

	public:
		TEST_CLASS_INITIALIZE(BeforeClass) {
			setupLog();
		}

		TEST_METHOD(TestSignalBase) {
			SignalBase<EventKlass> signal(EventKlass(10));
			Assert::AreEqual(0., signal.Times[0]);
			Assert::IsTrue(std::isnan(signal.Times[MAX_EVENTS-1]));

			Assert::IsTrue(signal.Events[0].i == 10);
			Assert::IsTrue(signal.Events[1].i == 0);

			signal.InsertEvent(10, EventKlass(100));			
			Assert::AreEqual(100, signal.Events[1].i);

			auto &e = signal.EventFor(10);
			Assert::IsTrue(e.i == 100);

			{
				signal.InsertEvent(20, EventKlass(200));
				auto e = signal.EventsForRange(0, 20);
				Assert::AreEqual(2, (int)e.size());
				Assert::AreEqual(100, e[1].i);
				Assert::AreEqual(200, e[0].i);
			}

			Assert::AreEqual(20.0, signal.LatestTime());
			
			{
				auto e = signal.Reset(10);
				Assert::AreEqual(100, e.i);
				Assert::AreEqual(10.0, signal.LatestTime());
			}

			//LOG(lvl::info) << ;
		}
	};
}