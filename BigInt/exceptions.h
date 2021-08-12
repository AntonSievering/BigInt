#pragma once

namespace math
{
	namespace error
	{
		struct base_error{};

		struct overflow : base_error {};

		struct underflow : base_error{};
		
		struct bad_length : base_error{};
		
		struct unrecognized_char : base_error{};
		
		struct out_of_bounds : base_error{};
	}
}
