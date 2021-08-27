#pragma once

#include "int_type.h"
#include "exceptions.h"
#include "ExpandingVector.h"
#include <bitset>

#ifdef _DEBUG
#define _BIGINT_EXCEPTIONS_
#define _BIGINT_MEMCHECK_
#endif

#ifdef _BIGINT_EXCEPTIONS_
#define BIGINT_NOEXCEPT noexcept(false)

#else
#define BIGINT_NOEXCEPT noexcept
#endif

namespace math
{
	class BigInt
	{
	private:
		ExpandingVector m_data;

	public:
		BigInt() noexcept = default;

		BigInt(std::string sNumber) BIGINT_NOEXCEPT
		{
			auto toLowerCase = [](char c) -> char
			{
				return c | 0b00100000;
			};

			// hex
			if (sNumber[0] == '0' && toLowerCase(sNumber[1]) == 'x')
			{
				sNumber = sNumber.substr(2);
				uint64_t i = sNumber.size();
				uint64_t count = 0;

				while (i-- != 0)
				{
					uint8_t  quarterWord = getQuarterWord(sNumber.at(i));
					uint64_t nBlockOffset = (count % 16) / 2;
					uint64_t nBlockIndex = count / 16;
					bool bOffset = !(count & 1);

					setQuarterWord(quarterWord, nBlockIndex, nBlockOffset, bOffset);

					count++;
				}
			}
			// binary
			else if (sNumber[0] == '0' && toLowerCase(sNumber[1]) == 'b')
			{
			}
			// decimal
			else
			{
			}
		}

		BigInt(int_t rhs) noexcept
		{
			setBlock(0, rhs);
		}

		BigInt(uint64_t rhs) noexcept
		{
			setBlock(0, int_t(rhs));
		}

	private:
		void setByte(uint8_t byte, uint64_t byteIndex) noexcept
		{
			const size_t nBlockIndex = byteIndex / sizeof(uint64_t);
			const size_t nBlockOffset = byteIndex % sizeof(uint64_t);

			int_t block{};

			if (nBlockIndex < m_data.size())
				block = getBlock(nBlockIndex);
			block.u8[nBlockOffset] = byte;

			setBlock(nBlockIndex, block);
		}

		static constexpr uint8_t getQuarterWord(char c) BIGINT_NOEXCEPT
		{
			if (c >= '0' && c <= '9')
				return c - '0' + 0x0;
			else if (c >= 'a' && c <= 'f')
				return c - 'a' + 0xa;
#ifdef _BIGINT_EXCEPTIONS_
			throw error::unrecognized_char{};
#else
			return 0;
#endif
		}

		void setQuarterWord(const uint8_t &nQuarterWord, const uint64_t &nBlockIndex, const uint64_t &nBlockOffset, const bool &bOffset) noexcept
		{
			int_t b = getBlockCheck(nBlockIndex);
			int_t output = b;

			if (bOffset)
			{
				uint8_t mask = b.u8[nBlockOffset] & 0xF0;
				output.u8[nBlockOffset] = mask | (nQuarterWord & 0x0F);
			}
			else
			{
				uint8_t mask = b.u8[nBlockOffset] & 0x0F;
				output.u8[nBlockOffset] = mask | (nQuarterWord << 4);
			}

			setBlock(nBlockIndex, output);
		}

		static uint32_t saveAdd(const uint32_t a, const uint32_t b, uint32_t &carry) noexcept
		{
			uint64_t result = (uint64_t)a + (uint64_t)b + (uint64_t)carry;
			carry = result >> 32;

			return static_cast<uint32_t>(result);
		};

		void twosComplement() noexcept
		{
			for (uint64_t i = 0; i < m_data.size(); i++)
			{
				int_t data = getBlock(i);
				data.u64 = ~data.u64;
				setBlock(i, data);
			}

			uint32_t carry = 1;

			for (uint64_t nBlock = 0; nBlock < m_data.size(); nBlock++)
			{
				for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
				{
					int_t block = getBlock(nBlock);
					block.u32[nOffset] = saveAdd(getBlock(nBlock).u32[nOffset], 0, carry);
					setBlock(nBlock, block);
				}
			}
		}

		void addOffset(const int_t value, const uint64_t offset) noexcept
		{
			auto internalAddOffset = [&](const uint32_t c, const uint64_t offset) -> void
			{
				uint32_t carry = c;
				uint64_t i = offset;
				for (uint64_t i = offset; carry != 0; i++)
				{
					int_t block = getBlock(i / 2);
					block.u32[i % 2] = saveAdd(block.u32[i % 2], 0, carry);
					setBlock(i / 2, block);
				}
			};

			internalAddOffset(value.u32[0], offset);
			internalAddOffset(value.u32[1], offset + 1);
		}

		void shift_left_set_last_bit(bool bit) noexcept
		{
			int_t carry = bit;
			uint64_t i = 0;
			do
			{
				int_t thisBlock = getBlock(i);
				setBlock(i, thisBlock << 1Ui64 | carry);
				carry = thisBlock >> 63Ui64;
				i++;
			} while (i < m_data.size());

			carryCorrect(carry.u32[0]);
		}

		void setBit(const size_t nBlockIndex, const size_t nBitIndex) noexcept
		{
			int_t block = getBlockCheck(nBlockIndex);
			block.u64 |= (uint64_t(1) << nBitIndex);
			setBlock(nBlockIndex, block);
		}

		[[nodiscard]] size_t getMinUsedSize(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();
			return std::min(nOwnUsedSize, nRhsUsedSize);
		}

		void carryCorrect(const uint32_t carry) noexcept
		{
			if (carry)
			{
				int_t block{};
				block.u32[0] = carry;
				m_data.setBlock(m_data.size(), block);
			}
		}

		[[nodiscard]] size_t usedSize() const noexcept
		{
			size_t nUsedSize = m_data.size();

			size_t i = nUsedSize;
			while (i-- != 0)
			{
				if (m_data.getBlock(i).u64 != 0)
					return nUsedSize;
				nUsedSize--;
			}

			return 0;
		}

	public:
		[[nodiscard]] int_t getBlock(const uint64_t index) const BIGINT_NOEXCEPT
		{
#ifdef _BIGINT_EXCEPTIONS_
			if (index < m_data.size())
				return m_data.getBlock(index);
			throw error::out_of_bounds{};
#else
			return m_data.getBlock(index);
#endif
		}

		[[nodiscard]] int_t getBlockCheck(const uint64_t index) const noexcept
		{
			if (index < m_data.size())
				return m_data.getBlock(index);
			return int_t(0);
		}

		inline void setBlock(const size_t index, const int_t block) noexcept
		{
			m_data.setBlock(index, block);
		}

		size_t getBlockCount() const noexcept
		{
			return m_data.size();
		}

	public:
		friend std::ostream &operator<<(std::ostream &os, const BigInt &i) noexcept
		{
			os << "0x";
			bool bWriteNeeded = false;

			for (int64_t idx = i.m_data.size() - 1; idx >= 0; idx--)
			{
				int_t val = i.getBlock(idx);

				if (bWriteNeeded || val.u64 != 0 || idx == 0)
				{
					os << std::setw(16) << std::hex << std::setfill('0') << val.u64;
					bWriteNeeded = true;
				}
			}

			return os;
		}

	public: // +; +=; ++
		[[nodiscard]] BigInt operator+(const BigInt &rhs) const noexcept
		{
			size_t nMaxSize = std::max(usedSize(), rhs.usedSize());

			BigInt out;
			out.m_data.resize(std::max(nMaxSize, size_t(1)));
			
			uint32_t carry = 0;
			for (uint64_t nBlock = 0; nBlock < nMaxSize; nBlock++)
			{
				for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
				{
					int_t block = out.getBlock(nBlock);
					block.u32[nOffset] = saveAdd(getBlockCheck(nBlock).u32[nOffset], rhs.getBlockCheck(nBlock).u32[nOffset], carry);
					out.setBlock(nBlock, block);
				}
			}

			out.carryCorrect(carry);
			
			return out;
		}

		BigInt &operator+=(const BigInt &rhs) noexcept
		{
			return *this = *this + rhs;
		}

		BigInt &operator++() noexcept
		{
			return *this += (int_t)1;
		}

		const BigInt operator++(int) noexcept
		{
			return *this += (int_t)1;
		}

		[[nodiscard]] BigInt operator+(const int_t rhs) const noexcept
		{
			BigInt out;
			out.m_data.resize(std::max(usedSize(), (size_t)1));
			
			uint32_t carry = 0;

			// add first 8 bytes
			for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
			{
				int_t block = out.getBlock(0);
				block.u32[nOffset] = saveAdd(getBlock(0).u32[nOffset], rhs.u32[nOffset], carry);
				out.setBlock(0, block);
			}
			
			// add the rest
			for (uint64_t nBlock = 1; nBlock < out.m_data.size(); nBlock++)
			{
				for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
				{
					int_t block = getBlock(nBlock);
					block.u32[nOffset] = saveAdd(getBlock(nBlock).u32[nOffset], 0, carry);
					out.setBlock(nBlock, block);
				}
			}

			out.carryCorrect(carry);

			return out;
		}

		BigInt &operator+=(const int_t rhs) noexcept
		{
			return *this = *this + rhs;
		}

	public: // +; -
		[[nodiscard]] BigInt operator+() const noexcept
		{
			return *this;
		}

		[[nodiscard]] BigInt operator-() const noexcept
		{
			BigInt v = *this;
			v.twosComplement();
			return v;
		}

	public: // -; -=; --
		[[nodiscard]] BigInt operator-(const BigInt &rhs) const noexcept
		{
			size_t nMaxSize = std::max(usedSize(), rhs.usedSize());

			BigInt v = rhs;
			v.m_data.resize(std::max(nMaxSize, (size_t)1));
			v.twosComplement();

			BigInt out;
			out.m_data.resize(nMaxSize);

			uint32_t carry = 0;
			for (uint64_t nBlock = 0; nBlock < nMaxSize; nBlock++)
			{
				for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
				{
					int_t block = out.getBlock(nBlock);
					block.u32[nOffset] = saveAdd(getBlock(nBlock).u32[nOffset], v.getBlock(nBlock).u32[nOffset], carry);
					out.setBlock(nBlock, block);
				}
			}

			return out;
		}

		BigInt &operator-=(const BigInt &rhs) noexcept
		{
			return *this = *this - rhs;
		}

		BigInt &operator--() noexcept
		{
			return *this -= (int_t)1;
		}

		const BigInt operator--(int) noexcept
		{
			return *this -= (int_t)1;
		}

		[[nodiscard]] BigInt operator-(const int_t rhs) const noexcept
		{
			return *this - BigInt(rhs);
		}

		BigInt &operator-=(const int_t rhs) noexcept
		{
			return *this = *this - rhs;
		}

	public: // *, *=
		[[nodiscard]] BigInt operator*(const BigInt &rhs) const noexcept
		{
			auto saveMul = [](const uint64_t a, const uint64_t b) -> uint64_t
			{
				return a * b;
			};

			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			BigInt out;
			out.m_data.resize(nOwnUsedSize * nRhsUsedSize + 1);

			for (uint64_t i = 0; i < nOwnUsedSize * 2; i++)
			{
				for (uint64_t j = 0; j < nRhsUsedSize * 2; j++)
				{
					uint64_t value = saveMul(getBlock(i / 2).u32[i % 2], rhs.getBlock(j / 2).u32[j % 2]);
					out.addOffset(value, i + j);
				}
			}

			return out;
		}

		BigInt operator*=(const BigInt &rhs) noexcept
		{
			return *this = *this * rhs;
		}

	public:
		BigInt operator~() const noexcept
		{
			BigInt out;
			out.m_data.resize(m_data.size());

			for (uint64_t i = 0; i < m_data.size(); i++)
				out.setBlock(i, ~getBlock(i));

			return out;
		}

		BigInt operator<<(const size_t nBits) const noexcept
		{
			BigInt out;
			out.m_data.resize(std::max(usedSize(), (size_t)1));

			const size_t nBlockOffset = nBits / 64;
			const size_t nBitOffset = nBits % 64;

			int_t carry = 0;
			for (uint64_t i = 0; i < m_data.size(); i++)
			{
				int_t thisBlock = getBlock(i);
				out.setBlock(i + nBlockOffset, thisBlock << nBitOffset | carry);
				carry = nBitOffset > 0 ? thisBlock >> (64 - nBitOffset) : int_t(0);
			}

			out.carryCorrect(carry.u32[0]);

			return out;
		}

		BigInt operator<<(const BigInt nBits) const noexcept
		{
			const size_t nBlockOffset = (nBits / 64).getBlock(0).u64;
			const size_t nBitOffset   = nBits.getBlock(0).u64 % 64;

			BigInt out;
			out.m_data.resize(std::max(usedSize(), (size_t)1));

			int_t carry = 0;
			for (uint64_t i = 0; i < m_data.size(); i++)
			{
				int_t thisBlock = getBlock(i);
				out.setBlock(i + nBlockOffset, thisBlock << nBitOffset | carry);
				carry = nBitOffset > 0 ? thisBlock >> (64 - nBitOffset) : int_t(0);
			}

			out.carryCorrect(carry.u32[0]);

			return out;
		}

		BigInt operator<<=(const size_t nBits) noexcept
		{
			*this = *this << nBits;
			return *this;
		}

		BigInt operator>>(const size_t nBits) const noexcept
		{
			BigInt out;
			out.m_data.resize(std::max(usedSize(), (size_t)1));

			const int64_t nBlockOffset = nBits / 64;
			const uint64_t nBitOffset = nBits % 64;

			int_t carry = 0;
			for (int64_t i = std::max((int64_t)m_data.size() - 1, nBlockOffset); i >= nBlockOffset; i--)
			{
				int_t thisBlock = getBlock(i);
				out.setBlock(i - nBlockOffset, thisBlock >> nBitOffset | carry);
				carry = nBitOffset > 0 ? thisBlock << (64 - nBitOffset) : int_t(0);
			}

			return out;
		}

		BigInt operator>>=(const size_t nBits) noexcept
		{
			*this = *this >> nBits;
			return *this;
		}

	public:
		bool operator<(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize < nRhsUsedSize) return true;
			if (nOwnUsedSize > nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
				if (getBlock(i) >= rhs.getBlock(i)) return false;
			
			return true;
		}

		bool operator<=(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize < nRhsUsedSize) return true;
			if (nOwnUsedSize > nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
			{
				if (getBlock(i) > rhs.getBlock(i)) return false;
				if (getBlock(i) < rhs.getBlock(i)) return true;
			}

			return getBlock(0) == rhs.getBlock(0);
		}

		bool operator>(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize > nRhsUsedSize) return true;
			if (nOwnUsedSize < nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
				if (getBlock(i) <= rhs.getBlock(i)) return false;

			return true;
		}

		bool operator>=(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize > nRhsUsedSize) return true;
			if (nOwnUsedSize < nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
			{
				if (getBlock(i) < rhs.getBlock(i)) return false;
				if (getBlock(i) > rhs.getBlock(i)) return true;
			}

			return getBlock(0) == rhs.getBlock(0);
		}
		
		bool operator==(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize != nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
				if (getBlock(i) != rhs.getBlock(i)) return false;

			return true;
		}

		bool operator==(const size_t rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			if (nOwnUsedSize > 1) return false;

			return getBlock(0).u64 == rhs;
		}

	public:
		static void divmod(const BigInt &dividend, const BigInt &divisor, BigInt &quotinent, BigInt &remainder) noexcept
		{
			quotinent = BigInt(0);
			remainder = BigInt(0);

			size_t i = dividend.m_data.size();
			while (i-- != 0)
			{
				size_t bit = 64;
				while (bit-- != 0)
				{
					remainder.shift_left_set_last_bit(dividend.getBlock(i).u64 >> bit & 1);

					if (divisor <= remainder)
					{
						quotinent.setBit(i, bit);
						remainder -= divisor;
					}
				}
			}
		}

		BigInt operator/(const BigInt &rhs) const noexcept
		{
			BigInt quotient, remainder;
			divmod(*this, rhs, quotient, remainder);
			return quotient;
		}

		BigInt operator/=(const BigInt &rhs) noexcept
		{
			return *this = *this / rhs;
		}

		BigInt operator%(const BigInt &rhs) const noexcept
		{
			BigInt remainder = BigInt(0);

			size_t i = m_data.size();
			while (i-- != 0)
			{
				size_t bit = 64;
				while (bit-- != 0)
				{
					remainder.shift_left_set_last_bit(getBlock(i).u64 >> bit & 1);

					if (remainder >= rhs)
						remainder -= rhs;
				}
			}

			return remainder;
		}

		BigInt operator%=(const BigInt &rhs) noexcept
		{
			return *this = *this % rhs;
		}

	public:
		BigInt operator&(const BigInt &rhs) const noexcept
		{
			size_t nMinUsedSize = getMinUsedSize(rhs);

			BigInt out;
			out.m_data.resize(std::max(nMinUsedSize, (size_t)1));

			for (size_t i = 0; i < nMinUsedSize; i++)
				out.setBlock(i, getBlock(i) & rhs.getBlock(i));
			
			return out;
		}

		BigInt operator&=(const BigInt &rhs) noexcept
		{
			size_t nMinUsedSize = getMinUsedSize(rhs);

			for (size_t i = 0; i < nMinUsedSize; i++)
				setBlock(i, getBlock(i) & rhs.getBlock(i));

			return *this;
		}
		
		BigInt operator|(const BigInt &rhs) const noexcept
		{
			size_t nMinUsedSize = getMinUsedSize(rhs);

			BigInt out;
			out.m_data.resize(std::max(nMinUsedSize, (size_t)1));

			for (size_t i = 0; i < nMinUsedSize; i++)
				out.setBlock(i, getBlock(i) | rhs.getBlock(i));

			return out;
		}

		BigInt operator|=(const BigInt &rhs) noexcept
		{
			size_t nMinUsedSize = getMinUsedSize(rhs);

			for (size_t i = 0; i < nMinUsedSize; i++)
				setBlock(i, getBlock(i) | rhs.getBlock(i));

			return *this;
		}

		BigInt operator^(const BigInt &rhs) const noexcept
		{
			size_t nMinUsedSize = getMinUsedSize(rhs);

			BigInt out;
			out.m_data.resize(std::max(nMinUsedSize, (size_t)1));

			for (size_t i = 0; i < nMinUsedSize; i++)
				out.setBlock(i, getBlock(i) ^ rhs.getBlock(i));

			return out;
		}

		BigInt operator^=(const BigInt &rhs) noexcept
		{
			size_t nMinUsedSize = getMinUsedSize(rhs);

			for (size_t i = 0; i < nMinUsedSize; i++)
				setBlock(i, getBlock(i) ^ rhs.getBlock(i));

			return *this;
		}

	public:
		BigInt powmod(const BigInt &exponent, const BigInt &modulus) const noexcept
		{
			BigInt out = BigInt(1);
			out.m_data.resize(2 * usedSize() + 1);

			bool bBitSet = false;
			size_t i = exponent.usedSize();
			while (i-- != 0)
			{
				size_t bit = 1Ui64 << 63;
				do
				{
					if (bBitSet)
					{
						BigInt n = out;
						out *= n;
						out %= modulus;
					}
					if (exponent.getBlock(i).u64 & bit)
					{
						out *= *this;
						out %= modulus;
						bBitSet = true;
					}
					bit >>= 1;
				} while (bit != 0);
			}

			return out;
		}

		/*BigInt pow(const BigInt &exponent) const noexcept
		{
			BigInt out = BigInt(1);
			out.m_data.resize(2 * usedSize() + 1);

			bool bBitSet = false;
			size_t i = exponent.usedSize();
			while (i-- != 0)
			{
				size_t bit = 1Ui64 << 63;
				do
				{
					if (bBitSet)
					{
						BigInt n = out;
						out *= n;
					}
					if (exponent.getBlock(i).u64 & bit)
					{
						out *= *this;
						bBitSet = true;
					}
					bit >>= 1;
				} while (bit != 0);
			}

			return out;
		}*/
	};

	bool operator>=(const int_t lhs, const BigInt &rhs) noexcept
	{
		return BigInt(lhs) >= rhs;
	}
}
