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

		TEST_METHOD(TestResettableBufferFull) {
			Entity ent;
			Resettable<Entity, Time> state(ent,2);
			Assert::AreEqual(0, state.stateCurrent.value);
			Assert::AreEqual(state.stateCurrent.value, state.stateOldest.value);

			auto inc = [](Entity &ent) {
				ent.value += 1;
			};

			state.apply(Change<Entity, Time>(inc, std::chrono::system_clock::now()));
			//Assert::AreEqual(1, (int)state.buffer.size());
			Assert::AreEqual(0, state.stateOldest.value);
			Assert::AreEqual(1, state.stateCurrent.value);

			state.apply(Change<Entity, Time>(inc, std::chrono::system_clock::now()));
			state.apply(Change<Entity, Time>(inc, std::chrono::system_clock::now()));
			Assert::AreEqual(3, state.stateCurrent.value);
			//Assert::IsTrue(state.buffer.size_approx > something );
			Assert::AreEqual(1, state.stateOldest.value);

			//state.apply(Change<Entity, Time>(setToOne, Time())); // Assertion fails
		}

		TEST_METHOD(TestResettableMoveOldest) {
			const auto m_log = spdlog::stdout_logger_st("unique name");
			Entity ent;
			Resettable<Entity, Time> state(ent, 10);

			auto inc = [](Entity &ent) {
				ent.value += 1;
			};

			std::vector<Time> times;
			for (int i = 0; i < 10; ++i) {
				Time t = std::chrono::system_clock::now();
				times.push_back(t);
				state.apply(Change<Entity, Time>(inc, t));
			}
			Assert::AreEqual(10, state.stateCurrent.value);
			Assert::AreEqual(0, state.stateOldest.value);

			//state.walk([&](Change<Entity, Time> const& change) -> bool {
			//	LOG(lvl::info) << timeToString(change.time) << " " << timeToString(times.at(4)) << " " << (change.time <= times.at(4));
			//	return true;
			//});

			state.moveOldest(times.at(4));
			Assert::AreEqual(5, state.stateOldest.value);
			//Assert::AreEqual(5, (int)state.buffer.size());

			Entity tmp = state.stateOldest;
			
			Assert::AreEqual(tmp.value, state.stateCurrent.value);

			// Moving to time > current time leaves the buffer unchanged
			state.moveOldest(std::chrono::system_clock::now());
			//Assert::AreEqual(5, (int)state.buffer.size(),L"Buffer changed");

			state.moveOldest(times.back());
			m_log->info("{0}",state.buffer.size_approx());
			Assert::IsTrue(state.buffer.peek() == nullptr,L"Buffer is not empty");
			Assert::AreEqual(state.stateCurrent.value, state.stateOldest.value);
		}

		typedef uint64_t RealTime;
		
		TEST_METHOD(TestResettableCommand) {
			class Command {
			public:
				Command(Resettable<Entity, RealTime> *target) : target(target) {}
				void execute() {
					RealTime now = std::chrono::system_clock::now().time_since_epoch().count();
					Change<Entity, RealTime> change(action, now);
					target->apply(change);
				}
			protected:
				Action<Entity> action;
				Resettable<Entity, RealTime> *target;
			};

			class IncrementCommand : public Command {
			public:
				IncrementCommand(Resettable<Entity, RealTime> *target) : Command(target) {
					action = [](Entity &e) {
						e.value++;
					};
				}
			};

			Entity ent;
			ent.value = 1;
			Resettable<Entity, RealTime> state(ent, 10, 0);

			IncrementCommand inc(&state);
			inc.execute();
			inc.execute();
			Assert::AreEqual(1, state.stateOldest.value);
			Assert::AreEqual(3, state.stateCurrent.value);
		}

		TEST_METHOD(TestResettableReset) {
			Entity ent;
			ent.value = 1;
			Resettable<Entity, Time> state(ent, 10);

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