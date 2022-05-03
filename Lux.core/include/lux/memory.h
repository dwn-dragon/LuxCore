#pragma once

#include <lux/base_core.h>

#include <lux/types.h>

#include <memory>

namespace lux
{

	template< class Ty >
	struct deref
	{
		using is_deref = lux::is_deref<Ty>;
		static_assert(is_deref::value, "Ty is not dereferencable");

		constexpr ~deref() = default;
		constexpr deref() = default;

		constexpr is_deref::type operator()(Ty&& ptr) const noexcept(noexcept(*(std::declval<Ty>()))) {
			return *(std::forward<Ty>(ptr));
		}
	};

}
