#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef int MateralIndex;

struct Materal
{
public:
    float f_viscosity;      //粘性
    float f_restDensity;    //静态密度
    float f_PointMass;      //质量

    glm::vec3 color;

    //int pointBegin; //粒子缓冲的起点；
    //int pointEndMO; //粒子缓冲的终点+1；
    Materal()
    {
        f_viscosity = 1.0f;     //粘度
        f_restDensity = 1000.f; //密度
        //f_PointMass = 0.0002f;  //粒子质量
        f_PointMass = f_restDensity * 2e-7;
        color = glm::vec3(1, 1, 1);
    }
    Materal(
        float f_viscosity_,
        float f_restDensity_,
        glm::vec3 color_
    )
        :f_viscosity(f_viscosity_)
        ,f_restDensity(f_restDensity_)
        ,f_PointMass(f_restDensity_ * 2e-7)
        ,color(color_)
    {}
};

