#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>

#include "Timer.h"
#include "BigInt.h"

int main()
{
	math::BigInt i = "0xad5f0d0cad54de9c35648294aedff0d0cad5f0d0cad5f0d0c";
	math::BigInt j = "0x25fda45da2ef34e";

	Engine::Timer timer = Engine::Timer().start();
	math::BigInt remainder = i % j;
	float fTime = timer.getElapsedTime();
	std::cout << "divmod took " << fTime << "s\n";

	std::cout << i << " % " << j << " = " << remainder << std::endl;

	return 0;
}
