#include "McRxFace.h"
#include "TriangleTable.h"

rxMCMesh::rxMCMesh()
{
    //初始化；
    m_Grid.g_Min[0] = 0;
    m_Grid.g_Min[1] = 0;
    m_Grid.g_Min[2] = 0;
    m_Grid.g_Width[0] = 0;
    m_Grid.g_Width[1] = 0;
    m_Grid.g_Width[2] = 0;
    m_Grid.i_Num[0] = 0;
    m_Grid.i_Num[1] = 0;
    m_Grid.i_Num[2] = 0;

    m_nTriangles = 0;
    m_nNormals = 0;
    m_nVertices = 0;

    m_ptScalarField = nullptr;
    //m_fpScalarFunc = 0;
    m_tIsoLevel = 0;
    m_bValidSurface = false;
}

rxMCMesh::~rxMCMesh()
{
    DeleteSurface();
}

//删除已创建的等值面；
void rxMCMesh::DeleteSurface()
{
    //清空；
    m_Grid.g_Width[0] = 0;
    m_Grid.g_Width[1] = 0;
    m_Grid.g_Width[2] = 0;
    m_Grid.i_Num[0] = 0;
    m_Grid.i_Num[1] = 0;
    m_Grid.i_Num[2] = 0;

    m_nTriangles = 0;
    m_nNormals = 0;
    m_nVertices = 0;

    m_ptScalarField = nullptr;
    m_tIsoLevel = 0;
    m_bValidSurface = false;
}

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
bool rxMCMesh::CreateMeshV(float* field, glm::vec3 min_p, double h, int* n, float threshold, vector<glm::vec3>& vrts, vector<glm::vec3>& nrms, vector<rxFace>& face)
{
    //字段中没有样本，创建失败；
    if (field == nullptr)
        return false;

    RxScalarField sf;
    for (int i = 0; i < 3; i++)
    {
        //x、y、z的网格数量；
        sf.i_Num[i] = n[i];
        //每个网格的宽度；
        sf.g_Width[i] = h;
        //网格的空间位置（最小坐标）
        sf.g_Min[i] = min_p[i];
    }

    vector<int> tris;
    GenerateSurfaceV(sf, field, threshold, vrts, nrms, tris);

    if (IsSurfaceValid())
    {
        int nv = (int)GetNumVertices();
        int nm = (int)GetNumTriangles();
        int nn = (int)GetNumNormals();


        for (int i = 0; i < nn; ++i)
            nrms[i] *= -1.0;

        face.resize(nm);
        for (int i = 0; i < nm; ++i)
        {
            face[i].vertIdx.resize(3);
            for (int j = 0; j < 3; ++j)
                face[i][j] = tris[3 * i + (2 - j)];
        }
        return true;
    }
    return false;
}

/* 从样本量生成等值面网格；
 * [in] sf 标量场；
 * [in] field 字段样本量；
 * [in] 阈值（隐含函数值网格化此值）；
 * [out] vrts 顶点坐标；
 * [out] nrms 顶点正常；
 * [out] tris mesh；
 */
void rxMCMesh::GenerateSurfaceV(const RxScalarField sf, float* field, float threshold, vector<glm::vec3>& vrts, vector<glm::vec3>& nrms, vector<int>& tris)
{
    //是否已生成等值面，若生成，则删除它；
    if (m_bValidSurface)
        DeleteSurface();

    //保存阈值；
    m_tIsoLevel = threshold;
    //初始化当前标量场；
    m_Grid.i_Num[0] = sf.i_Num[0];
    m_Grid.i_Num[1] = sf.i_Num[1];
    m_Grid.i_Num[2] = sf.i_Num[2];
    m_Grid.g_Width = sf.g_Width;
    m_Grid.g_Min = sf.g_Min;
    //保存标量场样本量；
    m_ptScalarField = field;

    //构造对x的网格切片；
    unsigned int slice0 = (m_Grid.i_Num[0] + 1);
    //构造对y的网格切片；
    unsigned int slice1 = slice0 * (m_Grid.i_Num[1] + 1);

    //遍历网格空间；
    //生成等値面；
    for (unsigned int z = 0; z < m_Grid.i_Num[2]; z++)
        for (unsigned int y = 0; y < m_Grid.i_Num[1]; y++)
            for (unsigned int x = 0; x < m_Grid.i_Num[0]; x++)
            {
                //计算网格中的顶点放置信息表参考索引；
                unsigned int tableIndex = 0;
                unsigned int z_x_slice1 = z * slice1;
                unsigned int Zadd1_x_slice1 = (z + 1) * slice1;
                unsigned int y_x_slice0 = y * slice0;
                unsigned int Yadd1_x_slice0 = (y + 1) * slice0;

                //索引至标量场中的数值，判断是否符合顶点；
                if (m_ptScalarField[z_x_slice1 + y_x_slice0 + x] < m_tIsoLevel)
                    tableIndex |= 1;//0000 0001
                if (m_ptScalarField[z_x_slice1 + Yadd1_x_slice0 + x] < m_tIsoLevel)
                    tableIndex |= 2;//0000 0010
                if (m_ptScalarField[z_x_slice1 + Yadd1_x_slice0 + (x + 1)] < m_tIsoLevel)
                    tableIndex |= 4;//0000 0100
                if (m_ptScalarField[z_x_slice1 + y_x_slice0 + (x + 1)] < m_tIsoLevel)
                    tableIndex |= 8;//0000 1000
                if (m_ptScalarField[Zadd1_x_slice1 + y_x_slice0 + x] < m_tIsoLevel)
                    tableIndex |= 16;//0001 0000
                if (m_ptScalarField[Zadd1_x_slice1 + Yadd1_x_slice0 + x] < m_tIsoLevel)
                    tableIndex |= 32;//0010 0000
                if (m_ptScalarField[Zadd1_x_slice1 + Yadd1_x_slice0 + (x + 1)] < m_tIsoLevel)
                    tableIndex |= 64;//0100 0000
                if (m_ptScalarField[Zadd1_x_slice1 + y_x_slice0 + (x + 1)] < m_tIsoLevel)
                    tableIndex |= 128;//1000 0000

                int g_edgeTable_tableIndex = g_edgeTable[tableIndex];
                if (g_edgeTable_tableIndex != 0)
                {
                    // 计算边上的顶点
                    if (g_edgeTable_tableIndex & 8)
                    {
                        RxVertexID pt = CalculateIntersection(x, y, z, 3);
                        unsigned int id = GetEdgeID(x, y, z, 3);
                        m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                    }

                    if (g_edgeTable_tableIndex & 1)
                    {
                        RxVertexID pt = CalculateIntersection(x, y, z, 0);
                        unsigned int id = GetEdgeID(x, y, z, 0);
                        m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                    }

                    if (g_edgeTable_tableIndex & 256)
                    {
                        RxVertexID pt = CalculateIntersection(x, y, z, 8);
                        unsigned int id = GetEdgeID(x, y, z, 8);
                        m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                    }

                    if (x == m_Grid.i_Num[0] - 1)
                    {
                        if (g_edgeTable_tableIndex & 4)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 2);
                            unsigned int id = GetEdgeID(x, y, z, 2);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }

                        if (g_edgeTable_tableIndex & 2048)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 11);
                            unsigned int id = GetEdgeID(x, y, z, 11);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }
                    }
                    if (y == m_Grid.i_Num[1] - 1)
                    {
                        if (g_edgeTable_tableIndex & 2)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 1);
                            unsigned int id = GetEdgeID(x, y, z, 1);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }

                        if (g_edgeTable_tableIndex & 512)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 9);
                            unsigned int id = GetEdgeID(x, y, z, 9);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }
                    }
                    if (z == m_Grid.i_Num[2] - 1)
                    {
                        if (g_edgeTable_tableIndex & 16)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 4);
                            unsigned int id = GetEdgeID(x, y, z, 4);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }

                        if (g_edgeTable_tableIndex & 128)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 7);
                            unsigned int id = GetEdgeID(x, y, z, 7);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }
                    }

                    if ((x == m_Grid.i_Num[0] - 1) && (y == m_Grid.i_Num[1] - 1))
                        if (g_edgeTable_tableIndex & 1024)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 10);
                            unsigned int id = GetEdgeID(x, y, z, 10);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }

                    if ((x == m_Grid.i_Num[0] - 1) && (z == m_Grid.i_Num[2] - 1))
                        if (g_edgeTable_tableIndex & 64)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 6);
                            unsigned int id = GetEdgeID(x, y, z, 6);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }

                    if ((y == m_Grid.i_Num[1] - 1) && (z == m_Grid.i_Num[2] - 1))
                        if (g_edgeTable_tableIndex & 32)
                        {
                            RxVertexID pt = CalculateIntersection(x, y, z, 5);
                            unsigned int id = GetEdgeID(x, y, z, 5);
                            m_i2pt3idVertices.insert(ID2VertexID::value_type(id, pt));
                        }

                    // 多边形生成
                    for (unsigned int i = 0; g_triTable[tableIndex][i] != 255; i += 3)
                    {
                        RxTriangle triangle;
                        unsigned int pointID0, pointID1, pointID2;
                        pointID0 = GetEdgeID(x, y, z, g_triTable[tableIndex][i]);
                        pointID1 = GetEdgeID(x, y, z, g_triTable[tableIndex][i + 1]);
                        pointID2 = GetEdgeID(x, y, z, g_triTable[tableIndex][i + 2]);
                        triangle.pointID[0] = pointID0;
                        triangle.pointID[1] = pointID1;
                        triangle.pointID[2] = pointID2;
                        m_trivecTriangles.push_back(triangle);
                    }
                }
            }        

    RenameVerticesAndTriangles(vrts, m_nVertices, tris, m_nTriangles);
    CalculateNormals(vrts, m_nVertices, tris, m_nTriangles, nrms, m_nNormals);

    m_bValidSurface = true;
}



/*
 * 获得边缘ID
 * [in] nX，nY，nZ网格位置
 * [in] nEdgeNo边数
 * return 边缘ID
 */
unsigned int rxMCMesh::GetEdgeID(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo)
{
    switch (nEdgeNo)
    {
    case 0:
        return GetVertexID(nX, nY, nZ) + 1;
    case 1:
        return GetVertexID(nX, nY + 1, nZ);
    case 2:
        return GetVertexID(nX + 1, nY, nZ) + 1;
    case 3:
        return GetVertexID(nX, nY, nZ);
    case 4:
        return GetVertexID(nX, nY, nZ + 1) + 1;
    case 5:
        return GetVertexID(nX, nY + 1, nZ + 1);
    case 6:
        return GetVertexID(nX + 1, nY, nZ + 1) + 1;
    case 7:
        return GetVertexID(nX, nY, nZ + 1);
    case 8:
        return GetVertexID(nX, nY, nZ) + 2;
    case 9:
        return GetVertexID(nX, nY + 1, nZ) + 2;
    case 10:
        return GetVertexID(nX + 1, nY + 1, nZ) + 2;
    case 11:
        return GetVertexID(nX + 1, nY, nZ) + 2;
    default:
        // Invalid edge no.
        return -1;
    }
}


/* 获取顶点ID
 * [in] nX，nY，nZ网格位置
 * return 顶点ID
 */
unsigned int rxMCMesh::GetVertexID(unsigned int nX, unsigned int nY, unsigned int nZ)
{
    return 3 * (nZ * (m_Grid.i_Num[1] + 1) * (m_Grid.i_Num[0] + 1) + nY * (m_Grid.i_Num[0] + 1) + nX);
}


/* 通过插值计算边缘上的等值点（从样本量）
 * [in] nX，nY，nZ网格位置
 * [in] nEdgeNo边数
 * return 网格顶点信息
 */
RxVertexID rxMCMesh::CalculateIntersection(unsigned int nX, unsigned int nY, unsigned int nZ, unsigned int nEdgeNo)
{
    double x1, y1, z1, x2, y2, z2;
    unsigned int v1x = nX, v1y = nY, v1z = nZ;
    unsigned int v2x = nX, v2y = nY, v2z = nZ;

    switch (nEdgeNo)
    {
    case 0:
        v2y += 1;
        break;
    case 1:
        v1y += 1;
        v2x += 1;
        v2y += 1;
        break;
    case 2:
        v1x += 1;
        v1y += 1;
        v2x += 1;
        break;
    case 3:
        v1x += 1;
        break;
    case 4:
        v1z += 1;
        v2y += 1;
        v2z += 1;
        break;
    case 5:
        v1y += 1;
        v1z += 1;
        v2x += 1;
        v2y += 1;
        v2z += 1;
        break;
    case 6:
        v1x += 1;
        v1y += 1;
        v1z += 1;
        v2x += 1;
        v2z += 1;
        break;
    case 7:
        v1x += 1;
        v1z += 1;
        v2z += 1;
        break;
    case 8:
        v2z += 1;
        break;
    case 9:
        v1y += 1;
        v2y += 1;
        v2z += 1;
        break;
    case 10:
        v1x += 1;
        v1y += 1;
        v2x += 1;
        v2y += 1;
        v2z += 1;
        break;
    case 11:
        v1x += 1;
        v2x += 1;
        v2z += 1;
        break;
    }

    // 获取边的两点坐标
    x1 = m_Grid.g_Min[0] + v1x * m_Grid.g_Width[0];
    y1 = m_Grid.g_Min[1] + v1y * m_Grid.g_Width[1];
    z1 = m_Grid.g_Min[2] + v1z * m_Grid.g_Width[2];
    x2 = m_Grid.g_Min[0] + v2x * m_Grid.g_Width[0];
    y2 = m_Grid.g_Min[1] + v2y * m_Grid.g_Width[1];
    z2 = m_Grid.g_Min[2] + v2z * m_Grid.g_Width[2];

    unsigned int slice0 = (m_Grid.i_Num[0] + 1);
    unsigned int slice1 = slice0 * (m_Grid.i_Num[1] + 1);
    float val1 = m_ptScalarField[v1z * slice1 + v1y * slice0 + v1x];
    float val2 = m_ptScalarField[v2z * slice1 + v2y * slice0 + v2x];
    RxVertexID intersection = { 1,1,1,1 };//= Interpolate(x1, y1, z1, x2, y2, z2, val1, val2);

    return intersection;
}

/* 以输出形式存储顶点和网格几何信息
 * [out] vrts顶点坐标
 * [out] nvrts顶点数
 * [out] tris三角形多边形几何信息
 * [out] ntris三角形多边形的数量
 */
void rxMCMesh::RenameVerticesAndTriangles(vector<glm::vec3>& vrts, unsigned int& nvrts, vector<int>& tris, unsigned int& ntris)
{
    unsigned int nextID = 0;
    ID2VertexID::iterator mapIterator = m_i2pt3idVertices.begin();
    RxTriangleVector::iterator vecIterator = m_trivecTriangles.begin();

    // 刷新点
    while (mapIterator != m_i2pt3idVertices.end())
    {
        (*mapIterator).second.newID = nextID;
        nextID++;
        mapIterator++;
    }

    // 刷新三角面.
    while (vecIterator != m_trivecTriangles.end())
    {
        for (unsigned int i = 0; i < 3; i++)
        {
            unsigned int newID = m_i2pt3idVertices[(*vecIterator).pointID[i]].newID;
            (*vecIterator).pointID[i] = newID;
        }
        vecIterator++;
    }

    // 将所有顶点和三角形复制到两个数组中，以便可以有效地访问它们。
    // 复制点
    mapIterator = m_i2pt3idVertices.begin();
    nvrts = (int)m_i2pt3idVertices.size();
    vrts.resize(nvrts);
    for (unsigned int i = 0; i < nvrts; i++, mapIterator++)
    {
        vrts[i][0] = (*mapIterator).second.x;
        vrts[i][1] = (*mapIterator).second.y;
        vrts[i][2] = (*mapIterator).second.z;
    }
    // 复制制作三角形的顶点索引。
    vecIterator = m_trivecTriangles.begin();
    ntris = (int)m_trivecTriangles.size();
    tris.resize(ntris * 3);
    for (unsigned int i = 0; i < ntris; i++, vecIterator++)
    {
        tris[3 * i + 0] = (*vecIterator).pointID[0];
        tris[3 * i + 1] = (*vecIterator).pointID[1];
        tris[3 * i + 2] = (*vecIterator).pointID[2];
    }

    //释放空间
    m_i2pt3idVertices.clear();
    m_trivecTriangles.clear();
}

/* 顶点法线计算
 * [in] vrts顶点坐标
 * [in] nvrts顶点数
 * [in] tris三角形多边形几何信息
 * [in] ntris三角形多边形的数量
 * [out] nrms法线
 * [out] nnrms法线数（=顶点数）
 */
void rxMCMesh::CalculateNormals(const vector<glm::vec3>& vrts, unsigned int nvrts, const vector<int>& tris, unsigned int ntris, vector<glm::vec3>& nrms, unsigned int& nnrms)
{
    nnrms = nvrts;
    nrms.resize(nnrms);

    // 初始化法线.
    for (unsigned int i = 0; i < nnrms; i++)
    {
        nrms[i][0] = 0;
        nrms[i][1] = 0;
        nrms[i][2] = 0;
    }

    // 计算法线.
    for (unsigned int i = 0; i < ntris; i++)
    {
        glm::vec3 vec1, vec2, normal;
        unsigned int id0, id1, id2;
        id0 = tris[3 * i + 0];
        id1 = tris[3 * i + 1];
        id2 = tris[3 * i + 2];

        vec1 = vrts[id1] - vrts[id0];
        vec2 = vrts[id2] - vrts[id0];
        normal = cross(vec1, vec2);

        nrms[id0] += normal;
        nrms[id1] += normal;
        nrms[id2] += normal;
    }

    // 单位化法线.
    for (unsigned int i = 0; i < nnrms; i++)
        normalize(nrms[i]);
}