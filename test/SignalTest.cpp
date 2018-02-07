#include "stdafx.h"
#include "SignalBase.h"
#include "Signal.h"
#include "CompositeSignal.h"
#include "SignalFunctions.h"
#include "CppUnitTest.h"
#include "MenticsCommonTest.h"
#include <iostream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MenticsGame {

	class EventClass {
	public:
		EventClass() : i(0) {};
		EventClass(int i) : i(i) {}
		int i;
		friend std::ostream& operator<<(std::ostream &os, const EventClass &e);
	};

	std::ostream& operator<<(std::ostream &os, const EventClass &e) {
		os << e.i;
		return os;
	}

	TEST_CLASS(SignalTest)
	{
		const std::string name = "SignalTest";

	public:
		TEST_CLASS_INITIALIZE(BeforeClass) {
			setupLog();
		}

		TEST_METHOD(TestSignalBase) {
			SignalBase<EventClass> signal(EventClass(10));
			Assert::AreEqual(0., signal.Times[0]);
			Assert::IsTrue(std::isnan(signal.Times[MAX_EVENTS-1]));

			Assert::IsTrue(signal.Events[0].i == 10);
			Assert::IsTrue(signal.Events[1].i == 0);

			signal.InsertEvent(10, EventClass(100));			
			Assert::AreEqual(100, signal.Events[1].i);

			auto &e = signal.EventFor(10);
			Assert::IsTrue(e.i == 100);

			{
				signal.InsertEvent(20, EventClass(200));
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

		TEST_METHOD(TestCompositSignal) {
			CompositeSignal<EventClass> cs(EventClass(100));
			cs.InsertEvent(2, EventClass(200));
			cs.InsertEvent(5, EventClass(300));
			auto e = cs.GetTimes(1,5);
			Assert::AreEqual(2, (int)e.size());
			Assert::AreEqual(300.0, e[0]);
			Assert::AreEqual(200.0, e[1]);
		}

		TEST_METHOD(TestSignal) {
			auto initial = [](double time) -> EventClass {
				return time;
			};
			Signal<EventClass> signal(initial);
		}

		TEST_METHOD(TestSignalFunction) {
			EventClass k;
			auto initial = SignalFunctions::ConstantFunction<EventClass>(k);
			auto f = SignalFunctions::IncreasingFunction(10,20,5.5,8);
			auto f2 = SignalFunctions::TransferEnergyFunction(10, 20, 4.5);
		}
	};
}