#include "UserSettings.h"
#include "MathLinearStructures.cuh"
#include "ObjStruct.h"

#ifndef _CUDA_RUNTIME_
#define _CUDA_RUNTIME_
#include "cuda_runtime.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


CalcMesh* CreateCalcMesh(ObjMesh *objMesh);
void CopyResults(CalcMesh *calcMesh, ObjMesh *objMesh);
void DeleteCalcMesh(CalcMesh *calcMesh);


__host__ __device__ void PlaneDefine(CalcFace *face);
__host__ __device__ void CorrectQuadrilateralSimplisity(CalcFace *face);
__host__ __device__ Bool PointInPolygon(CalcVertex *point, CalcFace *face);
__host__ __device__ Bool IsSegmentIntersectPolygon(CalcVertex *point1, CalcVertex *point2, CalcFace *face);

__host__ __device__ integer IsSegmentIntersectModel(CalcVertex *point1, CalcVertex *point2, CalcMesh *mesh);
void ToCountFirstFaces(CalcVertex *light, CalcMesh *mesh);
void ToCountSecondAndDoubleFaces(CalcMesh *mesh);
void GPU_example(CalcMesh *mesh);
//what the fuck with visual studio linker?


#ifdef __cplusplus
}
#endif
