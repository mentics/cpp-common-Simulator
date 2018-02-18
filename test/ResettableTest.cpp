#include "stdafx.h"
#include "Resettable.h"
#include "CppUnitTest.h"
#include "MenticsCommonTest.h"
//#include "date.h"
#include <iostream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//std::string timeToString(std::chrono::system_clock::time_point const& tp) {
//	using namespace date;
//	std::stringstream buffer;
//	buffer << tp;
//	return buffer.str();
//}

namespace MenticsGame {

	using Time = std::chrono::system_clock::time_point;

	struct Entity {
		Entity() : value(0) {}
		int value;
	};
	bool operator==(Entity const& a, Entity const &b) {
		return a.value == b.value;
	}

	TEST_CLASS(ResettableTest)
	{
		const std::string name = "ResettableTest";

	public:
		TEST_CLASS_INITIALIZE(BeforeClass) {
			//setupLog();
		}

		TEST_METHOD(TestResettableReset) {
			Entity ent;
			ent.value = 1;

			std::queue <Entity> q;
			q.push(ent);
			Resettable<Time, Entity, std::queue<Entity> , &q, push , pop > state(ent, 10);

			auto before = std::chrono::system_clock::now();

			std::vector<Time> times;
			for (int i = 0; i < 10; ++i) {
				Time now = std::chrono::system_clock::now();
				times.push_back(now);
				state.apply(Change<Entity, Time>([](Entity &e) {
					e.value = e.value * 2;
				},now));
			}

			state.reset(before);
			//Assert::AreEqual(10, (int)state.buffer.size(), L"Cannot be reset to older than the oldest state");

			state.reset(std::chrono::system_clock::now());
			//Assert::AreEqual(10, (int)state.buffer.size(), L"Cannot be reset to newer than the current state");
			
			state.reset(times.at(4));
			Assert::IsTrue(state.buffer.peek() == nullptr);
			Assert::AreEqual(32, state.stateCurrent.value);
			Assert::IsTrue(state.stateCurrent == state.stateOldest);
			Assert::IsTrue(state.timeCurrent == times.at(4));
		}
	};
}