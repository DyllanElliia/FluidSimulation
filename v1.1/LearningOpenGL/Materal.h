#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef int MateralIndex;

struct Materal
{
public:
    float f_viscosity;      //ճ��
    float f_restDensity;    //��̬�ܶ�
    float f_PointMass;      //����

    glm::vec3 color;

    //int pointBegin; //���ӻ������㣻
    //int pointEndMO; //���ӻ�����յ�+1��
    Materal()
    {
        f_viscosity = 1.0f;     //ճ��
        f_restDensity = 1000.f; //�ܶ�
        //f_PointMass = 0.0002f;  //��������
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

