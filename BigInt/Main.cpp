#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>
#include <fstream>

#include "Timer.h"
#include "BigInt.h"
#include "Random.h"
#include "Prime.h"

int main()
{
	/*math::BigInt e = "0x37856876767ad876f786e86878c8d9";
	math::BigInt m = "0xdaef768d768a";
	math::BigInt base = "0xfe678d";

	math::BigInt r = base.powmod(e, m); // r = base ** e % m
	std::cout << r << std::endl;*/

	std::ofstream file = std::ofstream("large_primes.txt");
	if (!file.is_open()) return EXIT_FAILURE;

	Random randomDevice = Random(0);

	math::BigInt n;
	while (true)
	{
		n = randomDevice.get(1024);
		if (primeTest(n, randomDevice, 4))
		{
			std::cout << n << std::endl;
			file << n << '\n';
			file.flush();
		}
	}

	return EXIT_FAILURE;
}
