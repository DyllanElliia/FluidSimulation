#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
struct Point
{
	glm::vec3 pos;			//��λ��
	float density;			//�ܶ�
	float pressure;			//ѹ��
	glm::vec3 accel;		//���ٶ�
	glm::vec3 velocity;		//�ٶ�
	glm::vec3 velocityEval;	//����ٶ�
	//���ڹ�ϣ������
	int next;//ָ����һ���������
};
#define ELEM_MAX 1000000
class PointBuffer
{
public:
	PointBuffer();
	//�������ӻ�������
	void resetPointBuffer(unsigned int capacity);
	//������Ӹ���
	unsigned int getPointNumber()const { return p_PointNumber; }
	//��õ�index����
	Point* getPoint(unsigned int index) { return &(p_PointBuffer[index]); }
	const Point* getPoint(unsigned int index)const { return &(p_PointBuffer[index]); }
	//����������µ㲢���ظõ�
	Point* addPointReuse();
	virtual ~PointBuffer();
private:
	Point* p_PointBuffer;//���ӻ�������
	unsigned int p_BufferCapcitySize;//���ӻ��������С
	unsigned int p_PointNumber;//���Ӹ���

	void clearPointBuffer();
};