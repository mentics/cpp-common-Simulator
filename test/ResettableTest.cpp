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
			std::vector<Entity> v;
			std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
			Resettable<std::chrono::system_clock::time_point> R;

			for (int i = 1; i <= 5; i++) 
			{
				Entity tmp;
				tmp.value = i;
		
				R.addItem(std::chrono::system_clock::now(), &v, tmp);

				
			}

			R.reset(t);
			for(int i = 0; i < 5; i++)
			{
				if (!v.empty())
				Logger::WriteMessage("there is a v");
			}

			

			
		}
	};
}