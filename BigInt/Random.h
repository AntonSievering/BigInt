#pragma once

#include "BigInt.h"
#include <random>

class Random
{
private:
	uint32_t m_nSeed = 0;

public:
	Random() noexcept = default;

	Random(const uint32_t seed) noexcept
	{
		m_nSeed = seed;
	}

public:
	bool get1() noexcept
	{
		return get32();
	}

	uint16_t get16() noexcept
	{
		return get32();
	}

	uint32_t get32() noexcept
	{
		m_nSeed += 0xe120fc15;
		uint64_t temp;
		temp = (uint64_t)m_nSeed * 0x4a39b70d;
		uint32_t m1 = (temp >> 32) ^ temp;
		temp = (uint64_t)m1 * 0x12fad5c9;
		uint32_t m2 = (temp >> 32) ^ temp;
		return m2;
	}

	uint64_t get64() noexcept
	{
		math::int_t random{};
		random.u32[0] = get32();
		random.u32[1] = get32();
		return random.u64;
	}

	math::BigInt get(const size_t nBits) noexcept
	{
		math::BigInt integer = math::BigInt(0);
		
		const size_t nBlocksToFill = nBits / 64;
		const size_t nBitsToFill   = nBits % 64;

		for (size_t i = 0; i < nBlocksToFill; i++)
			integer.setBlock(i, get64());
		
		if (nBitsToFill > 0)
		{
			size_t nShifts = 64 - nBitsToFill;
			uint64_t v = ~(uint64_t)0;
			integer.setBlock(nBlocksToFill, ((v << nShifts) >> nShifts) & get64());
		}
		
		return integer;
	}

	static size_t getBitCount(const math::BigInt &number) noexcept
	{
		auto getBit64Count = [](uint64_t n) -> size_t
		{
			size_t bits = 0;
			while (n)
			{
				bits++;
				n >>= 1;
			}

			return bits;
		};

		size_t i = number.getBlockCount();
		while (i-- != 0)
			if (number.getBlock(i))
				return i * 64 + getBit64Count(number.getBlock(i).u64);

		return 0;
	}

	math::BigInt rangeto(const math::BigInt &upper) noexcept
	{
		math::BigInt random;
		std::random_device rd;
		std::default_random_engine generator(rd());

		bool bMaxAllowed = false;
		size_t i = upper.getBlockCount();
		while (i-- != 0)
		{
			uint64_t nLimit = bMaxAllowed ? 0xFFFFFFFFFFFFFFFF : upper.getBlock(i).u64;
			std::uniform_int_distribution<uint64_t> dist(0, nLimit);
			random.setBlock(i, dist(generator));
			if (!bMaxAllowed && random.getBlock(i) < upper.getBlock(i)) bMaxAllowed = true;
		}

		return random;
	}

	math::BigInt range(const math::BigInt &lower, const math::BigInt &upper) noexcept
	{
		return lower + rangeto(upper - lower);
	}
};
