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

		TEST_METHOD(TestValueSignal) {
			ValueSignal<int, int> vs;
			vs.add(3, 1);
			if (vs.get(1) != 3) {
				log->info("added {}", vs.get(1));
				Assert::Fail();
			}
			vs.undo();
		
		


		}

	};
}