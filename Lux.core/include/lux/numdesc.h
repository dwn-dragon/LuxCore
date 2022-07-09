#pragma once

#include <lux/base_core.h>

namespace lux
{

	class number_descriptor
	{
	public:
		enum NUMBER_TYPE
			: uint8_t
		{
			UNKNOWN								= 0b00000000,
			UNSIGNED_INTEGER					= 0b01000000,
			SIGNED_INTEGER						= 0b10000000,
			FLOATING_POINTER					= 0b11000000,
		};

	private:
		static constexpr uint8_t TYPE_MASK		= 0b11000000;
		static constexpr uint8_t LENGTH_MASK	= 0b00111111;

		uint8_t _data;

	public:
		constexpr number_descriptor()
			: _data(UNKNOWN | 0) {
		}
		constexpr number_descriptor(uint8_t type, uint8_t length)
			: number_descriptor() {
			this->type(type);
			this->length(length);
		}

		constexpr uint8_t type() const noexcept {
			return _data & TYPE_MASK;
		}
		constexpr uint8_t type(uint8_t _type) noexcept {
			//	Cleans the new type of unneeded bytes
			_type &= TYPE_MASK;
			//	Removes the current type
			_data &= ~TYPE_MASK;
			//	Sets the new type
			_data |= _type;
			//	Returns the current type
			return type();
		}

		constexpr uint8_t length() const noexcept {
			return _data & LENGTH_MASK;
		}
		constexpr uint8_t length(uint8_t _len) noexcept {
			//	Cleans the new length of unneeded bits
			_len &= LENGTH_MASK;
			//	Removes the current length
			_data &= ~LENGTH_MASK;
			//	Sets the new length
			_data |= _len;
			//	Returns the current length
			return length();
		}
	};

}
