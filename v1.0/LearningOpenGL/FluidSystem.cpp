#include "FluidSystem.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

FluidSystem::FluidSystem()
{
    //初始化参数：水；
    f_unitScale = 0.004f;   //尺寸单位
    f_viscosity = 1.0f;     //粘度
    f_restDensity = 1000.f; //密度
    f_PointMass = 0.0002f;  //粒子质量
    f_gasConstantK = 1.0f;  //理想气体方程常量
    f_smoothRadius = 0.01f; //光滑核半径
    f_PointDistance = 0.0;  //粒子半径
    //初始化网格尺寸；
    f_rexSize[0] = 0;
    f_rexSize[1] = 0;
    f_rexSize[2] = 0;
    //设置限制；
    f_boundaryStiffness = 10000.f;  //边界刚性
    f_boundaryDampening = 256;      //边界阻尼
    f_speedLimiting = 200;          //速度限制
    //设置光滑核函数参数；
    //Poly6 Kernel
    f_kernelPoly6 = 315.0f / (64.0f * 3.141592f * pow(f_smoothRadius, 9));
    //Spiky Kernel
    f_kernelSpiky = -45.0f / (3.141592f * pow(f_smoothRadius, 6));
    //Viscosity Kernel
    f_kernelViscosity = 45.0f / (3.141592f * pow(f_smoothRadius, 6));
}

FluidSystem::~FluidSystem()
{}

//构造流体中的点
void FluidSystem::_addFluidVolume(const Box& fluidBox, float spacing)
{
    //按照步长为spacing进行空间遍历；
    for (float z = fluidBox.max.z; z >= fluidBox.min.z; z -= spacing)
        for (float y = fluidBox.min.y; y <= fluidBox.max.y; y += spacing)
            for (float x = fluidBox.min.x; x <= fluidBox.max.x; x += spacing)
            {
                //添加点；
                Point* p = f_PointBuffer.addPointReuse();
                //初始化点坐标；
                p->pos = glm::vec3(x, y, z);
            }
}

void FluidSystem::_addFluidVolume(const Box& fluidBox, float spacing, glm::vec3 velocity)
{
    //按照步长为spacing进行空间遍历；
    for (float z = fluidBox.max.z; z >= fluidBox.min.z; z -= spacing)
        for (float y = fluidBox.min.y; y <= fluidBox.max.y; y += spacing)
            for (float x = fluidBox.min.x; x <= fluidBox.max.x; x += spacing)
            {
                //添加点；
                Point* p = f_PointBuffer.addPointReuse();
                //初始化点坐标；
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
    //初始化缓冲，构造缓冲区；
    f_PointBuffer.resetPointBuffer(maxPointCounts);
    //存储包含流体的流体网格；
    f_sphWallBox = wallBox;
    //构造包含流体的流体网格空间；
    //设置网格尺寸(2r)；
    f_GridContainer.init(wallBox, f_unitScale, f_smoothRadius * 2.0, 1.0, f_rexSize);
    //写入重力；
    f_gravityDir = gravity;
    //计算粒子间距
    //(质量/密度)^(1/3)=粒子间距；
    f_PointDistance = pow(f_PointMass / f_restDensity, 1.0 / 3.0);
    //在initFluidBox空间中构造初始流体；
    _addFluidVolume(initFluidBox, f_PointDistance / f_unitScale);
    //整个粒子缓冲，用于快速绑定VBO；
    posData = std::vector<GLfloat>(3 * f_PointBuffer.getPointNumber(), 0);
}

void FluidSystem::_computerPressure()
{
    // 光滑核函数半径的平方，使用次数多，因此提取出来；
    float smoothR2 = f_smoothRadius * f_smoothRadius;
    // 重置邻接表；
    // 全部初始化；
    f_NeighborTable.reset(f_PointBuffer.getPointNumber());
    // 遍历所有点，计算每个点的密度；
    for (unsigned int i = 0, limit = f_PointBuffer.getPointNumber(); i < limit; i++)
    {
        // 获得索引值为i的粒子；
        Point* pi = f_PointBuffer.getPoint(i);
        float sum = 0.0;
        // 准备，存储当前粒子的索引，然后初始化邻接点数量；
        f_NeighborTable.pointPrepare(i);
        // 存储索引最近8个单元网格的值的数组；
        int gridCell[8];
        // 获得可能pi的光滑核函数球的8个单元网格；
        f_GridContainer.findCells_8(pi->pos, f_smoothRadius / f_unitScale, gridCell);
        // 遍历8个网格中的所有粒子；
        for (int cell = 0; cell < 8; cell++)
        {
            // cell索引值为-1，即当前元素不索引任何网格；
            if (gridCell[cell] == -1)
                continue;
            // 获取cell索引的单位网格内存储的第一个粒子的索引；
            int pndx = f_GridContainer.getGridData(gridCell[cell]);
            // 按照索引遍历哈希链表；
            // 直到-1，退出循环；
            while (pndx != -1) {
                // 获得索引为pndx的粒子；
                Point* pj = f_PointBuffer.getPoint(pndx);
                // 若相等，即是当前点；
                if (pj == pi)
                    // 对于自身来说，ri-rj=0，即可直接跳过该步，减少算力的消耗；
                    sum += pow(smoothR2, 3.0);
                else
                {
                    // 计算实际的向量；
                    glm::vec3 pi_pj = (pi->pos - pj->pos) * f_unitScale;
                    float r2 = pi_pj.x * pi_pj.x + pi_pj.y * pi_pj.y + pi_pj.z * pi_pj.z;
                    // 判断该粒子是否被光滑核函数球包含，是，则累加该粒子的值；
                    if (smoothR2 > r2)
                    {
                        float h2_r2 = smoothR2 - r2;
                        sum += pow(h2_r2, 3.0);
                        // 该点的的邻接表满了，无法插入更多节点；
                        if (!f_NeighborTable.pointAddNeighbor(pndx, glm::sqrt(r2)))
                            // 跳出循环；
                            goto FINALL;
                    }
                }
                //获得下一个粒子的索引值；
                pndx = pj->next;
            }
        }
    FINALL:
        //将当前邻接表写入邻接表缓冲中；
        f_NeighborTable.pointCommit();
        //完成计算；
        pi->density = f_kernelPoly6 * f_PointMass * sum;
        //计算压强：
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
    //遍历所有点，对每一个点的粘度进行计算；
    //for (unsigned int i = 0, limit = f_PointBuffer.getPointNumber(); i < limit; i++)
    for (unsigned int i = begin; i < end; i++)
    {
        //获取当前索引值i对应的粒子；
        Point* pi = f_PointBuffer.getPoint(i);
        //初始化粘度；
        //glm::vec3 accelSum = glm::vec3(0.0);
        //加速度；
        glm::vec3 pressureS = glm::vec3(0.0);
        glm::vec3 viscosityS = glm::vec3(0.0);
        //获取之前存储的领接表中在该粒子的光滑核函数半径的粒子个数；
        int neighborCounts = f_NeighborTable.getNeighborCounts(i);
        //遍历光滑核函数球内的所有粒子，求得粘度和；
        for (unsigned int j = 0; j < neighborCounts; j++)
        {
            //邻接表粒子索引；
            unsigned short neighborIndex;
            
            float distanceR;
            //输入第i个索引，将第i索引对应的邻接表中第j个中存储的粒子索引和它到中心的距离返回给neighborIndex和distanceR；
            f_NeighborTable.getNeighborInfo(i, j, neighborIndex, distanceR);
            //获得索引值neighborIndex对应的粒子；
            Point* pj = f_PointBuffer.getPoint(neighborIndex);
            
            glm::vec3 ri_rj = (pi->pos - pj->pos) * f_unitScale;
            
            float h_r = f_smoothRadius - distanceR;
            //仅被使用一次，因此写成宏；
            //压力计算公式；
            #define pterm (h_r * h_r * (pi->pressure + pj->pressure) / (pj->density))
            pressureS += ri_rj * pterm / distanceR;
            //仅被使用一次，因此写成宏；
            //粘度计算公式；
            #define vterm (h_r / (pj->density))
            viscosityS += (pj->velocityEval - pi->velocityEval) * vterm;
        }
        //pressure中提取出来的公有项：
        #define pressureH (-f_kernelSpiky * f_PointMass / (2.0f * pi->density))
        //viscosity中提取出来的公有项：
        #define viscosityH (f_kernelViscosity * f_viscosity * f_PointMass / (pi->density))
        //计算最后的加速度：
        pi->accel = f_gravityDir + pressureH * pressureS + viscosityH * viscosityS;
    }
}

//std::mutex posDataMU;

void FluidSystem::_advance(int i) {
    int begin = int(float(i) / float(thread_max) * max);
    int end = int(float(i+1) / float(thread_max) * max);
    //设置时间间隔；
    float deltaTime = 0.001;
    //最大速度的平方；
    float speedLimit2 = f_speedLimiting * f_speedLimiting;
    //若模拟流体的粒子的数量不变的话可以不用清空；
    //若模拟烟雾、海浪和火焰等粒子数量会发生变化的情况，则必须清空；
    //posData.clear();
    //遍历点，逐一处理；
    //for (unsigned int i = 0, limit = f_PointBuffer.getPointNumber(); i < limit; i++)
    for (unsigned int i = begin; i < end; i++)
    {
        //逐一获得点；
        Point* p = f_PointBuffer.getPoint(i);
        //获取p的当前加速度；
        glm::vec3 accel = p->accel;
        //取速度矢量的模的平方；
        float accel2 = accel.x * accel.x + accel.y * accel.y + accel.z * accel.z;
        //将速度矢量的模的平方与最大速度的平方作比较；
        //速度的模不能大于最大速度限制；
        if (accel2 > speedLimit2)
            //若超过，则将原速度矢量乘一个比例变为模为最大速度限制的速度矢量；
            //比例为\frac{最大速度限制}{速度的模}；
            accel *= f_speedLimiting / glm::sqrt(accel2);

        float diff;
        //边界情况
        //Z方向边界
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

        //X方向边界
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
        
        //Y方向边界
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
        // 位置计算；
        glm::vec3 vnext = p->velocity + accel * deltaTime;
        p->velocityEval = (p->velocity + vnext) * 0.5f;
        p->velocity = vnext;
        p->pos += vnext * deltaTime / f_unitScale;
        //弹入位置数据，用于快速绑定VBO；
        posData[3 * i] = p->pos.x;
        posData[3 * i + 1] = p->pos.y;
        posData[3 * i + 2] = p->pos.z;
    }
}

std::mutex mtx;             // 全局互斥锁
std::condition_variable cr; // 全局条件变量
std::mutex countM;
int count = 0;

void FluidSystem::_comForce_And_advance(int i)
{
    std::unique_lock<std::mutex> lck(mtx);
    //cr.wait(lck);
    this->_computerForce(i);
    //同步锁，避免部分进程先其他进程进入位移计算；
    //导致其他进程计算加速度时读取了错误数据；
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
    // 每帧刷新粒子位置；
    f_GridContainer.insertPoint(&f_PointBuffer);
    // 压强计算，并构建领接表；
    _computerPressure();
    // 计算合力；
    //for (int i = 0; i < thread_max; i++)
    //    threads[i] = std::thread(&FluidSystem::_computerForce, this, i);
    //for (auto& t : threads)
    //    t.join();
    //_computerForce();
    // 加速度计算与边界控制；
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