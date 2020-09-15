#include "PointBuffer.h"
#include <stdio.h>
PointBuffer::PointBuffer() :p_PointBuffer(NULL), p_BufferCapcitySize(0), p_PointNumber(0) {}

void PointBuffer::clearPointBuffer()
{
	delete[] p_PointBuffer;
	p_PointBuffer = nullptr;
}

PointBuffer::~PointBuffer()
{
	clearPointBuffer();
}

void PointBuffer::resetPointBuffer(unsigned int capacity)
{
	p_BufferCapcitySize = capacity;
	//当粒子缓冲不为空时，清空缓冲
	if (p_PointBuffer != nullptr)
		clearPointBuffer();
	//当容量大于0时，为粒子缓冲分配空间
	if (p_BufferCapcitySize > 0)
		p_PointBuffer = new Point[p_BufferCapcitySize]();
	//更新粒子个数
	p_PointNumber = 0;
}

Point* PointBuffer::addPointReuse()
{
	//若粒子个数大于等于粒子缓冲容量，则需要给粒子缓冲扩容
	if (p_PointNumber >= p_BufferCapcitySize)
	{
		//防止扩容后溢出
		if (p_BufferCapcitySize * 2 > ELEM_MAX)
		{
			return &(p_PointBuffer[p_PointNumber - 1]);
		}
		//为粒子缓冲扩容
		p_BufferCapcitySize *= 2;
		Point* data = new Point[p_BufferCapcitySize]();
		memcpy(data, p_PointBuffer, p_PointNumber * sizeof(Point));
		delete[] p_PointBuffer;
		p_PointBuffer = data;
	}
	//添加粒子
	Point* point = &(p_PointBuffer[(p_PointNumber++)]);
	point->pos = glm::vec3(0);
	point->density = 0;
	point->pressure = 0;
	point->accel = glm::vec3(0);
	point->velocity = glm::vec3(0);
	point->velocityEval = glm::vec3(0);
	//粒子索引，用于构建后面的哈希表
	point->next = -1;
	//返回添加的新粒子
	return point;
}