#pragma once
class Materal
{
    float f_viscosity;      //粘性
    float f_restDensity;    //静态密度
    float f_PointMass;      //质量

    int pointBegin; //粒子缓冲的起点；
    int pointEndMO; //粒子缓冲的终点+1；
};

