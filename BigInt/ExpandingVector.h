#pragma once

#include <vector>
#include "int_type.h"
#include <iostream>

class ExpandingVector
{
	std::vector<math::int_t> m_vData;
	
public:
	ExpandingVector() noexcept = default;

public:
	math::int_t getBlock(const size_t index) const noexcept
	{
		return m_vData.at(index);
	}

	void setBlock(const size_t index, const math::int_t data) noexcept
	{
		if (index >= m_vData.size())
		{
			resize(index + 1);
			//std::cout << "reallocation" << std::endl;
		}
		m_vData.at(index) = data;
	}

	void resize(const size_t size) noexcept
	{
		m_vData.resize(size);
	}

	size_t size() const noexcept
	{
		return m_vData.size();
	}

	void shrink_to(const size_t size) noexcept
	{
		while (this->size() > size)
			m_vData.pop_back();
	}
};
