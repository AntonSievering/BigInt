#pragma once

namespace math
{
	namespace error
	{
		struct base_error{};

		struct unrecognized_char : base_error{};

		struct out_of_bounds : base_error{};
	}
}
