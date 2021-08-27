#pragma once

#include "BigInt.h"


namespace eucl
{
	math::BigInt ggT(const math::BigInt &larger, const math::BigInt &smaller) noexcept
	{
		math::BigInt a = larger, b = smaller, r = 0, r_old{};

		do
		{
			r_old = r;
			r = a % b;
			a = b;
			b = r;
		} while (r != 0);

		return r_old;
	}

	struct maybe_negative
	{
		math::BigInt value;
		bool bNegative = false;

	public:
		maybe_negative() noexcept = default;

		maybe_negative(const math::BigInt &n, bool bNegative = false)
		{
			value = n;
			this->bNegative = bNegative;
		}

	public:
		bool isNegative() const noexcept
		{
			return bNegative;
		}

		math::BigInt get() const noexcept
		{
			return value;
		}

	public:
		maybe_negative operator*(const math::BigInt &rhs) const noexcept
		{
			return maybe_negative(get() * rhs, bNegative);
		}

		maybe_negative operator-(const maybe_negative &rhs) const noexcept
		{
			if (isNegative() && rhs.isNegative())
			{
				// (-x0) - (-x1) = (-x0) + x1 = x1 - x0
				return maybe_negative(rhs.get()) - maybe_negative(get());
			}
			else if (!isNegative() && rhs.isNegative())
			{
				// x0 - (-x1) = x0 + x1
				return maybe_negative(get() + rhs.get());
			}
			else if (isNegative() && !rhs.isNegative())
			{
				// (-x0) - x1 = (-x0) + (-x1) = -(x0 + x1)
				return maybe_negative(rhs.get() + get(), true);
			}

			// else (!isNegative() && !rhs.isNegative())
			// x0 - x1

			if (get() >= rhs.get()) // get() - rhs.get() cant be negative
				return maybe_negative(get() - rhs.get());

			// get() - rhs.get() is negative
			return maybe_negative(rhs.get() - get(), true);
		}

		friend std::ostream &operator<<(std::ostream &os, const maybe_negative &i) noexcept
		{
			if (i.isNegative()) os << "-";
			os << i.get();

			return os;
		}
	};

	struct Entry
	{
		math::BigInt a, b, q, r;
		maybe_negative x, y;
	};

	math::BigInt ext_euclidean(const math::BigInt &a, const math::BigInt &b)
	{
		std::vector<Entry> vEntries{};

		math::BigInt q, r;
		math::BigInt::divmod(a, b, q, r);
		vEntries.push_back({ a, b, q, r });

		while (r != 0)
		{
			Entry entry{ vEntries.at(vEntries.size() - 1).b, vEntries.at(vEntries.size() - 1).r };
			math::BigInt::divmod(entry.a, entry.b, q, r);
			entry.q = q;
			entry.r = r;
			vEntries.push_back(entry);
		}

		size_t i = vEntries.size() - 1;
		vEntries.at(i).x = math::BigInt(0);
		vEntries.at(i).y = math::BigInt(1);

		while (i-- != 0)
		{
			vEntries.at(i).x = vEntries.at(i + 1).y;
			maybe_negative negativeOffset = vEntries.at(i + 1).y * vEntries.at(i).q;
			maybe_negative newy = vEntries.at(i + 1).x - negativeOffset;
			vEntries.at(i).y = newy;
		}

		if (vEntries.at(0).x.isNegative())
			return b - vEntries.at(0).x.get();
		return vEntries.at(0).x.get();
	}
}
