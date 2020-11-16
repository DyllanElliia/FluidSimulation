#include "NeighborTable.h"
#include <string>
NeighborTable::NeighborTable()
    ://��ʼ����
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
    //ɾ��������
    //ɾ���ڽӱ�
    if (n_PointExtraData)
    {
        delete[] n_PointExtraData;
        n_PointExtraData = nullptr;
    }
    //ɾ���ڽӱ����ݻ��壻
    if (n_NeighborDataBuf)
    {
        delete[] n_NeighborDataBuf;
        n_NeighborDataBuf = nullptr;
    }
}

void NeighborTable::reset(unsigned short pointNumber)
{
    int a = sizeof(PointExtraData);
    //����Ƿ����������Ƿ��ã�
    //��������������ݣ�
    if (pointNumber > n_PointCapacity||!n_PointExtraData)
    {
        //��ʼ�������ջ����ȹ���
        if (n_PointExtraData)
        {
            delete[] n_PointExtraData;
            n_PointExtraData = nullptr;
        }
        n_PointExtraData = new PointExtraData[a * pointNumber]();
        //�����ǶԴ洢�����ռ��������С���и��£�
        n_PointCapacity = pointNumber;
    }
    //�洢��ǰ��������
    n_PointNumber = pointNumber;
    //��ʼ�����ݻ��壻
    memset(n_PointExtraData, 0, a * n_PointCapacity);
    //��ʼ��λ�ƣ�
    n_dataBufOffset = 0;
}
//׼����
void NeighborTable::pointPrepare(unsigned short pointIndex)
{
    //�洢��ǰ���ӵ�������
    n_currPoint = pointIndex;
    //��ʼ���ڽӵ�������
    n_currNeighborCounts = 0;
}

bool NeighborTable::pointAddNeighbor(unsigned short pointIndex, float distance)
{
    //̫���ˣ��Ӳ���ȥ��xxx
    if (n_currNeighborCounts >= MAX_NEIGHBOR_COUNTS)
        return false;
    //�洢�ڽӵ㣻
    n_currNeightborIndex[n_currNeighborCounts] = pointIndex;
    n_currNrighborDistance[n_currNeighborCounts] = distance;
    n_currNeighborCounts++;
    return true;
}

void NeighborTable::pointCommit()
{
    //û������ɶ������ȥ��
    if (n_currNeighborCounts == 0)
        return;
    //��������������
    unsigned int indexSize = n_currNeighborCounts * sizeof(unsigned short);
    unsigned int distanceSize = n_currNeighborCounts * sizeof(float);
    //��ʣ���������������ڲ����µ㣬�����󻺳壻
    if (n_dataBufOffset + indexSize + distanceSize > n_dataBufSize)
        _growDataBuf(n_dataBufOffset + indexSize + distanceSize);
    //�������ݣ�
    //�洢��ǰ���Ӧ�������ݻ����е�λ�ã�ƫ��������
    n_PointExtraData[n_currPoint].neighborCounts = n_currNeighborCounts;
    n_PointExtraData[n_currPoint].neighborDataOffset = n_dataBufOffset;
    //�������������Ϣ�����ݻ����У�
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
    //���ݣ�
    unsigned char* newBuf = new unsigned char[newSize]();
    if (n_NeighborDataBuf)
    {
        //�������ݣ�
        memcpy(newBuf, n_NeighborDataBuf, n_dataBufSize);
        delete[] n_NeighborDataBuf;
    }
    //�������ݣ�
    n_NeighborDataBuf = newBuf;
    n_dataBufSize = newSize;
}

//getNeighborCounts��һ����ʽ��
//pointIndex�ǵ��������index�Ǹõ��ڽӱ��ڵ��ڽ���������ȡ��neighborIndexΪ���ڽӱ��Ĭ��������
inline void NeighborTable::getNeighborInfo(unsigned short pointIndex, int index, unsigned short& neighborIndex, float& neighborDistance)
{
    PointExtraData neighData = n_PointExtraData[pointIndex];
    unsigned short* indexBuf = (unsigned short*)(n_NeighborDataBuf + neighData.neighborDataOffset);
    float* distanceBuf = (float*)(n_NeighborDataBuf + neighData.neighborDataOffset + sizeof(unsigned short) * neighData.neighborCounts);

    neighborIndex = indexBuf[index];
    neighborDistance = distanceBuf[index];
}