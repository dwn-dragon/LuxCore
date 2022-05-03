#include "head.h"

namespace lux::test::containers
{

	TEST_CLASS(dynamic_array)
	{
	public:
		using da_type = lux::dynamic_array<int>;

		TEST_METHOD(init_ilist) {
			std::initializer_list<da_type::value_type> _data{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			da_type container{ _data };

			Assert::AreEqual(_data.size(), container.size(), L"size mismatch");
			
			auto d_it = _data.begin();
			auto c_it = container.begin();
			while (d_it != _data.end() && c_it != container.end())
				Assert::AreEqual(*(d_it++), *(c_it++), L"values mismatch");
		}
		TEST_METHOD(assign_ilist) {
			da_type container{20};

			std::initializer_list<da_type::value_type> _data{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			container = _data;

			Assert::AreEqual(_data.size(), container.size(), L"size mismatch");

			auto d_it = _data.begin();
			auto c_it = container.begin();
			while (d_it != _data.end() && c_it != container.end())
				Assert::AreEqual(*(d_it++), *(c_it++), L"values mismatch");
		}
	};

}