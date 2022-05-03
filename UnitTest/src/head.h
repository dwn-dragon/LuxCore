#pragma once

#include <lux/lux_core.h>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace lux::test
{

	inline std::wstring wchar_convert(std::string&& msg) {
		std::wstringstream wss;
		wss << msg.c_str();

		return wss.str();
	}

	using simple_t = int;

	struct complex_t
	{
		complex_t()
			: x(0), y(0) {
		}
		complex_t(simple_t x, simple_t y)
			: x(x), y(y) {
		}

		complex_t(const complex_t&) = default;
		complex_t(complex_t&&) = default;

		complex_t& operator=(const complex_t&) = default;
		complex_t& operator=(complex_t&&) = default;

		simple_t x, y;

		constexpr bool operator==(const complex_t& other) const noexcept {
			return y == other.y && x == other.x;
		}
		constexpr std::strong_ordering operator<=>(const complex_t& other) const noexcept {
			if (y < other.y)
				return std::strong_ordering::less;
			if (y > other.y)
				return std::strong_ordering::greater;
			return x <=> other.x;
		}
	};

}

template< >
std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString(const lux::test::complex_t& type) {
	std::wstringstream wss;
	wss << type.x << "/" << type.y;
	return wss.str();
}

#define CATCHER(Block)									\
	try {												\
		Block;											\
	} catch (const std::exception& e) {					\
		Assert::Fail(wchar_convert(e.what()).c_str());	\
	};
