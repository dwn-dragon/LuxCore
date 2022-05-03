#include "head.h"

#define ERROR_MSG(TYPE) L"invalid result with \"" #TYPE L"\" type"

namespace lux::test::types
{

	TEST_CLASS(is_deref)
	{
		template< class T >
		struct _ptr
		{
			T operator*();
		};

		struct _incomplete;

		struct _virtual
		{
			virtual void method() = 0;
		};

	public:
		TEST_METHOD(fundamental_types) {
			// int

			Assert::IsTrue(not	lux::is_deref<int>::value,					ERROR_MSG(int));
			Assert::IsTrue(		lux::is_deref<int*>::value,					ERROR_MSG(int*));
			Assert::IsTrue(		lux::is_deref<_ptr<int>>::value,			ERROR_MSG(_ptr<int>));

			// void

			Assert::IsTrue(not	lux::is_deref<void>::value,					ERROR_MSG(void));
			Assert::IsTrue(not	lux::is_deref<void*>::value,				ERROR_MSG(void*));
			Assert::IsTrue(		lux::is_deref<_ptr<void>>::value,			ERROR_MSG(_ptr<void>));
		}

		TEST_METHOD(incomplete_type) {
			// incomplete type

			Assert::IsTrue(not	lux::is_deref<_incomplete*>::value,			ERROR_MSG(_incomplete*));
			Assert::IsTrue(not	lux::is_deref<_ptr<_incomplete>>::value,	ERROR_MSG(_ptr<_incomplete>));
			Assert::IsTrue(		lux::is_deref<_ptr<_incomplete*>>::value,	ERROR_MSG(_ptr<_incomplete*>));
			Assert::IsTrue(not	lux::is_deref<_ptr<_incomplete&>>::value,	ERROR_MSG(_ptr<_incomplete&>));
		}

		TEST_METHOD(virtual_type) {
			// virtual class

			Assert::IsTrue(		lux::is_deref<_virtual*>::value,			ERROR_MSG(_virtual*));
			Assert::IsTrue(not	lux::is_deref<_ptr<_virtual>>::value,		ERROR_MSG(_ptr<_virtual>));
			Assert::IsTrue(		lux::is_deref<_ptr<_virtual*>>::value,		ERROR_MSG(_ptr<_virtual*>));
			Assert::IsTrue(		lux::is_deref<_ptr<_virtual&>>::value,		ERROR_MSG(_ptr<_virtual&>));
		}

	};

}