//Obj - is a computing model.
//Warning: Faces Counting (in __) only for one obj and onó light in the list!

//	Need the windows headers to be included for openGL when coding on WIN32
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "ObjStruct.h"
#include "Obj.h"




//macros for change color which depends of type of the color (first, second, double or other)
//output: the flaot[3] vector (StandartColor, FirstVisionColor, SecondVisionColor, DoubleVisionColor)
//input: {0, 1, 2, 3}
#define CHOOSE_FACE_COLOR(Type) ((Type) ? ( (Type)==1 ? FirstVisionColor : ((Type) == 2 ? SecondVisionColor : DoubleVisionColor) ) : StandartColor)



/* All structures and comments about they you can read in the ObjStruct.h file */


/*
**	The global head of the linked list of meshes. This is a linked list because it is possible that you will be
**	loading and deleting meshes during the course of the programs execution.
*/
ObjMesh *g_LinkedListHead = NULL;

/*
**	This is used to generate a unique ID for each Obj File
*/
unsigned int g_ObjIdGenerator = 0;


/*
**	This function is only called from within the *.c file and is used to create an ObjMesh structure and
**	initialise its values (adds the mesh to the linked list also).
*/
ObjMesh *MakeOBJ()
{
	/*
	**	The pointer we will create the mesh at the end of
	*/
	ObjMesh *pMesh = NULL;


	pMesh = (ObjMesh*) malloc (sizeof(ObjMesh));

	/*	If the program asserts here, then there was a memory allocation failure	*/
	assert(pMesh);

	/*
	**	Initialise all pointers to NULL
	*/
	pMesh->m_aFaces				= NULL;
	pMesh->m_aNormalArray		= NULL;
	pMesh->m_aTexCoordArray		= NULL;
	pMesh->m_aVertexArray		= NULL;
	pMesh->m_iNumberOfFaces		= 0;
	pMesh->m_iNumberOfNormals	= 0;
	pMesh->m_iNumberOfTexCoords = 0;
	pMesh->m_iNumberOfVertices	= 0;
	pMesh->m_iMeshID			= ++g_ObjIdGenerator;
	pMesh->m_aTypesOfFaces		= NULL;


	/*
	**	Insert the mesh at the beginning of the linked list
	*/
	pMesh->m_pNext				= g_LinkedListHead;
	g_LinkedListHead			= pMesh;

	return pMesh;
}


ObjFile LoadOBJ(const char *filename)
{
	ObjMesh *pMesh=NULL;
	unsigned int vc=0,nc=0,tc=0,fc=0,i;
	char buffer[256];
	FILE *fp = NULL;

	/*
	**	Open the file for reading
	*/
	fp = fopen(filename,"r");

	/*
	**	If the program asserted here, then the file could not be found or opened
	*/
	assert(fp);

	/*
	**	Create the mesh structure and add it to the linked list
	*/
	pMesh = MakeOBJ();

	/*
	**	Run through the whole file looking for the various flags so that we can count
	**	up how many data elements there are. This is done so that we can make one memory
	**	allocation for the meshes data and then run through the file once more, this time
	**	reading in the data. It's purely done to reduce system overhead of memory allocation due
	**	to otherwise needing to reallocate data everytime we read in a new element.
	*/
	while(fgets(buffer,255,fp) != NULL)
	{	/*	Grab a line at a time	*/

		/*	look for the 'vn' - vertex normal - flag	*/
		if( strncmp("vn ",buffer,3) == 0 )
		{
			++pMesh->m_iNumberOfNormals;
		}
		else

		/*	look for the 'vt' - texturing co-ordinate - flag  */
		if( strncmp("vt ",buffer,3) == 0 )
		{
			++pMesh->m_iNumberOfTexCoords;
		}
		else

		/*	look for the 'v ' - vertex co-ordinate - flag  */
		if( strncmp("v ",buffer,2) == 0 )
		{
			++pMesh->m_iNumberOfVertices;
		}
		else

		/*	look for the 'f ' - face - flag  */
		if( strncmp("f ",buffer,2) == 0 )
		{
			++pMesh->m_iNumberOfFaces;
		}
	}

	/*
	**	close the file for now....
	*/
	fclose(fp);


	//default types of faces: OTHER
	pMesh->m_aTypesOfFaces = (unsigned char*) calloc(pMesh->m_iNumberOfFaces, sizeof(unsigned char));
	for (i=0; i < pMesh->m_iNumberOfFaces; i++)
	{
		pMesh->m_aTypesOfFaces[i] = 0;
	}


	/*
	**	Allocate the memory for the data arrays and check that it allocated ok
	*/

	pMesh->m_aVertexArray	= (ObjVertex*  )malloc( pMesh->m_iNumberOfVertices	* sizeof(ObjVertex)	  );
	assert(pMesh->m_aVertexArray);

	/*	there are occasionally times when the obj does not have any normals in it */
	if( pMesh->m_iNumberOfNormals > 0 )
	{
		pMesh->m_aNormalArray	= (ObjNormal*  )malloc( pMesh->m_iNumberOfNormals	* sizeof(ObjNormal)	  );
		assert(pMesh->m_aNormalArray);
	}

	/*	there are occasionally times when the obj does not have any tex coords in it */
	if( pMesh->m_iNumberOfTexCoords > 0 )
	{
		pMesh->m_aTexCoordArray = (ObjTexCoord*)malloc( pMesh->m_iNumberOfTexCoords	* sizeof(ObjTexCoord) );
		assert(pMesh->m_aTexCoordArray);
	}

	pMesh->m_aFaces			= (ObjFace*    )malloc( pMesh->m_iNumberOfFaces		* sizeof(ObjFace)	  );
	assert(pMesh->m_aFaces);

	/*
	**	Now we know how much data is contained in the file and we've allocated memory to hold it all.
	**	What we can therefore do now, is load up all of the data in the file easily.
	*/
	fp = fopen(filename,"r");

	while(fgets(buffer,255,fp) != NULL)
	{	/*	Grab a line at a time	*/

		/*	look for the 'vn' - vertex normal - flag	*/
		if( strncmp("vn ",buffer,3) == 0 )
		{
			sscanf((buffer+2),"%f%f%f",
							&pMesh->m_aNormalArray[ nc ].x,
							&pMesh->m_aNormalArray[ nc ].y,
							&pMesh->m_aNormalArray[ nc ].z);
			++nc;
		}
		else

		/*	look for the 'vt' - texturing co-ordinate - flag  */
		if( strncmp("vt ",buffer,3) == 0 )
		{
			sscanf((buffer+2),"%f%f",
							&pMesh->m_aTexCoordArray[ tc ].u,
							&pMesh->m_aTexCoordArray[ tc ].v);
			++tc;
		}
		else

		/*	look for the 'v ' - vertex co-ordinate - flag  */
		if( strncmp("v ",buffer,2) == 0 )
		{
			sscanf((buffer+1),"%f%f%f",
							&pMesh->m_aVertexArray[ vc ].x,
							&pMesh->m_aVertexArray[ vc ].y,
							&pMesh->m_aVertexArray[ vc ].z);
			++vc;
		}
		else

		/*	look for the 'f ' - face - flag  */
		if( strncmp("f ",buffer,2) == 0 )
		{
			/*
			**	some data for later....
			*/
			char *pSplitString = NULL;
			unsigned int i,ii = 0;

			/*
			**	Pointer to the face we are currently dealing with. It's only used so that
			**	the code becomes more readable and I have less to type.
			*/
			ObjFace *pf = &pMesh->m_aFaces[ fc ];

			/*
			**	These next few lines are used to figure out how many '/' characters there
			**	are in the string. This gives us the information we need to find out how
			**	many vertices are used in this face (by dividing by two)
			*/
			for(i=0;i<strlen(buffer);i++)
			{
				if(buffer[i] == '/')
					ii++;
			}
			ii/=2;

			/*
			**	Allocate the indices for the vertices of this face
			*/
			pf->m_aVertexIndices	= (unsigned int*)malloc( ii * sizeof(unsigned int) );

			/*
			**	Allocate the indices for the normals of this face only if the obj file
			**	has normals stored in it.
			*/
			if( pMesh->m_iNumberOfNormals > 0 )
			{
				pf->m_aNormalIndices	= (unsigned int*)malloc( ii * sizeof(unsigned int) );
			}

			/*
			**	Allocate the indices for the texturing co-ordinates of this face only if the obj file
			**	has texturing co-ordinates stored in it.
			*/
			if( pMesh->m_iNumberOfTexCoords > 0 )
			{
				pf->m_aTexCoordIndicies = (unsigned int*)malloc( ii * sizeof(unsigned int) );
			}

			/*
			**	tokenise the string using strtok(). Basically this splits the string up
			**	and removes the spaces from each chunk. This way we only have to deal with
			**	one set of indices at a time for each of the poly's vertices.
			*/
			pSplitString = strtok((buffer+2)," \t\n");
			i=0;
			do
			{
				if( tc > 0 &&
					nc > 0 )
				{
					sscanf(pSplitString, "%d/%d/%d",
						&pf->m_aVertexIndices   [i],
						&pf->m_aTexCoordIndicies[i],
						&pf->m_aNormalIndices   [i] );

					/* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
					--pf->m_aTexCoordIndicies[i];
					--pf->m_aNormalIndices   [i];
				}
				else
				if( tc > 0 )
				{
					sscanf(pSplitString, "%d/%d/",
						&pf->m_aVertexIndices   [i],
						&pf->m_aTexCoordIndicies[i] );

					/* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
					--pf->m_aTexCoordIndicies[i];
				}
				else
				if( nc > 0 )
				{
					sscanf(pSplitString, "%d//%d",
						&pf->m_aVertexIndices   [i],
						&pf->m_aNormalIndices   [i] );

					/* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
					--pf->m_aNormalIndices   [i];
				}

				/* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
				--pf->m_aVertexIndices[i];

				++i;
				pSplitString = strtok(NULL," \t\n");
			}
			while( pSplitString );
			pf->m_iVertexCount = i;
			++fc;
		}
	}

	fclose(fp);

	return pMesh->m_iMeshID;
}


void DrawOBJ(ObjFile id)
{
	float StandartColor[3] = {0.5f, 0.5f, 0.5f},
		FirstVisionColor[3] = {0.5f, 0.0f, 0.0f},
		SecondVisionColor[3] = {0.0f, 0.5f, 0.0f},
		DoubleVisionColor[3] = {0.5f, 0.0f, 0.5f};


	/*
	**	Because the meshes are on a linked list, we first need to find the
	**	mesh with the specified ID number so traverse the list.
	*/
	ObjMesh *pMesh = g_LinkedListHead;

	while(pMesh && pMesh->m_iMeshID != id)
	{
		pMesh = pMesh->m_pNext;
	}

	/*
	**	Check to see if the mesh ID is valid.
	*/
	if(pMesh != NULL)
	{
		/*
		**	In order to draw our mesh, we would basically like something of the following :
		**
		**	for_each_polygon_in_mesh
		**	{
		**		start_drawing_poly;
		**		for_each_vertex_in_the_poly
		**		{
		**			use_the_vertex_index_to_find_vertex_in_the_mesh's_array
		**		}
		**	}
		*/

		/*
		**	Check for the existance of uv coords and normals
		*/
		if( pMesh->m_aNormalArray   != NULL &&
			pMesh->m_aTexCoordArray != NULL )
		{
			unsigned int i;
			for(i=0;i<pMesh->m_iNumberOfFaces;i++)
			{
				unsigned int j;
				ObjFace *pf = &pMesh->m_aFaces[i];

				/*
				**	Draw the polygons with normals & uv co-ordinates
				*/

				//change color if we know types of face vision
				glColor3fv(  CHOOSE_FACE_COLOR(pMesh->m_aTypesOfFaces[i])  );
				glBegin(GL_POLYGON);
				for(j=0;j<pf->m_iVertexCount;j++)
				{
					glTexCoord2f( pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].u,
								  pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].v);
					glNormal3f( pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].x,
								pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].y,
								pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].z);
					glVertex3f( pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].x,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].y,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].z);
				}
				glEnd();
				glColor3fv(StandartColor);

			}
		}
		else

		/*
		**	See if just the normals exist
		*/
		if( pMesh->m_aNormalArray   != NULL )
		{
			unsigned int i;
			for(i=0;i<pMesh->m_iNumberOfFaces;i++)
			{
				unsigned int j;
				ObjFace *pf = &pMesh->m_aFaces[i];

				/*
				**	Draw the polygons with Normals only
				*/
				glColor3fv(  CHOOSE_FACE_COLOR(pMesh->m_aTypesOfFaces[i])  );
				glBegin(GL_POLYGON);
				for(j=0;j<pf->m_iVertexCount;j++)
				{
					glNormal3f( pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].x,
								pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].y,
								pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].z);
					glVertex3f( pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].x,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].y,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].z);
				}
				glEnd();
				glColor3fv(StandartColor);


			}
		}
		else

		/*
		**	Do we have any uv-coords
		*/
		if( pMesh->m_aTexCoordArray != NULL )
		{
			unsigned int i;
			for(i=0;i<pMesh->m_iNumberOfFaces;i++)
			{
				unsigned int j;
				ObjFace *pf = &pMesh->m_aFaces[i];

				/*
				**	Draw the polygons with Texturing co-ordinates only
				*/
				glColor3fv( CHOOSE_FACE_COLOR(pMesh->m_aTypesOfFaces[i]) );
				glBegin(GL_POLYGON);
				for(j=0;j<pf->m_iVertexCount;j++)
				{
					glTexCoord2f( pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].u,
								  pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].v);
					glVertex3f( pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].x,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].y,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].z);
				}
				glEnd();
				glColor3fv(StandartColor);

			}
		}
	}
}


/*
**	Calling free() on NULL is VERY BAD in C, so make sure we
**	check all pointers before calling free.
*/
void DeleteMesh(ObjMesh* pMesh)
{
	/*
	**	If the pointer is valid
	*/
	if(pMesh)
	{
		/*	delete the face array */
		if(pMesh->m_aFaces)
		{
			int i;
			for (i = 0; i < pMesh->m_iNumberOfFaces; i++)
			{
				if (pMesh->m_aFaces[i].m_aVertexIndices)
				{
					free(pMesh->m_aFaces[i].m_aVertexIndices);
					pMesh->m_aFaces[i].m_aVertexIndices = NULL;
				}

				if (pMesh->m_aFaces[i].m_aNormalIndices)
				{
					free(pMesh->m_aFaces[i].m_aNormalIndices);
					pMesh->m_aFaces[i].m_aNormalIndices = NULL;
				}

				if (pMesh->m_aFaces[i].m_aTexCoordIndicies)
				{
					free(pMesh->m_aFaces[i].m_aTexCoordIndicies);
					pMesh->m_aFaces[i].m_aTexCoordIndicies = NULL;
				}
			}

			free(pMesh->m_aFaces);
			pMesh->m_aFaces = NULL;
		}

		/*	delete the vertex array */
		if(pMesh->m_aVertexArray)
		{
			free(pMesh->m_aVertexArray);
			pMesh->m_aVertexArray = NULL;
		}

		/*	delete the normal array */
		if(pMesh->m_aNormalArray)
		{
			free(pMesh->m_aNormalArray);
			pMesh->m_aNormalArray = NULL;
		}

		/*	delete the texturing co-ordinate array */
		if(pMesh->m_aTexCoordArray)
		{
			free(pMesh->m_aTexCoordArray);
			pMesh->m_aTexCoordArray = NULL;
		}

		//delete Types of Faces
		if (pMesh->m_aTypesOfFaces)
		{
			free(pMesh->m_aTypesOfFaces);
			pMesh->m_aTypesOfFaces = NULL;
		}

		//delete type of faces
		free(pMesh->m_aTypesOfFaces);

		/*	free the mesh */
		free( pMesh );
	}
}


void DeleteOBJ(ObjFile id)
{
	/*
	**	Create two pointers to walk through the linked list
	*/
	ObjMesh *pCurr,
			*pPrev = NULL;

	/*
	**	Start traversing the list from the start
	*/
	pCurr = g_LinkedListHead;

	/*
	**	Walk through the list until we either reach the end, or
	**	we find the node we are looking for
	*/
	while(pCurr != NULL && pCurr->m_iMeshID != id)
	{
		pPrev = pCurr;
		pCurr = pCurr->m_pNext;
	}

	/*
	**	If we found the node that needs to be deleted
	*/
	if(pCurr != NULL)
	{
		/*
		**	If the pointer before it is NULL, then we need to
		**	remove the first node in the list
		*/
		if(pPrev == NULL)
		{
			g_LinkedListHead = pCurr->m_pNext;
		}

		/*
		**	Otherwise we are removing a node from somewhere else
		*/
		else
		{
			pPrev->m_pNext = pCurr->m_pNext;
		}

		/*
		**	Free the memory allocated for this mesh
		*/

		DeleteMesh(pCurr);
	}
}


/*
**	Delete all of the meshes starting from the front.
*/
void CleanUpOBJ(void)
{
	ObjMesh *pCurr;
	while(g_LinkedListHead != NULL)
	{
		pCurr = g_LinkedListHead;
		g_LinkedListHead = g_LinkedListHead->m_pNext;
		DeleteMesh(pCurr);
	}
}


/*
** Return pointer to ObjMesh structure by his ID ObjFile.
** Used for creating CalcMesh
*/
ObjMesh* ReturnObjMesh(ObjFile objFile)
{
	/*
	**	Create two pointers to walk through the linked list
	*/
	ObjMesh *pCurr;

	/*
	**	Start traversing the list from the start
	*/
	pCurr = g_LinkedListHead;

	/*
	**	Walk through the list until we either reach the end, or
	**	we find the node we are looking for
	*/
	while(pCurr != NULL && pCurr->m_iMeshID != objFile)
	{
		pCurr = pCurr->m_pNext;
	}

	return pCurr;
}
