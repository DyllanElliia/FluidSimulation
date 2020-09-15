#include "Grid.h"
inline Box operator+(Box box1, Box box2)
{
	return Box(	glm::vec3(	box1.min.x < box2.min.x ? box1.min.x : box2.min.x,
							box1.min.y < box2.min.y ? box1.min.y : box2.min.y,
							box1.min.z < box2.min.z ? box1.min.z : box2.min.z),
				glm::vec3(	box1.max.x > box2.max.x ? box1.max.x : box2.max.x,
							box1.max.y > box2.max.y ? box1.max.y : box2.max.y,
							box1.max.z > box2.max.z ? box1.max.z : box2.max.z));
}

int Grid::getGridData(int gridCellIndex)
{
    //��������Ӧ�����񲻴��ڣ�����-1
    if (gridCellIndex < 0 || gridCellIndex >= g_GridData.size())return -1;
    //���ڣ��򷵻ض�Ӧ��ϣ����Point��ͷ��������
    return g_GridData[gridCellIndex];
}

int Grid::getGridCellIndex(float px, float py, float pz)
{
    /*
    ��ת��Ϊ������ꣻ
    �ٳ�һ��ƫ������ת��Ϊ��������ռ��λ��
    */
    int gx = (int)((px - g_GridMin.x) * g_GridDelta.x);
    int gy = (int)((py - g_GridMin.y) * g_GridDelta.y);
    int gz = (int)((pz - g_GridMin.z) * g_GridDelta.z);
    /*
    Grid��һ������ά�ռ�������ɵ�һά���飻
    �������ռ䣨x, y, z����ת��Ϊ��Ӧ����������������
    gz*g_GridRes.y*g_GridRes.x+gy*g_GridRes.x+gx��
    ����ã�(gz * g_GridRes.y + gy) * g_GridRes.x + gx��
    */
    return (gz * g_GridRes.y + gy) * g_GridRes.x + gx;
}

void Grid::init(const Box& box, float scale, float cellSize, float border, int* rex)
{
    //��ʵ��ϸ������߳�ת��ΪGrid���������������߳���
    //scale��ʵ��ϸ������߳�/���������ϸ������߳���
    g_GridCellDelta = cellSize / scale;
    //��ʼ����������Ŀռ䣻
    //borderΪ�߽磬����ռ䣻
    g_GridMin = box.min;
    g_GridMin -= border;
    g_GridMax = box.max;
    g_GridMax += border;
    //�洢��ʼ����������ռ�ռ��С��
    g_GridSize = g_GridMax;
    g_GridSize -= g_GridMin;
    //����������
    //ceil������ȡ������֤������ɵĵ�λ����ɱ�����ϵͳ��ȫ��Χ��
    g_GridRes.x = (int)ceil(g_GridSize.x / g_GridCellDelta);
    g_GridRes.y = (int)ceil(g_GridSize.y / g_GridCellDelta);
    g_GridRes.z = (int)ceil(g_GridSize.z / g_GridCellDelta);
    //������������ռ䣬������Χ���е�Ԫ�������С����͹����
    //�������С����Ϊ��Ԫ���С�ı�����
    g_GridSize.x = g_GridRes.x * g_GridCellDelta;//cellSize / scale;
    g_GridSize.y = g_GridRes.y * g_GridCellDelta;//cellSize / scale;
    g_GridSize.z = g_GridRes.z * g_GridCellDelta;//cellSize / scale;
    //����ƫ����
    //��λ������ƫ�Ƶĵ�λ��������
    g_GridDelta = g_GridRes;
    g_GridDelta /= g_GridSize;
    //�ܵ�λ������
    int gridTotal = (int)(g_GridRes.x * g_GridRes.y * g_GridRes.z);

    rex[0] = g_GridRes.x * 8;
    rex[1] = g_GridRes.y * 8;
    rex[2] = g_GridRes.z * 8;
    //�������������ռ䣻
    g_GridData.resize(gridTotal);
}

void Grid::insertPoint(PointBuffer* pointBuffer)
{
    //��ʼ����ȫ�����-1��
    std::fill(g_GridData.begin(), g_GridData.end(), -1);
    //�������Ϊ0�ĵ㣻
    Point* p = pointBuffer->getPoint(0);
    //�����������ӣ������Ӳ����ϣ����
    for (unsigned int n = 0, size = pointBuffer->getPointNumber(); n < size; n++, p++)
    {
        //��õ����ڵĵ�Ԫ�����������
        int gs = getGridCellIndex(p->pos.x, p->pos.y, p->pos.z);
        //ÿ�������ڵĵ㻮��Ϊһ������(m_gridData[gs]�Ǹ������������ͷ�ڵ�)��
        if (gs >= 0 && gs < g_GridData.size()) {
            //���ýڵ���Ϊͷ�����룻
            p->next = g_GridData[gs];
            g_GridData[gs] = n;
        }
        else
            p->next = -1;
    }
}

int Grid::findCell(const glm::vec3& p)
{
    //���findCell�£��������꣬���ص�Ԫ����������
    int gc = getGridCellIndex(p.x, p.y, p.z);
    //ѽ��ѽ�ϣ�p�㲻�������ھͷ���-1�£�
    if (gc < 0 || gc >= g_GridData.size())return -1;
    return gc;
}

void Grid::findCells_8(const glm::vec3& p, float radius, int* gridCell)
{
    //��ʼ��gridCell[8]
    for (int i = 0; i < 8; i++)gridCell[i] = -1;
    /*
    ���㵱ǰ���ӵ�⻬����������Χ;
    �⻬�˺�������Box(p-radius,p+radius)�ڣ�
    Box.min-g_Gridminת��Ϊ�����ʵ������ռ�ϵ��
    int((Box.min-g_Gridmin)*g_GridDelta^T)ת��Ϊsph���ڵ������������ϵԭ��ĵ�λ�����������ꣻ
    */
    //**ע��**���㷨������������������Ӧ�������ˮƽƽ��ͶӰ����ȫ�����⻬�˺������ˮƽƽ��ͶӰ��
    //���������汻��ȫ������
    int sph_min_x = int((-radius + p.x - g_GridMin.x) * g_GridDelta.x);
    int sph_min_y = int((-radius + p.y - g_GridMin.y) * g_GridDelta.y);
    int sph_min_z = int((-radius + p.z - g_GridMin.z) * g_GridDelta.z);
    //���������Ϊ(0,0,0)�����������С����ض��ڵ������ޣ������С����Ӧ�������㴦��
    if (sph_min_x < 0) sph_min_x = 0;
    if (sph_min_y < 0) sph_min_y = 0;
    if (sph_min_z < 0) sph_min_z = 0;
    //��ȡ8������
    //ֱ�Ӽ�����������������getGridCellIndex��
    gridCell[0] = (sph_min_z * g_GridRes.y + sph_min_y) * g_GridRes.x + sph_min_x;
    gridCell[1] = gridCell[0] + 1;
    //����
    gridCell[2] = gridCell[0] + g_GridRes.x;
    gridCell[3] = gridCell[2] + 1;
    //��һ���������汻��ȫ�������������Խ�磬��������һ�㣻
    if (sph_min_z + 1 < g_GridRes.z)
    {
        gridCell[4] = gridCell[0] + g_GridRes.y * g_GridRes.x;
        gridCell[5] = gridCell[4] + 1;
        gridCell[6] = gridCell[4] + g_GridRes.x;
        gridCell[7] = gridCell[6] + 1;
    }
    //�����Ƿ�Խ�磻
    if (sph_min_x + 1 >= g_GridRes.x)
    {
        gridCell[1] = -1;
        gridCell[3] = -1;
        gridCell[5] = -1;
        gridCell[7] = -1;
    }
    if (sph_min_y >= g_GridRes.y)
    {
        gridCell[2] = -1;
        gridCell[4] = -1;
        gridCell[6] = -1;
        gridCell[8] = -1;
    }
}

void Grid::findTwoCells(const glm::vec3& p, float radius, int* gridCell)
{
    //4*4*4=64
    //��ʼ����
    for (int i = 0; i < 64; i++)gridCell[i] = -1;
    //���㵱ǰ���ӵ�⻬����������Χ��
    int sph_min_x = ((-radius + p.x - g_GridMin.x) * g_GridDelta.x);
    int sph_min_y = ((-radius + p.y - g_GridMin.y) * g_GridDelta.y);
    int sph_min_z = ((-radius + p.z - g_GridMin.z) * g_GridDelta.z);
    if (sph_min_x < 0) sph_min_x = 0;
    if (sph_min_y < 0) sph_min_y = 0;
    if (sph_min_z < 0) sph_min_z = 0;
    //����λ�ƣ�
    int base = (sph_min_z * g_GridRes.y + sph_min_y) * g_GridRes.x + sph_min_x;
    //�ڻ���λ�ƵĻ��������64������
    for (int z = 0; z < 4; z++)
        for (int y = 0; y < 4; y++)
            for (int x = 0; x < 4; x++)
                if ((sph_min_x + x < g_GridRes.x) && (sph_min_y + y < g_GridRes.y && (sph_min_z + z < g_GridRes.z)))
                    gridCell[16 * z + 4 * y + x] = base + (z * g_GridRes.y + y) * g_GridRes.x + x;
                else
                    gridCell[16 * z + 4 * y + x] = -1; 
}