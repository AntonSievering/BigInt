#pragma once

#include <cstdint>

namespace math
{
	struct int_t
	{
		union
		{
			uint64_t u64{};
			uint32_t u32[2];
			uint16_t u16[4];
			uint8_t  u8[8];
		};

	public:
		constexpr int_t() noexcept = default;

		constexpr int_t(const uint64_t value) noexcept
		{
			u64 = value;
		}

		constexpr int_t(const uint32_t val1, const uint32_t val2) noexcept
		{
			u32[0] = val1;
			u32[1] = val2;
		}

	public:
		constexpr int_t operator~() const noexcept
		{
			return ~u64;
		}

		constexpr int_t operator<<(const size_t nBits) const noexcept
		{
			return u64 << nBits;
		}

		constexpr int_t operator>>(const size_t nBits) const noexcept
		{
			return u64 >> nBits;
		}

		constexpr int_t operator|(const int_t rhs) const noexcept
		{
			return u64 | rhs.u64;
		}
	};
}
