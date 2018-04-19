#include "stdafx.h"
#include "Resettable.h"
#include "CppUnitTest.h"
#include "MenticsCommonTest.h"
//#include "date.h"
#include <iostream>
#include <string>
#include "SignalStack.h"
#include "../src/SignalStack.cpp"

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


		TEST_METHOD(TestSignalStack) 
		{
			setupLog();
			
			SignalStack<int, int> rs;

			rs.push(0, 0);
			rs.push(77, 0);
			
			rs.push(1, 2);
			rs.push(2, 4);

			mlog->error(rs.peek());
			if (rs.peek() != 2)Assert::Fail();

			rs.pop(4);

			mlog->error(rs.peek());
			if (rs.peek() != 1) Assert::Fail();

			rs.reset(1);
			mlog->error(rs.peek());
			if (rs.peek() != 77) Assert::Fail();




		}

		//TEST_METHOD(TestResettableReset) {
		//	setupLog();
		//	std::vector<Entity> v;
		//	Resettable<uint64_t> R;
		//
		//
		//	Entity e;
		//	e.value = 5555;
		//
		//	R.addItem(1, &v, e);
		//	R.changeValue(2, &e.value, 4444);
		//	R.addItem(2, &v, e);
		//	R.changeValue(2, &e.value, 3333);
		//	R.addItem(2, &v, e);
		//	R.deleteItem(3, &v, 2);
		//	R.deleteItem(3, &v, 1);
		//	
		//	R.reset(2);
		//	
		//	if(e.value != v[0].value)
		//	{
		//		mlog->info("Entity {0} vector {1}", e.value, v[0].value);
		//		Assert::Fail(L"Fisrt Entity object value does not match vecotr [0] value ");
		//	}
		//	
		//
		//	R.reset(0);
		//
		//	
		//	for (Entity s : v)mlog->info("val : {0}", s.value);
		//
		//	if (!v.empty())
		//	{
		//		mlog->error("size : {0} should be 0", v.size());
		//		Assert::Fail(L"size of vector should be 0");
		//	};
		//
		//	
		//
		//	
		//}
	};
}