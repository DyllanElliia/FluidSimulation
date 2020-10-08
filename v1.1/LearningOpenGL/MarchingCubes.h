#pragma once

#include "TriangleTable.h"
#include "PointBuffer.h"
#include "NeighborTable.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef x
#define x 0
#endif // !x
#ifndef y
#define y 1
#endif // !y
#ifndef z
#define z 2
#endif // !z

class MarchingCubes
{
private:
	float offset;	// 存储当前计算的立方体的偏移量；
	float scale[3]; // 计算当前立方体的x y z的缩放值；



};

