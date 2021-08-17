#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>

#include "Timer.h"
#include "BigInt.h"

int main()
{
	math::BigInt i = "0xad5f0d0cad54de9c35648294aedff0d0cad5f0d0cad5f0d0c";
	math::BigInt j = "0xffffffffffffffff";

	std::cout << i - ~math::int_t(0) << std::endl;

	return 0;
}
