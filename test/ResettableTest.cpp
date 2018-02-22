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
		//const std::string name = "ResettableTest";

	public:
		TEST_CLASS_INITIALIZE(BeforeClass) {
			//setupLog();
		}

		TEST_METHOD(TestResettableReset) {
			setupLog();
			std::vector<Entity> v;
			Resettable<uint64_t> R;


			Entity e;
			e.value = 5555;

			R.addItem(1, &v, e);
			R.changeValue(2, &e.value, 4444);
			R.addItem(2, &v, e);
			R.changeValue(2, &e.value, 3333);
			R.addItem(2, &v, e);
			R.deleteItem(3, &v, 2);
			R.deleteItem(3, &v, 1);
			
			R.reset(2);
			
			if(e.value != v[0].value)
			{
				log->info("Entity {0} vector {1}", e.value, v[0].value);
				Assert::Fail(L"Fisrt Entity object value does not match vecotr [0] value ");
			}
			

			R.reset(0);

			
			for (Entity s : v)log->info("val : {0}", s.value);

			if (!v.empty())
			{
				log->error("size : {0} should be 0", v.size());
				Assert::Fail(L"size of vector should be 0");
			};

			

			
		}
	};
}