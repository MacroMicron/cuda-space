/* This header implement structures (vertex, face and mesh), which are used in MathLinear modules,
and interfaces to access data from it.*/

#include "UserSettings.h"



#ifndef _MATH_LINEAR_STRUCTURES_
#define _MATH_LINEAR_STRUCTURES_

#ifdef __cplusplus
extern "C" {
#endif

//it's types of faces (polygons): first, second and other
#define FIRST_VISION 1
#define SECOND_VISION 2
#define DOUBLE_VISION 3 //if the face is first and second vision
#define OTHER_VISION 0



/*			implement interfaces to structures:
	Warning! all this methods you use as is. There are no checks on errors.
*/



/* Vertex interface: */

//it is access to x, y, z as in array, so, pointerCalcVertex[0] == pointerCalcVertex->x, ...etc. Of course, q from {0, 1, 2}, no more
//real <= (CalcVertex*, unsigned char)
#define ___XYZOf(pointerCalcVertex, q)		( *((real *)(pointerCalcVertex) + (q)) )

//get x-coordinate from vertex
//real <= (CalcVertex*)
#define ___XOf(pointerCalcVertex)			( ((CalcVertex *)(pointerCalcVertex))->x )

//get y-coordinate from vertex
//real  <= (CalcVertex*)
#define ___YOf(pointerCalcVertex)			( ((CalcVertex *)(pointerCalcVertex))->y )

//get z-coordinate from vertex
//real  <= (CalcVertex*)
#define ___ZOf(pointerCalcVertex)			( ((CalcVertex *)(pointerCalcVertex))->z )



/* Face (Polygon) interface: */

//this method to simplify readability other methods. Don't use it. Return point index in the VertexArray.
// integer <= (CalcFace*, integer)
#define _x_PointIndexFrom(pointerCalcFace, n)	(( ((CalcFace*)(pointerCalcFace))->VertexIndices )[(n)])

//Get the pointer to the Vertex, n - number of vertex in the face. Of course , n from {0, .., face VertexCount}
// CalcVertex* <= (CalcFace*, integer)
#define ___PointFrom(pointerCalcFace, n)		(((CalcFace*)(pointerCalcFace))->VertexArray + _x_PointIndexFrom(pointerCalcFace, n))

//Get q-coordinate (or x, or y, or maybe z) from n-vertex in the face. q from {0,1,2}; n from {0,.., face VertexCount}
// real <= (CalcFace*, integer, unsigned char)
#define ___XYZPointFrom(pointerCalcFace, n, q)		___XYZOf( ___PointFrom(pointerCalcFace, n), q)

//Get x-coordinate from n-vertex in the face. n from {0,...face VertexCount}
// real <= (CalcFace*, integer)
#define ___XPointFrom(pointerCalcFace, n)			___XOf( ___PointFrom(pointerCalcFace, n) )

//Get y-coordinate from n-vertex in the face. n from {0,...face VertexCount}
// real <= (CalcFace*, integer)
#define ___YPointFrom(pointerCalcFace, n)			___YOf( ___PointFrom(pointerCalcFace, n) )

//Get z-coordinate from n-vertex in the face. n from {0,...face VertexCount}
// real <= (CalcFace*, integer)
#define ___ZPointFrom(pointerCalcFace, n)			___ZOf( ___PointFrom(pointerCalcFace, n) )

//Get plane coefficients from polygon(face)
// real* <= (CalcFace*)
#define ___PlaneFrom(pointerCalcFace)					( ((CalcFace*)(pointerCalcFace))->PlaneCoefficients )

//Get number of vertices in the face (polygon dimension)
// unsigned char <= (CalcFace*)
#define ___DimOf(pointerCalcFace)						( ((CalcFace*)(pointerCalcFace))->VertexCount )

//Change the order of points in the face (polygon). n1 and n2 from {0,..face VertexCount}
// void <= (integer, integer, CalcFace*)
#define ___ExchangeVertices(n1, n2, pointerCalcFace)	{																			\
						int temp_j43rj39 = ((CalcFace*)(pointerCalcFace))->VertexIndices[(n1)];										\
						((CalcFace*)(pointerCalcFace))->VertexIndices[(n1)] = ((CalcFace*)(pointerCalcFace))->VertexIndices[(n2)];	\
						((CalcFace*)(pointerCalcFace))->VertexIndices[(n2)] = temp_j43rj39;}




/*					structures implement:
*/


/* Vertex (Point) Structure */
typedef struct
{
	real x, y, z;
} CalcVertex;
/* important! x, y and z fields in the structure must be stored step by step in the memory (it's used in our interfaces access -
in the access to structure fields as in the array)
It's ok in C99 and higher (C99 [6.7.2.1], 13): "Within a structure object, the non-bit-field members and the units in
which bit-fields reside have addresses that increase in the order in which they are declared"
and (3.10/4): "an aggregate or union type that includes one of the aforementioned types among its members (including,
recursively, a member of a subaggregate or contained union)"
*/



/* Face Structure */
typedef struct
{
	integer		  VertexIndices[4];			/*	array of indicies that reference the vertex array by pointer VertexArray.
											we store index and vertex array because it is hack to save our memory:
											size of one point == 3 * size of real,
											minimum size of one face (if we stored points not indexes) == 3 * size of one point,
											but now we can only: minimum size of now one face == 3 * size of integer, that good!
											CalcFace only for 3- or 4-polygons
											*/
	real		   PlaneCoefficients[4];	/*	Array of plane coefficients (first, second and third - normal coordinate).
											We calculate this coefficients in the special procedure by the first, second and third
											points */
	unsigned char  VertexCount;				/*	the number of vertices that make up this mesh, ie, 3 = triangle; 4 = quad etc */

	CalcVertex	  *VertexArray;				/* It's a constant for all polygons (faces) in one mesh, which is equal a VertexArray
											in the mesh. Yes, it's a static field (in our mind) but not in real life,
											because C-language does not support static fields.
											It's very useful to has access from face to vertixes*/
} CalcFace;



/* Mesh structure */
//CalcMesh - is a type of all figure
typedef struct
{
	CalcVertex		*VertexArray;		   /*	Array of Vertices. Pointer is equal for all CalcVertex in the faces*/
	CalcFace		*Faces;				   /* not eaual in ObjMesh.m_aFaces, but simetimes similar */
	integer			 NumberOfVertices,	/*	The number of vertices in the m_aVertexArray array, equal ObjMesh.m_iNumberOfVertices*/
					 NumberOfFaces;		/*	The number of faces in the m_aFaces array, equal ObjMesh.m_iNumberOfFaces */

	unsigned char	*TypesOfFaces;		/*FIRST or SECOND or OTHER types*/
} CalcMesh;



#ifdef __cplusplus
}
#endif

#endif
