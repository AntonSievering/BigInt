#pragma once

#include <cstdint>
#include <array>

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

		constexpr int_t(const std::array<uint16_t, 4> values) noexcept
		{
			for (size_t i = 0; i < values.size(); i++)
				u16[i] = values[i];
		}

		constexpr int_t(const std::array<uint8_t, 8> values) noexcept
		{
			for (size_t i = 0; i < values.size(); i++)
				u8[i] = values[i];
		}

	public:
		constexpr int_t operator+(const int_t rhs) const noexcept
		{
			return u64 + rhs.u64;
		}

		constexpr int_t operator+=(const int_t rhs) noexcept
		{
			return u64 += rhs.u64;
		}

		constexpr int_t operator-(const int_t rhs) const noexcept
		{
			return u64 - rhs.u64;
		}

		constexpr int_t operator-=(const int_t rhs) noexcept
		{
			return u64 -= rhs.u64;
		}

		constexpr int_t operator*(const int_t rhs) const noexcept
		{
			return u64 * rhs.u64;
		}

		constexpr int_t operator*=(const int_t rhs) noexcept
		{
			return u64 *= rhs.u64;
		}

		constexpr int_t operator/(const int_t rhs) const noexcept
		{
			return u64 / rhs.u64;
		}

		constexpr int_t operator/=(const int_t rhs) noexcept
		{
			return u64 /= rhs.u64;
		}

		constexpr int_t operator%(const int_t rhs) const noexcept
		{
			return u64 % rhs.u64;
		}

		constexpr int_t operator%=(const int_t rhs) noexcept
		{
			return u64 %= rhs.u64;
		}

		constexpr int_t &operator++() noexcept
		{
			u64++;
			return *this;
		}

		constexpr const int_t operator++(int) noexcept
		{
			return u64++;
		}

		constexpr int_t &operator--() noexcept
		{
			u64--;
			return *this;
		}

		constexpr const int_t operator--(int) noexcept
		{
			return u64--;
		}

	public:
		constexpr bool operator==(const int_t rhs) const noexcept
		{
			return u64 == rhs.u64;
		}

		constexpr bool operator!=(const int_t rhs) const noexcept
		{
			return u64 != rhs.u64;
		}

		constexpr bool operator>(const int_t rhs) const noexcept
		{
			return u64 > rhs.u64;
		}

		constexpr bool operator>=(const int_t rhs) const noexcept
		{
			return u64 >= rhs.u64;
		}

		constexpr bool operator<(const int_t rhs) const noexcept
		{
			return u64 < rhs.u64;
		}

		constexpr bool operator<=(const int_t rhs) const noexcept
		{
			return u64 <= rhs;
		}

	public:
		constexpr bool operator!() const noexcept
		{
			return !u64;
		}

		constexpr bool operator&&(const int_t rhs) const noexcept
		{
			return u64 && rhs.u64;
		}

		constexpr bool operator||(const int_t rhs) const noexcept
		{
			return u64 || rhs.u64;
		}

	public:
		constexpr int_t operator<<(const size_t nBits) const noexcept
		{
			return u64 << nBits;
		}

		constexpr int_t operator<<=(const size_t nBits) noexcept
		{
			return u64 <<= nBits;
		}

		constexpr int_t operator>>(const size_t nBits) const noexcept
		{
			return u64 >> nBits;
		}

		constexpr int_t operator>>=(const size_t nBits) noexcept
		{
			return u64 <<= nBits;
		}

	public:
		constexpr int_t operator+() const noexcept
		{
			return u64;
		}

		constexpr int_t operator-() const noexcept
		{
			return ~u64 + 1;
		}

	public:
		constexpr int_t operator&(const int_t rhs) const noexcept
		{
			return u64 & rhs.u64;
		}

		constexpr int_t operator&=(const int_t rhs) noexcept
		{
			return u64 &= rhs.u64;
		}

		constexpr int_t operator|(const int_t rhs) const noexcept
		{
			return u64 | rhs.u64;
		}

		constexpr int_t operator|=(const int_t rhs) noexcept
		{
			return u64 |= rhs.u64;
		}

		constexpr int_t operator^(const int_t rhs) noexcept
		{
			return u64 ^ rhs.u64;
		}

		constexpr int_t operator^=(const int_t rhs) noexcept
		{
			return u64 ^= rhs.u64;
		}

		constexpr int_t operator~() const noexcept
		{
			return ~u64;
		}

	public:
		constexpr operator uint64_t() const noexcept
		{
			return u64;
		}
	};
}
