#include "stdafx.h"
#include "Signal.h"
#include "CompositeSignal.h"

#include "CppUnitTest.h"
#include "MenticsCommonTest.h"
#include <iostream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MenticsGame {

	//class EventClass {
	//public:
	//	EventClass() : i(0) {};
	//	EventClass(int i) : i(i) {}
	//	int i;
	//	friend std::ostream& operator<<(std::ostream &os, const EventClass &e);
	//};
	//
	//std::ostream& operator<<(std::ostream &os, const EventClass &e) {
	//	os << e.i;
	//	return os;
	//}

	TEST_CLASS(SignalTest)
	{
		const std::string name = "SignalTest";

	public:

		TEST_METHOD(TestSignal) {
			setupLog();
			SignalValue<int, int> vs(0);
			vs.add(3, 3);
			vs.add(4, 5);
			if (vs.get(4) != 3) {
				mlog->info("added {}", vs.get(4));
				Assert::Fail();
			}
		}

		TEST_METHOD(TestSignalZero)
		{
			setupLog();
			SignalValue<int, int> vs(0);
			
			vs.get(0);
		}

		

		TEST_METHOD(TestFunctionSignal)
		{
			setupLog();
			

			//SignalFunction<int, int> fs([](int i) {return 0; });
			
			 
			
			

		}

		TEST_METHOD(TestSignalRemoveOldest)
		{
			setupLog();
			SignalValue<int, int> vs(0);
			vs.add(3, 1);
			vs.add(4, 2);
			vs.add(5, 3);
			vs.add(6, 4);

			vs.removeOldest(2);
			
			for (int i = 1; i < 5; i++)mlog->info("{0}", vs.get(i));


		}

	};
}