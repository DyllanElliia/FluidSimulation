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
	//�ռ仮��
	//��ʼ��
	void init(const Box& box, float scale, float cellSize, float border, int* rex);
	//��PointBuffer�����е����²��뵥Ԫ�����У�
	void insertPoint(PointBuffer* pointBuffer);
	//�������꣬���ص�Ԫ����������
	int findCell(const glm::vec3& p);
	//��������͹⻬�˺����뾶�����������8����λ����������
	void findCells_8(const glm::vec3& p, float radius, int* gridCell);
	void findTwoCells(const glm::vec3& p, float radius, int* gridCell);

	//�������ΪgridCellIndex��Grid��Ӧ��ϣ����Point��ͷ����������
	int getGridData(int gridCellIndex);
	//����������Ϣ��
	const glm::ivec3* getGridRes()const { return &g_GridRes; }
	const glm::vec3* getGridMin(void) const { return &g_GridMin; }
	const glm::vec3* getGridMax(void) const { return &g_GridMax; }
	const glm::vec3* getGridSize(void) const { return &g_GridSize; }
	//��ȡ��Ӧxyz�µ�����������
	int getGridCellIndex(float px, float py, float pz);
	//��
	float getDelta() { return g_GridDelta.x; }
private:
	std::vector<int>g_GridData;	//�洢��ǰ���������ӵ�����
	glm::vec3 g_GridMax;	//CellMax
	glm::vec3 g_GridMin;	//CellMin
	glm::ivec3 g_GridRes;	//������ n*m*h = �ܹ����е����������
	glm::vec3 g_GridSize;	//����Ŀռ��С
	glm::vec3 g_GridDelta;	//����ƫ����
	float g_GridCellDelta;	//��������߳�
};