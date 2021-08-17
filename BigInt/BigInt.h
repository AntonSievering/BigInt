#pragma once

#include "int_type.h"
#include "exceptions.h"
#include "ExpandingVector.h"

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

		BigInt(const char *ptrString) BIGINT_NOEXCEPT
		{
			uint64_t nStringLength = std::strlen(ptrString);

			auto toLowerCase = [](char c) -> char
			{
				return c | 0b00100000;
			};

			// hex
			if (ptrString[0] == '0' && toLowerCase(ptrString[1]) == 'x')
			{
				ptrString += 2;
				nStringLength -= 2;
				m_data.resize((nStringLength + 15) / 16);

				uint64_t i = 0;
				for (char c = *ptrString; c != 0; c = *++ptrString, i++)
				{
					uint64_t nIterator = nStringLength - 1 - i;
					uint8_t  quarterWord = getQuarterWord(c);
					uint64_t nBlockOffset = (nIterator / 2) % sizeof(uint64_t);
					bool     bOffset = (~nIterator) & 1;
					uint64_t nBlock = nIterator / (2 * sizeof(uint64_t));
					setQuarterWord(quarterWord, nBlock, nBlockOffset, bOffset);
				}
			}
			// binary
			else if (ptrString[0] == '0' && toLowerCase(ptrString[1]) == 'b')
			{
				ptrString += 2;
				nStringLength -= 2;
			}
			// decimal
			else
			{
			}
		}

		template <class T>
		explicit BigInt(const T &data) noexcept
		{
			const char *ptrData = (const char *)&data;

			for (uint64_t i = 0; i < sizeof(T); i++)
				setByte(ptrData[i], i);
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
			const uint64_t nBlockIndex = byteIndex / sizeof(uint64_t);
			const uint64_t nBlockOffset = byteIndex % sizeof(uint64_t);
			
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
			int_t b = getBlock(nBlockIndex);
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

			return result;
		};

		void twosComplement() noexcept
		{
			for (uint64_t i = 0; i < m_data.size(); i++)
			{
				int_t data = getBlock(i);
				data.u64 = ~data.u64;
				setBlock(i, data);
			}

			// add 1 but don't carry correct

			uint32_t carry = 0;
			int_t block = getBlock(0);
			block.u32[0] = saveAdd(getBlock(0).u32[0], 1, carry);
			setBlock(0, block);

			for (uint64_t nBlock = 1; nBlock < m_data.size(); nBlock++)
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
			int_t carry = 0;
			for (uint64_t i = 0; i < m_data.size(); i++)
			{
				int_t thisBlock = getBlock(i);
				setBlock(i, thisBlock << 1Ui64 | carry);
				carry = thisBlock >> 63Ui64;
			}

			carryCorrect(carry.u32[0]);
			setBlock(0, getBlock(0) | int_t(bit));
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

		size_t usedSize() const noexcept
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
		[[nodiscard]] int_t getBlock(const uint64_t index) const noexcept
		{
			return m_data.getBlock(index);
		}

		[[nodiscard]] int_t getBlockCheck(const uint64_t index) const noexcept
		{
			if (index < m_data.size())
				return m_data.getBlock(index);
			return int_t(0);
		}

		void setBlock(const size_t index, const int_t block) noexcept
		{
			m_data.setBlock(index, block);
		}

	public:
		friend std::ostream &operator<<(std::ostream &os, const BigInt &i) noexcept
		{
			os << "0x";
			bool bWriteNeeded = false;

			for (int64_t idx = i.m_data.size() - 1; idx >= 0; idx--)
			{
				int_t val = i.getBlock(idx);

				if (bWriteNeeded)
					os << '\'';

				if (bWriteNeeded || val.u64 != 0 || idx == 0)
				{
					os << std::setw(8) << std::hex << std::setfill('0') << val.u32[1] << '\'' << std::setw(8) << std::setfill('0') << val.u32[0];
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
			out.m_data.resize(nMaxSize);
			
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

		[[nodiscard]] BigInt operator+(const int_t rhs) const noexcept
		{
			BigInt out;
			out.m_data.resize(usedSize());
			
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
			v.m_data.resize(nMaxSize);
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
			return *this = *this - (int_t)1;
		}

		[[nodiscard]] BigInt operator-(const int_t rhs) const noexcept
		{
			if (rhs.u64 <= 0x8fffffffffffffff)
				return *this + int_t{ ~rhs.u64 + (uint64_t)1 };
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
			out.m_data.resize(usedSize());

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

		BigInt operator<<=(const size_t nBits) noexcept
		{
			*this = *this << nBits;
			return *this;
		}

		BigInt operator>>(const size_t nBits) const noexcept
		{
			BigInt out;
			out.m_data.resize(usedSize());

			const size_t nBlockOffset = nBits / 64;
			const size_t nBitOffset = nBits % 64;

			int_t carry = 0;
			for (int64_t i = std::max(m_data.size() - 1, nBlockOffset); i >= nBlockOffset; i--)
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
				if (getBlock(i).u64 < rhs.getBlock(i)) return true;
			
			return false;
		}

		bool operator<=(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize < nRhsUsedSize) return true;
			if (nOwnUsedSize > nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
				if (getBlock(i).u64 < rhs.getBlock(i)) return true;

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
				if (getBlock(i).u64 > rhs.getBlock(i)) return true;

			return false;
		}

		bool operator>=(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();

			if (nOwnUsedSize > nRhsUsedSize) return true;
			if (nOwnUsedSize < nRhsUsedSize) return false;

			uint64_t i = nOwnUsedSize;
			while (i-- != 0)
				if (getBlock(i).u64 > rhs.getBlock(i)) return true;

			return getBlock(0) == rhs.getBlock(0);
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
						quotinent.shift_left_set_last_bit(1);
						remainder -= divisor;
					}
					else
					{
						quotinent.shift_left_set_last_bit(0);
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
			BigInt quotient, remainder;
			divmod(*this, rhs, quotient, remainder);
			return remainder;
		}

		BigInt operator%=(const BigInt &rhs) noexcept
		{
			return *this = *this % rhs;
		}

	public:
		BigInt operator&(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();
			size_t nMinUsedSize = std::min(nOwnUsedSize, nRhsUsedSize);

			BigInt out;
			out.m_data.resize(nMinUsedSize);

			for (size_t i = 0; i < nMinUsedSize; i++)
				out.setBlock(i, getBlock(i) & rhs.getBlock(i));
			
			return out;
		}

		BigInt operator&=(const BigInt &rhs) noexcept
		{
			return *this = *this & rhs;
		}
		
		BigInt operator|(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();
			size_t nMinUsedSize = std::min(nOwnUsedSize, nRhsUsedSize);

			BigInt out;
			out.m_data.resize(nMinUsedSize);

			for (size_t i = 0; i < nMinUsedSize; i++)
				out.setBlock(i, getBlock(i) | rhs.getBlock(i));

			return out;
		}

		BigInt operator|=(const BigInt &rhs) noexcept
		{
			return *this = *this | rhs;
		}

		BigInt operator^(const BigInt &rhs) const noexcept
		{
			size_t nOwnUsedSize = usedSize();
			size_t nRhsUsedSize = rhs.usedSize();
			size_t nMinUsedSize = std::min(nOwnUsedSize, nRhsUsedSize);

			BigInt out;
			out.m_data.resize(nMinUsedSize);

			for (size_t i = 0; i < nMinUsedSize; i++)
				out.setBlock(i, getBlock(i) ^ rhs.getBlock(i));

			return out;
		}

		BigInt operator^=(const BigInt &rhs) noexcept
		{
			return *this = *this ^ rhs;
		}

	public:
		BigInt pow(const BigInt &rhs) const noexcept
		{
			BigInt out;

			// todo

			return out;
		}
	};
}
