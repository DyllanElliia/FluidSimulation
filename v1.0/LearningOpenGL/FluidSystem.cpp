#include "FluidSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

FluidSystem::FluidSystem()
{
    //��ʼ��������ˮ��
    f_unitScale = 0.004f;   //�ߴ絥λ
    f_viscosity = 1.0f;     //ճ��
    f_restDensity = 1000.f; //�ܶ�
    f_PointMass = 0.0002f;  //��������
    f_gasConstantK = 1.0f;  //�������巽�̳���
    f_smoothRadius = 0.01f; //�⻬�˰뾶
    f_PointDistance = 0.0;  //���Ӱ뾶
    //��ʼ������ߴ磻
    f_rexSize[0] = 0;
    f_rexSize[1] = 0;
    f_rexSize[2] = 0;
    //�������ƣ�
    f_boundaryStiffness = 10000.f;  //�߽����
    f_boundaryDampening = 256;      //�߽�����
    f_speedLimiting = 200;          //�ٶ�����
    //���ù⻬�˺���������
    //Poly6 Kernel
    f_kernelPoly6 = 315.0f / (64.0f * 3.141592f * pow(f_smoothRadius, 9));
    //Spiky Kernel
    f_kernelSpiky = -45.0f / (3.141592f * pow(f_smoothRadius, 6));
    //Viscosity Kernel
    f_kernelViscosity = 45.0f / (3.141592f * pow(f_smoothRadius, 6));
}

FluidSystem::~FluidSystem()
{}

//���������еĵ�
void FluidSystem::_addFluidVolume(const Box& fluidBox, float spacing)
{
    //���ղ���Ϊspacing���пռ������
    for (float z = fluidBox.max.z; z >= fluidBox.min.z; z -= spacing)
        for (float y = fluidBox.min.y; y <= fluidBox.max.y; y += spacing)
            for (float x = fluidBox.min.x; x <= fluidBox.max.x; x += spacing)
            {
                //��ӵ㣻
                Point* p = f_PointBuffer.addPointReuse();
                //��ʼ�������ꣻ
                p->pos = glm::vec3(x, y, z);
            }
}

void FluidSystem::_addFluidVolume(const Box& fluidBox, float spacing, glm::vec3 velocity)
{
    //���ղ���Ϊspacing���пռ������
    for (float z = fluidBox.max.z; z >= fluidBox.min.z; z -= spacing)
        for (float y = fluidBox.min.y; y <= fluidBox.max.y; y += spacing)
            for (float x = fluidBox.min.x; x <= fluidBox.max.x; x += spacing)
            {
                //��ӵ㣻
                Point* p = f_PointBuffer.addPointReuse();
                //��ʼ�������ꣻ
                p->pos = glm::vec3(x, y, z);
                p->velocity = velocity;
            }
}

bool FluidSystem::addPoint(glm::vec3 min, glm::vec3 max, glm::vec3 originVelocity)
{
    if (f_PointBuffer.getPointNumber() >= ELEM_MAX)
        return false;
    if (count++ >= tick_)
    {
        _addFluidVolume(Box(min, max), f_PointDistance / f_unitScale, originVelocity);
        posData.clear();
        posData = std::vector<GLfloat>(3 * f_PointBuffer.getPointNumber());
        count = 0;
    }
    return true;
}

void FluidSystem::_init(unsigned short maxPointCounts, const Box& wallBox, const Box& initFluidBox, const glm::vec3& gravity)
{
    //��ʼ�����壬���컺������
    f_PointBuffer.resetPointBuffer(maxPointCounts);
    //�洢�����������������
    f_sphWallBox = wallBox;
    //��������������������ռ䣻
    //��������ߴ�(2r)��
    f_GridContainer.init(wallBox, f_unitScale, f_smoothRadius * 2.0, 1.0, f_rexSize);
    //д��������
    f_gravityDir = gravity;
    //�������Ӽ��
    //(����/�ܶ�)^(1/3)=���Ӽ�ࣻ
    f_PointDistance = pow(f_PointMass / f_restDensity, 1.0 / 3.0);
    //��initFluidBox�ռ��й����ʼ���壻
    _addFluidVolume(initFluidBox, f_PointDistance / f_unitScale);
    //�������ӻ��壬���ڿ��ٰ�VBO��
    posData = std::vector<GLfloat>(3 * f_PointBuffer.getPointNumber(), 0);
}

void FluidSystem::_computerPressure()
{
    // �⻬�˺����뾶��ƽ����ʹ�ô����࣬�����ȡ������
    float smoothR2 = f_smoothRadius * f_smoothRadius;
    // �����ڽӱ�
    // ȫ����ʼ����
    f_NeighborTable.reset(f_PointBuffer.getPointNumber());
    // �������е㣬����ÿ������ܶȣ�
    for (unsigned int i = 0, limit = f_PointBuffer.getPointNumber(); i < limit; i++)
    {
        // �������ֵΪi�����ӣ�
        Point* pi = f_PointBuffer.getPoint(i);
        float sum = 0.0;
        // ׼�����洢��ǰ���ӵ�������Ȼ���ʼ���ڽӵ�������
        f_NeighborTable.pointPrepare(i);
        // �洢�������8����Ԫ�����ֵ�����飻
        int gridCell[8];
        // ��ÿ���pi�Ĺ⻬�˺������8����Ԫ����
        f_GridContainer.findCells_8(pi->pos, f_smoothRadius / f_unitScale, gridCell);
        // ����8�������е��������ӣ�
        for (int cell = 0; cell < 8; cell++)
        {
            // cell����ֵΪ-1������ǰԪ�ز������κ�����
            if (gridCell[cell] == -1)
                continue;
            // ��ȡcell�����ĵ�λ�����ڴ洢�ĵ�һ�����ӵ�������
            int pndx = f_GridContainer.getGridData(gridCell[cell]);
            // ��������������ϣ����
            // ֱ��-1���˳�ѭ����
            while (pndx != -1) {
                // �������Ϊpndx�����ӣ�
                Point* pj = f_PointBuffer.getPoint(pndx);
                // ����ȣ����ǵ�ǰ�㣻
                if (pj == pi)
                    // ����������˵��ri-rj=0������ֱ�������ò����������������ģ�
                    sum += pow(smoothR2, 3.0);
                else
                {
                    // ����ʵ�ʵ�������
                    glm::vec3 pi_pj = (pi->pos - pj->pos) * f_unitScale;
                    float r2 = pi_pj.x * pi_pj.x + pi_pj.y * pi_pj.y + pi_pj.z * pi_pj.z;
                    // �жϸ������Ƿ񱻹⻬�˺�����������ǣ����ۼӸ����ӵ�ֵ��
                    if (smoothR2 > r2)
                    {
                        float h2_r2 = smoothR2 - r2;
                        sum += pow(h2_r2, 3.0);
                        // �õ�ĵ��ڽӱ����ˣ��޷��������ڵ㣻
                        if (!f_NeighborTable.pointAddNeighbor(pndx, glm::sqrt(r2)))
                            // ����ѭ����
                            goto FINALL;
                    }
                }
                //�����һ�����ӵ�����ֵ��
                pndx = pj->next;
            }
        }
    FINALL:
        //����ǰ�ڽӱ�д���ڽӱ����У�
        f_NeighborTable.pointCommit();
        //��ɼ��㣻
        pi->density = f_kernelPoly6 * f_PointMass * sum;
        //����ѹǿ��
        pi->pressure = (pi->density - f_restDensity) * f_gasConstantK;
    }
}

int max = 0;
#define thread_max 25

void FluidSystem::_computerForce(int i)
{
    int begin = int(float(i) / float(thread_max) * max);
    int end = int(float(i + 1) / float(thread_max) * max);
    //printf("max:%6d  CF:min: %6d  CF:min: %6d\n", max, begin, end);
    //�������е㣬��ÿһ�����ճ�Ƚ��м��㣻
    //for (unsigned int i = 0, limit = f_PointBuffer.getPointNumber(); i < limit; i++)
    for (unsigned int i = begin; i < end; i++)
    {
        //��ȡ��ǰ����ֵi��Ӧ�����ӣ�
        Point* pi = f_PointBuffer.getPoint(i);
        //��ʼ��ճ�ȣ�
        //glm::vec3 accelSum = glm::vec3(0.0);
        //���ٶȣ�
        glm::vec3 pressureS = glm::vec3(0.0);
        glm::vec3 viscosityS = glm::vec3(0.0);
        //��ȡ֮ǰ�洢����ӱ����ڸ����ӵĹ⻬�˺����뾶�����Ӹ�����
        int neighborCounts = f_NeighborTable.getNeighborCounts(i);
        //�����⻬�˺������ڵ��������ӣ����ճ�Ⱥͣ�
        for (unsigned int j = 0; j < neighborCounts; j++)
        {
            //�ڽӱ�����������
            unsigned short neighborIndex;
            
            float distanceR;
            //�����i������������i������Ӧ���ڽӱ��е�j���д洢�������������������ĵľ��뷵�ظ�neighborIndex��distanceR��
            f_NeighborTable.getNeighborInfo(i, j, neighborIndex, distanceR);
            //�������ֵneighborIndex��Ӧ�����ӣ�
            Point* pj = f_PointBuffer.getPoint(neighborIndex);
            
            glm::vec3 ri_rj = (pi->pos - pj->pos) * f_unitScale;
            
            float h_r = f_smoothRadius - distanceR;
            //����ʹ��һ�Σ����д�ɺꣻ
            //ѹ�����㹫ʽ��
            #define pterm (h_r * h_r * (pi->pressure + pj->pressure) / (pj->density))
            pressureS += ri_rj * pterm / distanceR;
            //����ʹ��һ�Σ����д�ɺꣻ
            //ճ�ȼ��㹫ʽ��
            #define vterm (h_r / (pj->density))
            viscosityS += (pj->velocityEval - pi->velocityEval) * vterm;
        }
        //pressure����ȡ�����Ĺ����
        #define pressureH (-f_kernelSpiky * f_PointMass / (2.0f * pi->density))
        //viscosity����ȡ�����Ĺ����
        #define viscosityH (f_kernelViscosity * f_viscosity * f_PointMass / (pi->density))
        //�������ļ��ٶȣ�
        pi->accel = f_gravityDir + pressureH * pressureS + viscosityH * viscosityS;
    }
}

//std::mutex posDataMU;

void FluidSystem::_advance(int i) {
    int begin = int(float(i) / float(thread_max) * max);
    int end = int(float(i+1) / float(thread_max) * max);
    //����ʱ������
    float deltaTime = 0.001;
    //����ٶȵ�ƽ����
    float speedLimit2 = f_speedLimiting * f_speedLimiting;
    //��ģ����������ӵ���������Ļ����Բ�����գ�
    //��ģ���������˺ͻ�������������ᷢ���仯��������������գ�
    //posData.clear();
    //�����㣬��һ����
    //for (unsigned int i = 0, limit = f_PointBuffer.getPointNumber(); i < limit; i++)
    for (unsigned int i = begin; i < end; i++)
    {
        //��һ��õ㣻
        Point* p = f_PointBuffer.getPoint(i);
        //��ȡp�ĵ�ǰ���ٶȣ�
        glm::vec3 accel = p->accel;
        //ȡ�ٶ�ʸ����ģ��ƽ����
        float accel2 = accel.x * accel.x + accel.y * accel.y + accel.z * accel.z;
        //���ٶ�ʸ����ģ��ƽ��������ٶȵ�ƽ�����Ƚϣ�
        //�ٶȵ�ģ���ܴ�������ٶ����ƣ�
        if (accel2 > speedLimit2)
            //����������ԭ�ٶ�ʸ����һ��������ΪģΪ����ٶ����Ƶ��ٶ�ʸ����
            //����Ϊ\frac{����ٶ�����}{�ٶȵ�ģ}��
            accel *= f_speedLimiting / glm::sqrt(accel2);

        float diff;
        //�߽����
        //Z����߽�
        diff = (1 - (p->pos.z - f_sphWallBox.min.z)) * f_unitScale;
        if (diff > 0.0f)
        {
            glm::vec3 norm(0, 0, 1.0);
            float adj = f_boundaryStiffness * diff - f_boundaryDampening * glm::dot(norm, p->velocityEval);
            //accel.x += adj * norm.x;
            //accel.y += adj * norm.y;
            accel.z += adj * norm.z;
        }
        diff = (1 - (f_sphWallBox.max.z - p->pos.z)) * f_unitScale;
        if (diff > 0.0f)
        {
            glm::vec3 norm(0, 0, -1.0);
            float adj = f_boundaryStiffness * diff - f_boundaryDampening * glm::dot(norm, p->velocityEval);
            //accel.x += adj * norm.x;
            //accel.y += adj * norm.y;
            accel.z += adj * norm.z;
        }

        //X����߽�
        diff = (1 - (p->pos.x - f_sphWallBox.min.x)) * f_unitScale;
        if (diff > 0.0f)
        {
            glm::vec3 norm(1.0, 0, 0);
            float adj = f_boundaryStiffness * diff - f_boundaryDampening * glm::dot(norm, p->velocityEval);
            accel.x += adj * norm.x;
            //accel.y += adj * norm.y;
            //accel.z += adj * norm.z;
        }
        diff = (1 - (f_sphWallBox.max.x - p->pos.x)) * f_unitScale;
        if (diff > 0.0f)
        {
            glm::vec3 norm(-1.0, 0, 0);
            float adj = f_boundaryStiffness * diff - f_boundaryDampening * glm::dot(norm, p->velocityEval);
            accel.x += adj * norm.x;
            //accel.y += adj * norm.y;
            //accel.z += adj * norm.z;
        }
        
        //Y����߽�
        diff = (1 - (p->pos.y - f_sphWallBox.min.y)) * f_unitScale;
        if (diff > 0.0f)
        {
            glm::vec3 norm(0, 1.0, 0);
            float adj = f_boundaryStiffness * diff - f_boundaryDampening * glm::dot(norm, p->velocityEval);
            //accel.x += adj * norm.x;
            accel.y += adj * norm.y;
            //accel.z += adj * norm.z;
        }
        diff = (1 - (f_sphWallBox.max.y - p->pos.y)) * f_unitScale;
        if (diff > 0.0f)
        {
            glm::vec3 norm(0, -1.0, 0);
            float adj = f_boundaryStiffness * diff - f_boundaryDampening * glm::dot(norm, p->velocityEval);
            //accel.x += adj * norm.x;
            accel.y += adj * norm.y;
            //accel.z += adj * norm.z;
        }
        // λ�ü��㣻
        glm::vec3 vnext = p->velocity + accel * deltaTime;
        p->velocityEval = (p->velocity + vnext) * 0.5f;
        p->velocity = vnext;
        p->pos += vnext * deltaTime / f_unitScale;
        //����λ�����ݣ����ڿ��ٰ�VBO��
        posData[3 * i] = p->pos.x;
        posData[3 * i + 1] = p->pos.y;
        posData[3 * i + 2] = p->pos.z;
    }
}

std::mutex mtx;             // ȫ�ֻ�����
std::condition_variable cr; // ȫ����������
std::mutex countM;
int count = 0;

void FluidSystem::_comForce_And_advance(int i)
{
    std::unique_lock<std::mutex> lck(mtx);
    //cr.wait(lck);
    this->_computerForce(i);
    //ͬ���������ⲿ�ֽ������������̽���λ�Ƽ��㣻
    //�����������̼�����ٶ�ʱ��ȡ�˴������ݣ�
    countM.lock();
    if ((++count) != thread_max)
    {
        countM.unlock();
        cr.wait(lck);
    }
    else
    {
        countM.unlock();
        cr.notify_all();
    }
    this->_advance(i);
}

void FluidSystem::tick()
{
    count = 0;
    std::thread threads[thread_max];
    max = f_PointBuffer.getPointNumber();
    // ÿ֡ˢ������λ�ã�
    f_GridContainer.insertPoint(&f_PointBuffer);
    // ѹǿ���㣬��������ӱ�
    _computerPressure();
    // ���������
    //for (int i = 0; i < thread_max; i++)
    //    threads[i] = std::thread(&FluidSystem::_computerForce, this, i);
    //for (auto& t : threads)
    //    t.join();
    //_computerForce();
    // ���ٶȼ�����߽���ƣ�
    //for (int i = 0; i < thread_max; i++)
    //    threads[i] = std::thread(&FluidSystem::_advance, this, i);
    //for (auto& t : threads)
    //    t.join();
    //_advance();
    for (int i = 0; i < thread_max; i++)
        threads[i] = std::thread(&FluidSystem::_comForce_And_advance, this, i);
    //cr.notify_all();
    for (auto& t : threads)
        t.join();
}