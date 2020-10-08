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
    //若索引对应的网格不存在，返回-1
    if (gridCellIndex < 0 || gridCellIndex >= g_GridData.size())return -1;
    //存在，则返回对应哈希链表Point的头结点的索引
    return g_GridData[gridCellIndex];
}

int Grid::getGridCellIndex(float px, float py, float pz)
{
    /*
    先转换为相对坐标；
    再乘一个偏移量，转换为网格坐标空间的位置
    */
    int gx = (int)((px - g_GridMin.x) * g_GridDelta.x);
    int gy = (int)((py - g_GridMin.y) * g_GridDelta.y);
    int gz = (int)((pz - g_GridMin.z) * g_GridDelta.z);
    /*
    Grid是一个由三维空间数据组成的一维数组；
    索引到空间（x, y, z），转换为对应的数组索引方法：
    gz*g_GridRes.y*g_GridRes.x+gy*g_GridRes.x+gx；
    化简得：(gz * g_GridRes.y + gy) * g_GridRes.x + gx；
    */
    return (gz * g_GridRes.y + gy) * g_GridRes.x + gx;
}

void Grid::init(const Box& box, float scale, float cellSize, float border, int* rex)
{
    //将实际细胞网格边长转换为Grid中所需的流体网格边长；
    //scale：实际细胞网格边长/流体网格的细胞网格边长；
    g_GridCellDelta = cellSize / scale;
    //初始化流体网格的空间；
    //border为边界，扩充空间；
    g_GridMin = box.min;
    g_GridMin -= border;
    g_GridMax = box.max;
    g_GridMax += border;
    //存储初始流体网格所占空间大小；
    g_GridSize = g_GridMax;
    g_GridSize -= g_GridMin;
    //流体网格规格；
    //ceil：向下取整，保证最后生成的单位网格可被网格系统完全包围；
    g_GridRes.x = (int)ceil(g_GridSize.x / g_GridCellDelta);
    g_GridRes.y = (int)ceil(g_GridSize.y / g_GridCellDelta);
    g_GridRes.z = (int)ceil(g_GridSize.z / g_GridCellDelta);
    //更新流体网格空间，构建包围所有单元网格的最小网格凸包；
    //将网格大小调整为单元格大小的倍数；
    g_GridSize.x = g_GridRes.x * g_GridCellDelta;//cellSize / scale;
    g_GridSize.y = g_GridRes.y * g_GridCellDelta;//cellSize / scale;
    g_GridSize.z = g_GridRes.z * g_GridCellDelta;//cellSize / scale;
    //计算偏移量
    //单位长度下偏移的单位网格数量
    g_GridDelta = g_GridRes;
    g_GridDelta /= g_GridSize;
    //总单位网格数
    int gridTotal = (int)(g_GridRes.x * g_GridRes.y * g_GridRes.z);

    rex[0] = g_GridRes.x * 8;
    rex[1] = g_GridRes.y * 8;
    rex[2] = g_GridRes.z * 8;
    //给流体网格分配空间；
    g_GridData.resize(gridTotal);
}

void Grid::insertPoint(PointBuffer* pointBuffer)
{
    //初始化，全部填充-1；
    std::fill(g_GridData.begin(), g_GridData.end(), -1);
    //获得索引为0的点；
    Point* p = pointBuffer->getPoint(0);
    //遍历索引粒子，将粒子插入哈希链表
    for (unsigned int n = 0, size = pointBuffer->getPointNumber(); n < size; n++, p++)
    {
        //获得点所在的单元网格的索引；
        int gs = getGridCellIndex(p->pos.x, p->pos.y, p->pos.z);
        //每个网格内的点划分为一个链表(m_gridData[gs]是该网格中链表的头节点)；
        if (gs >= 0 && gs < g_GridData.size()) {
            //将该节点作为头结点插入；
            p->next = g_GridData[gs];
            g_GridData[gs] = n;
        }
        else
            p->next = -1;
    }
}

int Grid::findCell(const glm::vec3& p)
{
    //真就findCell呗，输入坐标，返回单元网格索引；
    int gc = getGridCellIndex(p.x, p.y, p.z);
    //呀嘞呀嘞，p点不在网格内就返回-1呗；
    if (gc < 0 || gc >= g_GridData.size())return -1;
    return gc;
}

void Grid::findCells_8(const glm::vec3& p, float radius, int* gridCell)
{
    //初始化gridCell[8]
    for (int i = 0; i < 8; i++)gridCell[i] = -1;
    /*
    计算当前粒子点光滑核所在网格范围;
    光滑核函数球在Box(p-radius,p+radius)内；
    Box.min-g_Gridmin转变为相对于实际网格空间系；
    int((Box.min-g_Gridmin)*g_GridDelta^T)转变为sph所在的最靠近流体网格系原点的单位网格索引坐标；
    */
    //**注意**该算法绝对满足出来的坐标对应的网格的水平平面投影包完全包含光滑核函数球的水平平面投影；
    //并满足下面被完全包含；
    int sph_min_x = int((-radius + p.x - g_GridMin.x) * g_GridDelta.x);
    int sph_min_y = int((-radius + p.y - g_GridMin.y) * g_GridDelta.y);
    int sph_min_z = int((-radius + p.z - g_GridMin.z) * g_GridDelta.z);
    //若相对坐标为(0,0,0)，则计算后的最小坐标必定在第七象限，因此最小坐标应该做归零处理；
    if (sph_min_x < 0) sph_min_x = 0;
    if (sph_min_y < 0) sph_min_y = 0;
    if (sph_min_z < 0) sph_min_z = 0;
    //获取8个网格
    //直接计算索引，方法类似getGridCellIndex；
    gridCell[0] = (sph_min_z * g_GridRes.y + sph_min_y) * g_GridRes.x + sph_min_x;
    gridCell[1] = gridCell[0] + 1;
    //换行
    gridCell[2] = gridCell[0] + g_GridRes.x;
    gridCell[3] = gridCell[2] + 1;
    //不一定满足上面被完全包含，因此若不越界，则再增加一层；
    if (sph_min_z + 1 < g_GridRes.z)
    {
        gridCell[4] = gridCell[0] + g_GridRes.y * g_GridRes.x;
        gridCell[5] = gridCell[4] + 1;
        gridCell[6] = gridCell[4] + g_GridRes.x;
        gridCell[7] = gridCell[6] + 1;
    }
    //计算是否越界；
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
    //初始化；
    for (int i = 0; i < 64; i++)gridCell[i] = -1;
    //计算当前粒子点光滑核所在网格范围；
    int sph_min_x = ((-radius + p.x - g_GridMin.x) * g_GridDelta.x);
    int sph_min_y = ((-radius + p.y - g_GridMin.y) * g_GridDelta.y);
    int sph_min_z = ((-radius + p.z - g_GridMin.z) * g_GridDelta.z);
    if (sph_min_x < 0) sph_min_x = 0;
    if (sph_min_y < 0) sph_min_y = 0;
    if (sph_min_z < 0) sph_min_z = 0;
    //基础位移；
    int base = (sph_min_z * g_GridRes.y + sph_min_y) * g_GridRes.x + sph_min_x;
    //在基础位移的基础上填充64个网格；
    for (int z = 0; z < 4; z++)
        for (int y = 0; y < 4; y++)
            for (int x = 0; x < 4; x++)
                if ((sph_min_x + x < g_GridRes.x) && (sph_min_y + y < g_GridRes.y && (sph_min_z + z < g_GridRes.z)))
                    gridCell[16 * z + 4 * y + x] = base + (z * g_GridRes.y + y) * g_GridRes.x + x;
                else
                    gridCell[16 * z + 4 * y + x] = -1; 
}