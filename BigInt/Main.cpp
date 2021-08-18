#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>

#include "Timer.h"
#include "BigInt.h"

int main()
{
	math::BigInt e = "0x37856876767ad876f786e86878c8d9";
	math::BigInt m = "0xdaef768d768a";
	math::BigInt base = "0xfe678d";

	math::BigInt r = base.powmod(e, m); // r = base ** e % m
	std::cout << r << std::endl;

	return 0;
}
