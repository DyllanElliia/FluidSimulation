#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <thread>
#include <mutex>
#include <chrono>

#include "PointBuffer.h"
#include "Grid.h"
#include "NeighborTable.h"

class FluidSystem
{
public:
    FluidSystem();
    void init
    (   unsigned short maxPointCounts,
        const glm::vec3& wallBox_min,
        const glm::vec3& wallBox_max,
        const glm::vec3& initFluidBox_min,
        const glm::vec3& initFluidBox_max,
        const glm::vec3& gravity
    )
    {
        _init(maxPointCounts, Box(wallBox_min, wallBox_max), Box(initFluidBox_min, initFluidBox_max), gravity);
    }
    void reset(){}
    GLfloat *getPosData() { return &(posData[0]); }
    void setTick(int tick) { tick_ = tick; }
    bool addPoint(glm::vec3 min, glm::vec3 max, glm::vec3 originVelocity);
    //��ȡ��ĳߴ磨�ֽڣ�
    unsigned int getPointStride()const { return sizeof(Point); }
    //��ȡ�������
    unsigned int getPointCounts()const { return f_PointBuffer.getPointNumber(); }
    //��ȡ����㻺��
    const Point* getPointBuf()const { return f_PointBuffer.getPoint(0); }
    //�߼���
    void tick();

    ~FluidSystem();
private:
    //��ʼ��ϵͳ
    void _init(unsigned short maxPointCounts, const Box& wallBox, const Box& initFluidBox, const glm::vec3& gravity);
    //�����ܶȣ�ѹ���Լ����ڹ�ϵ
    void _computerPressure();
    //������ٶ�
    void _computerForce(int i);
    //�ƶ�����
    void _advance(int i);
    //���̼߳���
    void _comForce_And_advance(int i);
    //������ʼҺ���
    void _addFluidVolume(const Box& fluidBox, float spacing);
    void _addFluidVolume(const Box& fluidBox, float spacing, glm::vec3 velocity);

    //���ݳ�Ա
    PointBuffer f_PointBuffer;
    Grid f_GridContainer;
    NeighborTable f_NeighborTable;

    //��λ�û�������(x,y,z)
    std::vector<GLfloat>posData;

    //SPH�⻬��
    float f_kernelPoly6;
    float f_kernelSpiky;
    float f_kernelViscosity;

    //��������
    float f_PointDistance;  //�뾶
    float f_unitScale;      //�ߴ絥λ
    float f_viscosity;      //ճ��
    float f_restDensity;    //��̬�ܶ�
    float f_PointMass;      //����
    float f_smoothRadius;   //�⻬�˰뾶
    float f_gasConstantK;   //���峣��
    float f_boundaryStiffness;  //�߽����
    float f_boundaryDampening;  //�߽�����
    float f_speedLimiting;  //�ٶ�����
    glm::vec3 f_gravityDir; //����ʸ��

    int f_rexSize[3];//����ߴ�

    Box f_sphWallBox;

    //������ӵļ��֡����
    int tick_ = 0;
    int count = 0;
};