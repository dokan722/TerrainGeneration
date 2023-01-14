#pragma once

#include <vector>


template <typename T>
class CyclicBuffer2D
{
	int height;
	int width;

	std::vector<T> buffer;
public:
	CyclicBuffer2D(std::vector<T> b, int h, int w) : buffer(b), height(h), width(w) { }

	std::vector<T> MoveLeft()
	{
		std::vector<T> replacement;
		for (int i = 0; i < height; ++i)
			replacement.push_back(buffer[i * width + width - 1]);
		for (int i = 0; i < height; ++i)
		{
			for (int j = width - 1; j > 0; --j)
				buffer[i * width + j] = buffer[i * width + j - 1];
			buffer[i * width] = replacement[i];
		}
		return replacement;
	}

	std::vector<T> MoveRight()
	{
		std::vector<T> replacement;
		for (int i = 0; i < height; ++i)
			replacement.push_back(buffer[i * width]);
		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width - 1; ++j)
				buffer[i * width + j] = buffer[i * width + j + 1];
			buffer[i * width + width - 1] = replacement[i];
		}
		return replacement;
	}

	std::vector<T> MoveDown()
	{
		std::vector<T> replacement;
		for (int i = 0; i < width; ++i)
			replacement.push_back(buffer[i]);
		for (int i = 0; i < height - 1; ++i)
		{
			for (int j = 0; j < width; ++j)
				buffer[i * width + j] = buffer[i * width + j + width];
		}
		for (int i = 0; i < width; ++i)
			buffer[(height - 1) * width + i] = replacement[i];
		return replacement;
	}

	std::vector<T> MoveUp()
	{
		std::vector<T> replacement;
		for (int i = 0; i < width; ++i)
			replacement.push_back(buffer[(height - 1) * width + i]);
		for (int i = height - 1; i > 0; --i)
		{
			for (int j = 0; j < width; ++j)
				buffer[i * width + j] = buffer[i * width + j - width];
		}
		for (int i = 0; i < width; ++i)
			buffer[i] = replacement[i];
		return replacement;
	}

	T& getValue(int x, int y)
	{
		return buffer[x * width + y];
	}
};