#pragma once

#define MAX_NEIGHBOR_COUNTS 80
class NeighborTable
{
public:
    NeighborTable();
    //�����ڽӱ�;
    void reset(unsigned short pointCounts);
    //Ԥ����������;
    void pointPrepare(unsigned short pointIndex);
    //����ǰ��������ڽӱ��У�
    bool pointAddNeighbor(unsigned short pointIndex, float distance);
    //���õ�ǰ�������ڽӱ��е����ݣ�
    void pointCommit();
    //��ȡ�ڽӱ���pointIndex���������Ӹ�����
    int getNeighborCounts(unsigned short pointIndex) { return n_PointExtraData[pointIndex].neighborCounts; }
    //��ȡ����pointIndex���ڽӱ��е�index��������ݣ�
    void getNeighborInfo(unsigned short pointIndex, int index, unsigned short& neighborIndex, float& neighborDistance);

    ~NeighborTable();
private:
    union PointExtraData
    {
        struct
        {
            //ƫ��
            unsigned neighborDataOffset : 24;
            //����
            unsigned neighborCounts : 8;
        };
    };
    PointExtraData* n_PointExtraData;   //�ڽӱ���Ϣ
    unsigned int n_PointNumber;         //������
    unsigned int n_PointCapacity;       //��������
    unsigned char* n_NeighborDataBuf;   //�ڽӱ�����ݻ���
    unsigned int n_dataBufSize;         //bytes ���ݻ���ߴ�
    unsigned int n_dataBufOffset;       //bytes ���ݻ����е�ƫ��

    //��ǰ���ӵ��������
    unsigned short n_currPoint; //����
    int n_currNeighborCounts;   //�ڽӵ�����
    //�ھ��е������
    unsigned short n_currNeightborIndex[MAX_NEIGHBOR_COUNTS];
    //�ھ��е�ľ���
    float n_currNrighborDistance[MAX_NEIGHBOR_COUNTS];

    void _growDataBuf(unsigned int need_size);//����
};