#pragma once

#include <lux/base_core.h>

namespace lux
{

	constexpr auto PI = M_PI;

	template< class Ty, class Comp = std::less<Ty> >
	inline Ty&& min(Ty&& left, Ty&& right, Comp comp = Comp{}) {
		return comp(left, right) ? std::forward<Ty>(left) : std::forward<Ty>(right);
	}
	template< class Ty, class Comp = std::greater<Ty> >
	inline Ty&& max(Ty&& left, Ty&& right, Comp comp = Comp{}) {
		return comp(left, right) ? std::forward<Ty>(left) : std::forward<Ty>(right);
	}

	/**
	 * @brief Calculate the hypotenuse with the catheti
	 * @param x Cathetus \c double
	 * @param y Cathetus \c double
	 * @return Hypotenuse \c double
	*/
	inline double pyth_theorem(double x, double y) {
		return std::sqrt((x * x) + (y * y));
	}

	template< class Ty, class Uy >
	struct less
	{
	private:
		template< class T, class U, class = decltype(std::declval<T>() < std::declval<U>()) >
			static std::true_type _test_less(int);
			template< class T, class U >
			static std::false_type _test_less(...);

			template< class T, class U, class = decltype(std::declval<T>() > std::declval<U>()) >
		static std::true_type _test_greater(int);
		template< class T, class U >
		static std::false_type _test_greater(...);

	public:
		constexpr ~less() = default;
		constexpr less() = default;

		constexpr bool operator()(const Ty& left, const Uy& right) {
			if constexpr (decltype(_test_less<Ty, Uy>(0))::value)
				return left < right;
			else if constexpr (decltype(_test_greater<Uy, Ty>(0))::value)
				return right > left;
			else
				static_assert(false, "No valid operator for Ty < Uy");
		}
		constexpr bool operator()(const Uy& left, const Ty& right) {
			if constexpr (decltype(_test_less<Uy, Ty>(0))::value)
				return left < right;
			else if constexpr (decltype(_test_greater<Ty, Uy>(0))::value)
				return right > left;
			else
				static_assert(false, "No valid operator for Uy < Ty");
		}
	};

}
