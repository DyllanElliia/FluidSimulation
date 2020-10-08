#pragma once

#include <vector>
#include <string>
#include <map>
using std::vector;
using std::string;
using std::map;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class rxFace
{
public:
    vector<int> vertIdx;        // 顶点索引
    string materialName;        // 材质：用于渲染；
    vector<glm::vec2> texcoords;// 像素坐标
    int attribute;              // 属性

public:
    rxFace() : attribute(0) {}

    inline int& operator[](int i) { return vertIdx[i]; }
    inline int  operator[](int i) const { return vertIdx[i]; }

    // 按函数进行顶点访问；
    inline int& at(int i) { return vertIdx.at(i); }
    inline int  at(int i) const { return vertIdx.at(i); }

    // 重新设置顶点数量；
    // 更改多边形顶点的数量；
    void resize(int size)
    {
        vertIdx.resize(size);
    }

    // 返回多边形顶点的数量；
    int size(void) const
    {
        return (int)vertIdx.size();
    }

    // 初始化，清空；
    void clear(void)
    {
        vertIdx.clear();
        materialName = "";
        texcoords.clear();
    }
};

// 点的信息；
struct RxVertexID
{
    unsigned int newID; //索引
    double x, y, z;     //位置
};

// 映射表：顶点索引序列，顶点；
typedef std::map<unsigned int, RxVertexID> ID2VertexID;

// 三角面的信息；
struct RxTriangle
{
    //存有每个点的索引；
    unsigned int pointID[3];
};

typedef std::vector<RxTriangle> RxTriangleVector;

// 标量场；
struct RxScalarField
{
    //x,y,z的网格数量；
    unsigned int i_Num[3];
    //每个网格宽度；
    glm::vec3 g_Width;
    //最小点坐标；
    glm::vec3 g_Min;
};

class rxMCMesh
{
public:
    rxMCMesh();
    ~rxMCMesh();


    /* 从样本生成三角形网格：
     * [in] 字段样本量；
     * [in] min_p 网格的最小坐标；
     * [in] h 网格的宽度；
     * [in] n [3]网格编号（x，y，z）；
     * [in] 阈值（隐含函数值网格化此值）；
     * //[in] 方法网格生成方法（“mc”，“rmt”，“bloomenthal”）；
     * [out] vrts 顶点坐标；
     * [out] nrms 顶点正常；
     * [out] tris mesh；
     * return true Mesh代成功；
     * return false网格生成失败；
     */
    bool CreateMeshV(   float* field, glm::vec3 min_p, double h, int n[3], float threshold,
                        vector<glm::vec3>& vrts, vector<glm::vec3>& nrms, vector<rxFace>& face);

    //从样本量生成等值面网格；
    void GenerateSurfaceV(  const RxScalarField sf, float* field, float threshold,
                            vector<glm::vec3>& vrts, vector<glm::vec3>& nrms, vector<int>& tris);

    //等值面创建成功则返回true；
    bool IsSurfaceValid() const { return m_bValidSurface; }

    //删除表面；
    void DeleteSurface();

    //用于网格划分的网格大小（在未创建网格的情况下，返回值为-1）；
    //int GetVolumeLengths(double& fVolLengthX, double& fVolLengthY, double& fVolLengthZ);

    //有关创建的网格的信息
    unsigned int GetNumVertices(void) const { return m_nVertices; }
    unsigned int GetNumTriangles(void) const { return m_nTriangles; }
    unsigned int GetNumNormals(void) const { return m_nNormals; }

private:
    unsigned int m_nVertices;   //点的个数
    unsigned int m_nNormals;    //法线个数
    unsigned int m_nTriangles;  //三角面个数

    ID2VertexID m_i2pt3idVertices;      //形成等值面的顶点列表(以边索引为key，等值面的点为value)
    RxTriangleVector m_trivecTriangles; //形成三角形的顶点列表
    RxScalarField m_Grid;               //标量场信息

    const float* m_ptScalarField;   //保存标量值的样本量

    float m_tIsoLevel;      //阈值
    bool m_bValidSurface;   //表面是否生成成功

    // 边id
    unsigned int GetEdgeID(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo);

    // 顶点ID
    unsigned int GetVertexID(unsigned int nX, unsigned int nY, unsigned int nZ);

    // 计算边缘上的等值面
    RxVertexID CalculateIntersection(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo);

    // 通过网格边缘两端的隐式函数值的线性插值计算等值点
    RxVertexID Interpolate(double fX1, double fY1, double fZ1, double fX2, double fY2, double fZ2, float tVal1, float tVal2) { return  RxVertexID(); }

    /* 以输出形式存储顶点和网格几何信息
     * [out] vrts顶点坐标
     * [out] nvrts顶点数
     * [out] tris三角形多边形几何信息
     * [out] ntris三角形多边形的数量
     */
    void RenameVerticesAndTriangles(vector<glm::vec3>& vrts, unsigned int& nvrts, vector<int>& tris, unsigned int& ntris);

    // 顶点法线计算
    void CalculateNormals(const vector<glm::vec3>& vrts, unsigned int nvrts, const vector<int>& tris, unsigned int ntris, vector<glm::vec3>& nrms, unsigned int& nnrms);

};

