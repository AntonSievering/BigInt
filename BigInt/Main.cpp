#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>
#include <fstream>

#include "Timer.h"
#include "BigInt.h"
#include "Random.h"
#include "Prime.h"
#include "euclidean.h"

int main_find_primes()
{
	std::ofstream file = std::ofstream("large_primes.txt");
	if (!file.is_open()) return EXIT_FAILURE;

	Random randomDevice = Random(0);

	math::BigInt n;
	while (true)
	{
		n = randomDevice.get(1024);
		if (primeTest_MillerRabin(n, randomDevice, 4))
		{
			std::cout << n << std::endl;
			file << n << '\n';
			file.flush();
		}
	}

	return EXIT_FAILURE;
}

math::BigInt readNumber(const size_t index)
{
	std::ifstream file = std::ifstream("large_primes.txt");
	if (!file.is_open()) throw std::runtime_error("primes file not found");

	size_t i = 0;
	std::string sLine;
	while (std::getline(file, sLine) && i++ != index);
	file.close();

	return math::BigInt(sLine);
}


int main_rsa()
{
	math::BigInt uninit, zero = 0;
	std::cout << (uninit == zero) << std::endl;

	Random random = Random(0);

	math::BigInt p = readNumber(10);
	math::BigInt q = readNumber(25);

	std::cout << p << std::endl << q << std::endl;

	math::BigInt N = p * q;
	std::cout << N << std::endl;

	math::BigInt euler = (p - (math::int_t)1) * (q - (math::int_t)1);
	std::cout << euler << std::endl;

	// gesucht ist [(d * e) % euler = 1]
	math::BigInt e = 65537;
	do e = random.get64();
	while (eucl::ggT(euler, e) != 1);
	std::cout << e << std::endl;

	math::BigInt d = eucl::ext_euclidean(e, euler);

	std::cout << "d: " << d << std::endl;

	auto test = [N, e](const math::BigInt &d) -> void
	{
		std::cout << "\nStarting test with " << d << std::endl;
		const math::BigInt &msg = 10;
		math::BigInt c = msg.powmod(e, N);
		math::BigInt m = c.powmod(d, N);
		if (m == msg)
			std::cout << "Test succeeded.\n";
		else
		{
			std::cout << m << std::endl;
			std::cout << "Test failed.\n";
		}
	};

	test(d);

	return 0;
}
