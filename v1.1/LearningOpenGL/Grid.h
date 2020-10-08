#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "PointBuffer.h"

class Box
{
public:
	glm::vec3 min, max;
	Box() :min(glm::vec3(0)), max(glm::vec3(0)) {}
	Box(glm::vec3 min, glm::vec3 max) :min(min), max(max) {}
};
inline Box operator+(Box box1, Box box2);
class Grid
{
public:
	Grid(){}
	~Grid(){}
	//空间划分
	//初始化
	void init(const Box& box, float scale, float cellSize, float border, int* rex);
	//将PointBuffer中所有点重新插入单元网格中；
	void insertPoint(PointBuffer* pointBuffer);
	//输入坐标，返回单元网格索引；
	int findCell(const glm::vec3& p);
	//输入坐标和光滑核函数半径，返回最近的8个单位网格索引；
	void findCells_8(const glm::vec3& p, float radius, int* gridCell);
	void findTwoCells(const glm::vec3& p, float radius, int* gridCell);

	//获得索引为gridCellIndex的Grid对应哈希链表Point的头结点的索引；
	int getGridData(int gridCellIndex);
	//返回网格信息；
	const glm::ivec3* getGridRes()const { return &g_GridRes; }
	const glm::vec3* getGridMin(void) const { return &g_GridMin; }
	const glm::vec3* getGridMax(void) const { return &g_GridMax; }
	const glm::vec3* getGridSize(void) const { return &g_GridSize; }
	//获取对应xyz下的网格索引；
	int getGridCellIndex(float px, float py, float pz);
	//？
	float getDelta() { return g_GridDelta.x; }
private:
	std::vector<int>g_GridData;	//存储当前网格内粒子的索引
	glm::vec3 g_GridMax;	//CellMax
	glm::vec3 g_GridMin;	//CellMin
	glm::ivec3 g_GridRes;	//网格规格 n*m*h = 总共含有的网格的数量
	glm::vec3 g_GridSize;	//网格的空间大小
	glm::vec3 g_GridDelta;	//网格偏移量
	float g_GridCellDelta;	//流体网格边长
};