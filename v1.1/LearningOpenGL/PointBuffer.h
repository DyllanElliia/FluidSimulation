#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
struct Point
{
	glm::vec3 pos;			//点位置
	float density;			//密度
	float pressure;			//压力
	glm::vec3 accel;		//加速度
	glm::vec3 velocity;		//速度
	glm::vec3 velocityEval;	//最后速度
	short materalIndex;		//材质索引
	//用于哈希表索引
	int next;//指向下一个点的索引
};
#define ELEM_MAX 1000000
class PointBuffer
{
public:
	PointBuffer();
	//重置粒子缓冲容量
	void resetPointBuffer(unsigned int capacity);
	//获得粒子个数
	unsigned int getPointNumber()const { return p_PointNumber; }
	//获得第index个点
	Point* getPoint(unsigned int index) { return &(p_PointBuffer[index]); }
	const Point* getPoint(unsigned int index)const { return &(p_PointBuffer[index]); }
	//缓冲中添加新点并返回该点
	Point* addPointReuse();
	Point* addPointReuse(int materalIndex);	//带粒子材质索引
	virtual ~PointBuffer();
private:
	Point* p_PointBuffer;//粒子缓冲数组
	unsigned int p_BufferCapcitySize;//粒子缓冲数组大小
	unsigned int p_PointNumber;//粒子个数

	void clearPointBuffer();
};