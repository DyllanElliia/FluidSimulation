#include "NeighborTable.h"
#include <string>
NeighborTable::NeighborTable()
    ://初始化；
    n_PointExtraData(0),
    n_PointNumber(0),
    n_PointCapacity(0),
    n_NeighborDataBuf(0),
    n_dataBufSize(0),
    n_dataBufOffset(0),
    n_currPoint(0),
    n_currNeighborCounts(0)
{ }

NeighborTable::~NeighborTable()
{
    //删除两个表；
    //删除邻接表；
    if (n_PointExtraData)
    {
        delete[] n_PointExtraData;
        n_PointExtraData = nullptr;
    }
    //删除邻接表数据缓冲；
    if (n_NeighborDataBuf)
    {
        delete[] n_NeighborDataBuf;
        n_NeighborDataBuf = nullptr;
    }
}

void NeighborTable::reset(unsigned short pointNumber)
{
    int a = sizeof(PointExtraData);
    //检测是否现有容量是否够用；
    //不够用则进行扩容；
    if (pointNumber > n_PointCapacity||!n_PointExtraData)
    {
        //初始化，回收缓冲先哈；
        if (n_PointExtraData)
        {
            delete[] n_PointExtraData;
            n_PointExtraData = nullptr;
        }
        n_PointExtraData = new PointExtraData[a * pointNumber]();
        //这里是对存储容器空间的容量大小进行更新；
        n_PointCapacity = pointNumber;
    }
    //存储当前粒子数；
    n_PointNumber = pointNumber;
    //初始化数据缓冲；
    memset(n_PointExtraData, 0, a * n_PointCapacity);
    //初始化位移；
    n_dataBufOffset = 0;
}
//准备点
void NeighborTable::pointPrepare(unsigned short pointIndex)
{
    //存储当前粒子的索引；
    n_currPoint = pointIndex;
    //初始化邻接点数量；
    n_currNeighborCounts = 0;
}

bool NeighborTable::pointAddNeighbor(unsigned short pointIndex, float distance)
{
    //太多了，加不进去了xxx
    if (n_currNeighborCounts >= MAX_NEIGHBOR_COUNTS)
        return false;
    //存储邻接点；
    n_currNeightborIndex[n_currNeighborCounts] = pointIndex;
    n_currNrighborDistance[n_currNeighborCounts] = distance;
    n_currNeighborCounts++;
    return true;
}

void NeighborTable::pointCommit()
{
    //没东西整啥？爬回去；
    if (n_currNeighborCounts == 0)
        return;
    //计算已用容量；
    unsigned int indexSize = n_currNeighborCounts * sizeof(unsigned short);
    unsigned int distanceSize = n_currNeighborCounts * sizeof(float);
    //若剩余容量不足以用于插入新点，则扩大缓冲；
    if (n_dataBufOffset + indexSize + distanceSize > n_dataBufSize)
        _growDataBuf(n_dataBufOffset + indexSize + distanceSize);
    //设置数据；
    //存储当前点对应的在数据缓冲中的位置（偏移量）；
    n_PointExtraData[n_currPoint].neighborCounts = n_currNeighborCounts;
    n_PointExtraData[n_currPoint].neighborDataOffset = n_dataBufOffset;
    //复制索引点的信息到数据缓冲中；
    memcpy(n_NeighborDataBuf + n_dataBufOffset, n_currNeightborIndex, indexSize);
    n_dataBufOffset += indexSize;
    memcpy(n_NeighborDataBuf + n_dataBufOffset, n_currNrighborDistance, distanceSize);
    n_dataBufOffset += distanceSize;
}

void NeighborTable::_growDataBuf(unsigned int needSize)
{
    unsigned int newSize = n_dataBufSize < 1024 ? 1024 : n_dataBufSize;
    while (needSize > newSize)
        newSize *= 2;
    /*
    if (newSize < 1024)
        newSize = 1024;
    */
    //扩容；
    unsigned char* newBuf = new unsigned char[newSize]();
    if (n_NeighborDataBuf)
    {
        //复制数据；
        memcpy(newBuf, n_NeighborDataBuf, n_dataBufSize);
        delete[] n_NeighborDataBuf;
    }
    //更新数据；
    n_NeighborDataBuf = newBuf;
    n_dataBufSize = newSize;
}

//getNeighborCounts的一般形式；
//pointIndex是点的索引，index是该点邻接表内的邻接索引，获取的neighborIndex为该邻接表的默认索引；
inline void NeighborTable::getNeighborInfo(unsigned short pointIndex, int index, unsigned short& neighborIndex, float& neighborDistance)
{
    PointExtraData neighData = n_PointExtraData[pointIndex];
    unsigned short* indexBuf = (unsigned short*)(n_NeighborDataBuf + neighData.neighborDataOffset);
    float* distanceBuf = (float*)(n_NeighborDataBuf + neighData.neighborDataOffset + sizeof(unsigned short) * neighData.neighborCounts);

    neighborIndex = indexBuf[index];
    neighborDistance = distanceBuf[index];
}