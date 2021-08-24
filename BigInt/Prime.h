#pragma once

#include "Random.h"
#include <array>

std::array<uint32_t, 70> g_vSomePrimes =
{   2,   3,   5,   7,  11,  13,  17,  19,  23,  29,
   31,  37,  41,  43,  47,  53,  59,  61,  67,  71,
   73,  79,  83,  89,  97, 101, 103, 107, 109, 113,
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
  179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
  233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
  283, 293, 307, 311, 313, 317, 331, 337, 347, 349 };

static bool isLowLevelPrime(const math::BigInt &value) noexcept
{
	if (value < 2) return false;

	if (value > 349 * 349)
	{
		for (const uint32_t prime : g_vSomePrimes)
			if (value % prime == 0) return false;

		return true;
	}

	for (const uint32_t prime : g_vSomePrimes)
	{
		if (value < static_cast<uint64_t>(prime) * prime) return true;
		if (value % prime == 0) return false;
	}

	return true;
}

static bool millerTest(math::BigInt d, const math::BigInt &n, Random &randomDevice) noexcept
{
	math::BigInt a = randomDevice.range(2, n - (math::int_t)2);
	math::BigInt x = a.powmod(d, n);
	math::BigInt n_minus_1 = n - (math::int_t)1;

	if (x == 1 || x == n_minus_1) return true;

	while (d != n_minus_1)
	{
		math::BigInt k = x * x;
		x = k % n;
		d <<= 1;

		if (x == 1)
			return false;
		if (x == n_minus_1)
			return true;
	}

	return false;
}

static bool primeTest(const math::BigInt &number, Random &randomDevice, const size_t nIterations = 20) noexcept
{
	if (!isLowLevelPrime(number)) return false;

	math::BigInt d = number - (math::int_t)1;
	while ((d.getBlock(0).u64 & 1) == 0) d >>= 1;

	for (size_t i = 0; i < nIterations; i++)
		if (!millerTest(d, number, randomDevice))
			return false;

	return true;
}
