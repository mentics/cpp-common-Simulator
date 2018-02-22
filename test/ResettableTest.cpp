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
			Resettable<std::chrono::system_clock::time_point> R;


			for (int i = 1; i <= 5; i++) 
			{
				Entity tmp;
				tmp.value = i;
		
				R.addItem(std::chrono::system_clock::now(), &v, tmp);
				if (v.back() == tmp) log->info("matches {0}",tmp.value);
				else log->info("NO match {0}", i);
			}
			std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
			R.deleteItem(std::chrono::system_clock::now(), &v, 0);
			R.deleteItem(std::chrono::system_clock::now(), &v, 3);

			for (Entity e : v)log->info("after del : {0} ", e.value);

			R.reset(t);
			
			if (!v.empty())log->info("size : {0}", v.size()); // should be 0

			

			
		}
	};
}