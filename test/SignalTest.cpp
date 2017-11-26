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
		EventKlass(int i) : i(i) {}
		int i;
	};

	TEST_CLASS(SignalTest)
	{
		boost::log::sources::severity_logger<boost::log::trivial::severity_level> lg;
		const std::string name = "SignalTest";

	public:
		TEST_CLASS_INITIALIZE(BeforeClass) {
			setupLog();
		}

		TEST_METHOD(TestSignalBase) {
			auto k = std::make_unique<EventKlass>(1);
			SignalBase<EventKlass> signal(std::move(k));
			Assert::AreEqual(0., signal.Times[0]);
			Assert::IsTrue(std::isnan(signal.Times[MAX_EVENTS-1]));

			Assert::IsFalse(signal.Events[0] == nullptr);
			Assert::IsTrue(signal.Events[1] == nullptr);

			signal.InsertEvent(10, std::make_unique<EventKlass>(100));
			auto &e = signal.EventFor(10);
			Assert::IsTrue(e->i == 100);
		}
	};
}