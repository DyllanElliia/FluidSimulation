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
	//�����ӻ��岻Ϊ��ʱ����ջ���
	if (p_PointBuffer != nullptr)
		clearPointBuffer();
	//����������0ʱ��Ϊ���ӻ������ռ�
	if (p_BufferCapcitySize > 0)
		p_PointBuffer = new Point[p_BufferCapcitySize]();
	//�������Ӹ���
	p_PointNumber = 0;
}

Point* PointBuffer::addPointReuse()
{
	//�����Ӹ������ڵ������ӻ�������������Ҫ�����ӻ�������
	if (p_PointNumber >= p_BufferCapcitySize)
	{
		//��ֹ���ݺ����
		if (p_BufferCapcitySize * 2 > ELEM_MAX)
		{
			printf("opppp...\n");
			return &(p_PointBuffer[p_PointNumber - 1]);
		}
		//Ϊ���ӻ�������
		p_BufferCapcitySize *= 2;
		Point* data = new Point[p_BufferCapcitySize]();
		memcpy(data, p_PointBuffer, p_PointNumber * sizeof(Point));
		delete[] p_PointBuffer;
		p_PointBuffer = data;
	}
	//�������
	Point* point = &(p_PointBuffer[(p_PointNumber++)]);
	point->pos = glm::vec3(0);
	point->density = 0;
	point->pressure = 0;
	point->accel = glm::vec3(0);
	point->velocity = glm::vec3(0);
	point->velocityEval = glm::vec3(0);
	point->materalIndex = 0;
	//�������������ڹ�������Ĺ�ϣ��
	point->next = -1;
	//������ӵ�������
	return point;
}

Point* PointBuffer::addPointReuse(int materalIndex)
{
	//�����Ӹ������ڵ������ӻ�������������Ҫ�����ӻ�������
	if (p_PointNumber >= p_BufferCapcitySize)
	{
		//��ֹ���ݺ����
		if (p_BufferCapcitySize * 2 > ELEM_MAX)
		{
			printf("opppp...\n");
			return &(p_PointBuffer[p_PointNumber - 1]);
		}
		//Ϊ���ӻ�������
		p_BufferCapcitySize *= 2;
		Point* data = new Point[p_BufferCapcitySize]();
		memcpy(data, p_PointBuffer, p_PointNumber * sizeof(Point));
		delete[] p_PointBuffer;
		p_PointBuffer = data;
	}
	//�������
	Point* point = &(p_PointBuffer[(p_PointNumber++)]);
	point->pos = glm::vec3(0);
	point->density = 0;
	point->pressure = 0;
	point->accel = glm::vec3(0);
	point->velocity = glm::vec3(0);
	point->velocityEval = glm::vec3(0);
	point->materalIndex = materalIndex;
	//�������������ڹ�������Ĺ�ϣ��
	point->next = -1;
	//������ӵ�������
	return point;
}