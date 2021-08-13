#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>

#include "Timer.h"
#include "int_type.h"
#include "exceptions.h"

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
		static constexpr uint64_t s_nSize = (uint64_t)10; // move to template

	private:
		int_t m_pBuffer[s_nSize]{};

	public:
		BigInt() noexcept = default;

		BigInt(const char *ptrString) BIGINT_NOEXCEPT
		{
			uint64_t nStringLength = std::strlen(ptrString);

			auto toLowerCase = [](char c) -> char
			{
				return c | 0b00100000;
			};

			auto throwIfNessesary = [](uint64_t len) -> void
			{
#ifdef _BIGINT_EXCEPTIONS_
				if (len > s_nSize * 16 || len == 0)
					throw error::bad_length();
#endif
			};

			// hex
			if (ptrString[0] == '0' && toLowerCase(ptrString[1]) == 'x')
			{
				ptrString += 2;
				nStringLength -= 2;
				throwIfNessesary(nStringLength);

				uint64_t i = 0;
				for (char c = *ptrString; c != 0; c = *++ptrString, i++)
				{
					uint64_t nIterator    = nStringLength - 1 - i;
					uint8_t  quarterWord  = getQuarterWord(c);
					uint64_t nBlockOffset = (nIterator / 2) % sizeof(uint64_t);
					bool     bOffset      = (~nIterator) & 1;
					uint64_t nBlock       = nIterator / (2 * sizeof(uint64_t));
					setQuarterWord(quarterWord, nBlock, nBlockOffset, bOffset);
				}
			}
			// binary
			else if (ptrString[0] == '0' && toLowerCase(ptrString[1]) == 'b')
			{
				ptrString += 2;
				nStringLength -= 2;
				throwIfNessesary(nStringLength);
			}
			// decimal
			else
			{
				throwIfNessesary(nStringLength);
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
			m_pBuffer[0] = rhs;
		}

		BigInt(uint64_t rhs) noexcept
		{
			m_pBuffer[0].u64 = rhs;
		}

	private:
		void setByte(uint8_t byte, uint64_t byteIndex) noexcept
		{
			const uint64_t nBlockIndex  = byteIndex / sizeof(uint64_t);
			const uint64_t nBlockOffset = byteIndex % sizeof(uint64_t);
			m_pBuffer[nBlockIndex].u8[nBlockOffset] = byte;
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

		static uint32_t saveAdd(uint32_t a, uint32_t b, uint32_t &carry) noexcept
		{
			uint64_t result = (uint64_t)a + (uint64_t)b + (uint64_t)carry;
			carry = result >> 32;

			return result & 0xFFFFFFFF;
		};

		static uint64_t saveMul(uint64_t a, uint64_t b) noexcept
		{
			return a * b;
		}

		void twosComplement() noexcept
		{
			for (uint64_t i = 0; i < s_nSize; i++)
			{
				int_t data = getBlock(i);
				data.u64 = ~data.u64;
				setBlock(i, data);
			}
			this->operator++();
		}

		void addOffset(const int_t value, const uint64_t offset) noexcept
		{
			auto internalAddOffset = [&](const uint32_t c, const uint64_t offset) -> void
			{
				uint32_t carry = c;
				for (uint64_t i = offset; i < s_nSize * 2 && carry != 0; i++)
				{
					int_t block = getBlock(i / 2);
					block.u32[i % 2] = saveAdd(block.u32[i % 2], 0, carry);
					setBlock(i / 2, block);
				}
			};

			internalAddOffset(value.u32[0], offset);
			internalAddOffset(value.u32[1], offset + 1);
		}

	public:
		[[nodiscard]] int_t getBlock(const uint64_t index) const noexcept
		{
			return m_pBuffer[index];
		}

		void setBlock(const uint64_t index, const int_t block) noexcept
		{
			m_pBuffer[index] = block;
		}

	public:
		friend std::ostream &operator<<(std::ostream &os, const BigInt &i) noexcept
		{
			os << "0x";
			bool bWriteNeeded = false;
			
			for (int64_t idx = s_nSize - 1; idx >= 0; idx--)
			{
				uint64_t val = i.getBlock(idx).u64;

				if (bWriteNeeded || val != 0 || idx == 0)
				{
					os << std::setw(16) << std::hex << std::setfill('0') << val;
					bWriteNeeded = true;
				}
			}
			
			return os;
		}

	public: // +; +=; ++
		[[nodiscard]] BigInt operator+(const BigInt &rhs) const noexcept
		{
			BigInt out;
			uint32_t carry = 0;
			
			for (uint64_t nBlock = 0; nBlock < s_nSize; nBlock++)
			{
				for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
				{
					int_t block = out.getBlock(nBlock);
					block.u32[nOffset] = saveAdd(getBlock(nBlock).u32[nOffset], rhs.getBlock(nBlock).u32[nOffset], carry);
					out.setBlock(nBlock, block);
				}
			}

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
			BigInt out = *this;
			uint32_t carry = 0;

			// add
			for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
			{
				int_t block = out.getBlock(0);
				block.u32[nOffset] = saveAdd(getBlock(0).u32[nOffset], rhs.u32[nOffset], carry);
				out.setBlock(0, block);
			}

			// carry while nessesary
			for (uint64_t nBlock = 1; nBlock < s_nSize; nBlock++)
				for (uint64_t nOffset = 0; nOffset < 2; nOffset++)
				{
					if (carry == 0)
						return out;

					int_t block{};
					block.u32[nOffset] = saveAdd(getBlock(nBlock).u32[nOffset], 0, carry);
					out.setBlock(nBlock, block);
				}

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
			return *this + (-rhs);
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
			return *this + int_t{ ~rhs.u64 + (uint64_t)1 };
		}

		BigInt &operator-=(const int_t rhs) noexcept
		{
			return *this = *this - rhs;
		}

	public: // *, *=
		[[nodiscard]] BigInt operator*(const BigInt &rhs) const noexcept
		{
			BigInt out;

			for (uint64_t i = 0; i < s_nSize * 2; i++)
			{
				for (uint64_t j = 0; j < s_nSize * 2; j++)
				{
					uint64_t value = saveMul(getBlock(i / 2).u32[i % 2], rhs.getBlock(j / 2).u32[j % 2]);
					out.addOffset(value, i + j);
				}
			}

			return out;
		}

	public:
		BigInt operator~() const noexcept
		{
			BigInt out;

			for (uint64_t i = 0; i < s_nSize; i++)
				out.setBlock(i, ~getBlock(i));

			return out;
		}

		BigInt operator<<(const size_t nBits) const noexcept
		{
			BigInt out;

			const size_t nBlockOffset = nBits / 64;
			const size_t nBitOffset   = nBits % 64;

			int_t carry = 0;
			for (uint64_t i = 0; i < s_nSize - nBlockOffset; i++)
			{
				int_t thisBlock = getBlock(i);
				out.setBlock(i + nBlockOffset, thisBlock << nBitOffset | carry >> (64 - nBits));
				carry = thisBlock;
			}

			return out;
		}
	};
}

int main()
{
	for (int i = 0; i < 10; (++i)++)
		std::cout << i << std::endl;

	for (math::int_t i = 0; i < 10; (++i)++)
		std::cout << i << std::endl;

	math::BigInt i = "0x1234567890abcdef";
	math::BigInt j = "0x1234567890abcdef";

	std::cout << i << " - " << j << " = " << i - j << std::endl;

	std::cout << i << " << 4 = " << (i << 4) << std::endl;
	std::cout << i << " << 2 = " << (i << 2) << std::endl;
	
	return 0;
}
