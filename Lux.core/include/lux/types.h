#pragma once

#include <lux/base_core.h>

#include <type_traits>
#include <ostream>

namespace lux
{

	namespace _impls
	{

		template< class T >
		struct _complete_test
		{
			template< class T, size_t = sizeof(T) >
			static constexpr std::true_type _test(int);
			template< class T >
			static constexpr std::false_type _test(...);

			static constexpr bool value = decltype(_test<T>(0))::value;
		};

	}

	template< class Ty >
	_INLINE_VAR constexpr bool is_complete_v = _impls::_complete_test<Ty>::value;

	template< class Ty >
	struct is_complete
		: std::bool_constant<is_complete_v<Ty>>
	{
	};

	namespace _impls
	{

		template< class T >
		struct _deref_test
		{
			template< class T, class = decltype(*std::declval<T>()) >
			static std::true_type _pure_test(int);
			template< class T >
			static std::false_type _pure_test(...);

			static constexpr bool _test() {
				if constexpr (decltype(_pure_test<T>(0))::value == false)
					return false;
				// 
				else if constexpr (std::is_same_v<decltype(*std::declval<T>()), void>)
					return true;
				//// a reference is not considered a complete type by default
				//else if constexpr (std::is_reference_v<decltype(*std::declval<T>())>)
				//	return true;
				// 
				else if constexpr (is_complete_v<decltype(*std::declval<T>())>)
					return true;
				// default
				else 
					return false;
			}

			static constexpr bool value = _test();

			template< class T, bool Res, std::enable_if_t<Res, int> = 0 >
			static decltype(*std::declval<T>()) _type_test(int);
			template< class T, bool Res >
			static void _type_test(...);

			using type = decltype(_type_test<T, value>(0));
		};

	}

	template< class Ty >
	struct is_deref
		: std::bool_constant<_impls::_deref_test<Ty>::value>
	{
		using type = typename _impls::_deref_test<Ty>::type;
	};

	template< class Ty >
	_INLINE_VAR constexpr bool is_deref_v = is_deref<Ty>::value;

	template< class Ty >
	using deref_t = typename is_deref<Ty>::type;

}
