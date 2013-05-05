/* Structures for Obj.c file */


#ifndef _OBJ_STRUCT_LOADER_H_
#define _OBJ_STRUCT_LOADER_H_


#include "Obj.h"

/*
**	To start with, only deal with the obj file as a list of points. Essentually,
**	if we are able to draw the points out they should roughly describe the mesh.
**	So, lets look for the 'v' flag that says that the 3 floats after it describes a
**	Vertex. Essentually then, we need to have a structure for a vertex.
**	The normal vector flag 'vn' could also do with a structure to represent it,
**	this will ultimately look very similar to the Vertex structure seeing as they have
**	similar data
*/
typedef struct
{
	float x,			/*	The x component of the vertex position (normal vector)	*/
		  y,			/*	The y component of the vertex position (normal vector)	*/
		  z;			/*	The z component of the vertex position (normal vector)	*/
} ObjVertex, ObjNormal;


/*
**	A Texturing co-ordinate usually has only two values, the u and the v. Make a third
**	struct for this fact.
*/
typedef struct
{
	float u,			/*	The u parametric texturing co-ordinate	*/
		  v;			/*	The v parametric texturing co-ordinate	*/
} ObjTexCoord;


/*
**	Each face is composed of a number of `corners`. At each `corner` there will be at a
**	minimum, one vertex coordinate, and possibly a normal vector and a texturing co-ordinate.
**	Seeing as quite often a specific vertex is used by a number of faces, rather than store the
**	vertices in the face structure itself, we can use an integer reference to the position of the
**	vertex in the array. Basically it allows us the ability of storing the data as a 4byte value
**	rather than the 12 needed for a vertex.
**
**	To summaraise,
**
**		Each face is comprised of a number of `corners`, in this case the number will be held in
**	the value "m_iVertexCount". Taking into account the worst case scenario, for each corner, we
**	could have a vertex, normal and a texturing co-ordinate (usual actually). So we may need to hold
**	3 * m_iVertexCount number of indices for our faces. It kindof makes sense to basically dynamically
**	allocate an array to hold the vertex indicies, an array for the normal indices and one for the uv
**	coords. These are m_aVertexIndices, m_aNormalIndices, and m_aTexCoordIndicies respectivley.
**
**	So our full face structure looks a bit like this :
*/
typedef struct
{
	unsigned int *m_aVertexIndices,			/*	array of indicies that reference the vertex array in the mesh	*/
				 *m_aNormalIndices,			/*	array of indicies that reference the normal array in the mesh	*/
				 *m_aTexCoordIndicies;		/*	array of indicies that reference the uv coordinate array in the mesh	*/
	unsigned int  m_iVertexCount;			/*	the number of vertices that make up this mesh, ie, 3 = triangle; 4 = quad etc */
} ObjFace;

/*
**	Each mesh is held as a structure with arrays of it's vertices, normals and texturing co-ordinates.
**	An Array of faces then references the arrays. There is also a pointer to the next node in the linked
**	list.
*/
typedef struct _ObjMesh
{
	ObjVertex		*m_aVertexArray;		/*	Array of vertices that make up this mesh	*/
	ObjNormal		*m_aNormalArray;		/*	Array of normals that make up this mesh		*/
	ObjTexCoord		*m_aTexCoordArray;		/*	Array of texturing co-ordinates that make up this mesh */
	ObjFace			*m_aFaces;				/*	Array of faces that make up this mesh */

	unsigned int	 m_iNumberOfVertices,	/*	The number of vertices in the m_aVertexArray array	*/
					 m_iNumberOfNormals,	/*	The number of normals in the m_aNormalArray array	*/
					 m_iNumberOfTexCoords,	/*	The number of uv's in the m_aTexCoordArray array	*/
					 m_iNumberOfFaces;		/*	The number of faces in the m_aFaces array			*/

	struct _ObjMesh *m_pNext;				/*	Each mesh will be stored internally as a node on a linked list */
	ObjFile			 m_iMeshID;				/*	the ID of the mesh	*/
	unsigned int	*m_aTypesOfFaces;		/*FIRST or SECOND or OTHER types*/
	ObjVertex	*m_aLights;			//array of lights - only 1 or 0 in current implementation	

	unsigned int	m_iNumberSphereDetalisation;		/*all our spaceship in the big sphere inside. m_iNumberSphereDetalisation^2 = NumberSpherePolygons  */
	float 		*m_aSpherePolygonRadiosity;	/*radiosity on polygon (number of polygons in the m_iNumberSpherePolygons */
	ObjVertex	*m_aSphereVertexArray;		/* for speedup opengl scene */
} ObjMesh;

#endif
