#pragma once

#define MAX_NEIGHBOR_COUNTS 80
class NeighborTable
{
public:
    NeighborTable();
    //重置邻接表;
    void reset(unsigned short pointCounts);
    //预备粒子数据;
    void pointPrepare(unsigned short pointIndex);
    //将当前粒子添加邻接表中；
    bool pointAddNeighbor(unsigned short pointIndex, float distance);
    //设置当前粒子在邻接表中的数据；
    void pointCommit();
    //获取邻接表中pointIndex索引的粒子个数；
    int getNeighborCounts(unsigned short pointIndex) { return n_PointExtraData[pointIndex].neighborCounts; }
    //获取索引pointIndex的邻接表中第index个点的数据；
    void getNeighborInfo(unsigned short pointIndex, int index, unsigned short& neighborIndex, float& neighborDistance);

    ~NeighborTable();
private:
    union PointExtraData
    {
        struct
        {
            //偏移
            unsigned neighborDataOffset : 24;
            //个数
            unsigned neighborCounts : 8;
        };
    };
    PointExtraData* n_PointExtraData;   //邻接表信息
    unsigned int n_PointNumber;         //粒子数
    unsigned int n_PointCapacity;       //粒子容量
    unsigned char* n_NeighborDataBuf;   //邻接表的数据缓存
    unsigned int n_dataBufSize;         //bytes 数据缓存尺寸
    unsigned int n_dataBufOffset;       //bytes 数据缓存中的偏移

    //当前粒子的相关数据
    unsigned short n_currPoint; //索引
    int n_currNeighborCounts;   //邻接点数量
    //邻居中点的索引
    unsigned short n_currNeightborIndex[MAX_NEIGHBOR_COUNTS];
    //邻居中点的距离
    float n_currNrighborDistance[MAX_NEIGHBOR_COUNTS];

    void _growDataBuf(unsigned int need_size);//扩容
};