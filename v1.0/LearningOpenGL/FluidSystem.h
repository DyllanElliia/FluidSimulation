#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

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
    //获取点的尺寸（字节）
    unsigned int getPointStride()const { return sizeof(Point); }
    //获取点的数量
    unsigned int getPointCounts()const { return f_PointBuffer.getPointNumber(); }
    //获取流体点缓存
    const Point* getPointBuf()const { return f_PointBuffer.getPoint(0); }
    //逻辑桢
    void tick();

    ~FluidSystem();
private:
    //初始化系统
    void _init(unsigned short maxPointCounts, const Box& wallBox, const Box& initFluidBox, const glm::vec3& gravity);
    //计算密度，压力以及相邻关系
    void _computerPressure();
    //计算加速度
    void _computerForce();
    //移动粒子
    void _advance();
    //创建初始液体块
    void _addFluidVolume(const Box& fluidBox, float spacing);
    void _addFluidVolume(const Box& fluidBox, float spacing, glm::vec3 velocity);

    //数据成员
    PointBuffer f_PointBuffer;
    Grid f_GridContainer;
    NeighborTable f_NeighborTable;

    //点位置缓存数据(x,y,z)
    std::vector<GLfloat>posData;

    //SPH光滑核
    float f_kernelPoly6;
    float f_kernelSpiky;
    float f_kernelViscosity;

    //其他参数
    float f_PointDistance;  //半径
    float f_unitScale;      //尺寸单位
    float f_viscosity;      //粘性
    float f_restDensity;    //静态密度
    float f_PointMass;      //质量
    float f_smoothRadius;   //光滑核半径
    float f_gasConstantK;   //气体常量
    float f_boundaryStiffness;  //边界刚性
    float f_boundaryDampening;  //边界阻尼
    float f_speedLimiting;  //速度限制
    glm::vec3 f_gravityDir; //重力矢量

    int f_rexSize[3];//网格尺寸

    Box f_sphWallBox;

    //流体添加的间隔帧计算
    int tick_ = 0;
    int count = 0;
};