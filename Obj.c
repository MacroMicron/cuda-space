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
#include <math.h>



//macros for change color which depends of type of the color (first, second, double or other)
//output: the float[3] vector (StandartColor/UndefinedVisionColor, FirstVisionColor, SecondVisionColor, DoubleVisionColor, OtherVision)
//input: {0, 1, 2, 3, 4}
#define CHOOSE_FACE_COLOR(Type) ((Type) ? ( (Type)==1 ? FirstVisionColor : ((Type) == 2 ? SecondVisionColor : ( (Type)==3 ? DoubleVisionColor : OtherVisionColor)) ) : StandartColor)

//buffer for read string
#define BUFFER 256 


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


//in this implementation only one light
//TODO support many lights
GLUquadricObj *GlobalLight = NULL;


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
	pMesh->m_aLights		= NULL;
	pMesh->m_aSpherePolygonRadiosity	= NULL;
	pMesh->m_aSphereVertexArray	= NULL;	

	/*
	**	Insert the mesh at the beginning of the linked list
	*/
	pMesh->m_pNext				= g_LinkedListHead;
	g_LinkedListHead			= pMesh;
	
	
	ChangeNumberSphereDetalisation(32, pMesh->m_iMeshID);		
	//it is like pMesh->m_iNumberSphereDetalisation = 256;
	//there is restrict on this number: i think only 2^n
	
	return pMesh;
}


//Change the detalisation of the spaceship outside sphere
//I think only 2^n numbers
void ChangeNumberSphereDetalisation(unsigned int SphereDetalisation, ObjFile id)
{
        ObjMesh *pMesh = g_LinkedListHead;
        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }
	
        if (pMesh != NULL)
        {
		if (pMesh->m_aSpherePolygonRadiosity)
		{
			free(pMesh->m_aSpherePolygonRadiosity);
		}
		if (pMesh->m_aSphereVertexArray)
		{
			free(pMesh->m_aSphereVertexArray);	
		}
		if (SphereDetalisation >= 3 )
		{
			unsigned int i, j;
			pMesh->m_iNumberSphereDetalisation = SphereDetalisation;
			pMesh->m_aSpherePolygonRadiosity = (float*) calloc(SphereDetalisation*(SphereDetalisation-1), sizeof(float));
			assert(pMesh->m_aSpherePolygonRadiosity);
			for (i=0; i < SphereDetalisation*(SphereDetalisation-1); i++)
			{
				pMesh->m_aSpherePolygonRadiosity[i] = 0.0;
				if (i%2) pMesh->m_aSpherePolygonRadiosity[i]+=0.5;
			}
			pMesh->m_aSphereVertexArray = (ObjVertex*) calloc(SphereDetalisation*SphereDetalisation, sizeof(ObjVertex));
			assert(pMesh->m_aSphereVertexArray);


			float phi, alpha, pi = 3.141592653;
                	float r = 3000.0;		//if you change this, don't forget to change in other places!
                	for (i=0; i<SphereDetalisation; i++)
                	{
                        	for (j=0; j<SphereDetalisation; j++)
                        	{
	                                phi = -pi/2.0 + i*pi/SphereDetalisation;
        	                        alpha = -pi + j*2.0*pi/SphereDetalisation;
        	                                
					pMesh->m_aSphereVertexArray[SphereDetalisation*i+j].x = r*cos(phi)*cos(alpha);
					pMesh->m_aSphereVertexArray[SphereDetalisation*i+j].y = r*cos(phi)*sin(alpha);
					pMesh->m_aSphereVertexArray[SphereDetalisation*i+j].z = r*sin(phi);
                                }    
			}
		}	
	}
}


//Draw a big the sphere outside of the spaceship 
//if you don't want to draw sphere just set at zero ChangeNumberSpherePolygons(0) 
//PS 4 polygons - minimal available polygons for this 'Sphere'
void DrawSphere(ObjFile id)
{
        ObjMesh *pMesh = g_LinkedListHead;
        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

        if (pMesh != NULL)
        {
		if (pMesh->m_aSphereVertexArray)
		{
			unsigned int i, j, i_, j_, SphereDetalisation = pMesh->m_iNumberSphereDetalisation;
			float SphereColor[] = {0.0f, 0.0f, 0.25f};
			for (i=1; i<SphereDetalisation; i++)	//important! NumberOf(polygons)=SphereDetalisation*(SphereDetalisation-1)
			{ 
				for (j=0; j<SphereDetalisation; j++)
				{
					i_ = i-1; j_ = j-1;
					if (j_==-1) j_+=SphereDetalisation;
					SphereColor[2] = 0.25 + pMesh->m_aSpherePolygonRadiosity[SphereDetalisation*i+j];
					glColor3fv(SphereColor);
 					glBegin(GL_POLYGON);
	                               	{
						
						//glNormal3f( x, y, z);
						glVertex3f(pMesh->m_aSphereVertexArray[SphereDetalisation*i+j].x, 
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i+j].y,
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i+j].z);
						
	                                	glVertex3f(pMesh->m_aSphereVertexArray[SphereDetalisation*i_+j].x, 
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i_+j].y,
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i_+j].z);
						
						glVertex3f(pMesh->m_aSphereVertexArray[SphereDetalisation*i_+j_].x,
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i_+j_].y,
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i_+j_].z);

						glVertex3f(pMesh->m_aSphereVertexArray[SphereDetalisation*i+j_].x,
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i+j_].y,
							   pMesh->m_aSphereVertexArray[SphereDetalisation*i+j_].z);
					}	
					glEnd();
				}
			}
		}	
	}
}



ObjFile LoadOBJ(const char *filename)
{
	ObjMesh *pMesh=NULL;
	unsigned int vc=0,nc=0,tc=0,fc=0,i;
	char buffer[BUFFER], buffer2[BUFFER];
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
	while(fgets(buffer, BUFFER, fp) != NULL)
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


	//default types of faces: UNDEFINED
	if ( pMesh->m_iNumberOfFaces > 0)
	{
		pMesh->m_aTypesOfFaces = (unsigned int*) calloc(pMesh->m_iNumberOfFaces, sizeof(unsigned int));
		assert(pMesh->m_aTypesOfFaces);
		for (i=0; i < pMesh->m_iNumberOfFaces; i++) 
		{
			pMesh->m_aTypesOfFaces[i] = 0;
		}
	}

	/*
	**	Allocate the memory for the data arrays and check that it allocated ok
	*/
	
	if ( pMesh->m_iNumberOfVertices > 0)
	{
		pMesh->m_aVertexArray	= (ObjVertex*  )malloc( pMesh->m_iNumberOfVertices	* sizeof(ObjVertex)	  );
		assert(pMesh->m_aVertexArray);
	}

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
	
	if( pMesh->m_iNumberOfFaces > 0)
	{
		pMesh->m_aFaces			= (ObjFace*    )malloc( pMesh->m_iNumberOfFaces		* sizeof(ObjFace)	  );
		assert(pMesh->m_aFaces);
	}
	
	/*
	**	Now we know how much data is contained in the file and we've allocated memory to hold it all.
	**	What we can therefore do now, is load up all of the data in the file easily.
	*/
	fp = fopen(filename,"r");
	
	while(fgets(buffer, BUFFER, fp) != NULL)
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
			unsigned int i,ii = 0, iii=0;

			/*
			**	Pointer to the face we are currently dealing with. It's only used so that
			**	the code becomes more readable and I have less to type.
			*/
			ObjFace *pf = &pMesh->m_aFaces[ fc ];
			
			/*	We can have
			**	[%d/%d/%d | %d//%d | %d/%d/ | %d/%d | %d]^n 
			**	where n vertices in this polygon
			*/
			
			
			//number of '/' in one vertex (0, 1 or 2)	
			sscanf(buffer+2,"%s",buffer2);
			for(i=0;i<strlen(buffer2);i++)
			{
				if(buffer2[i] == '/')
					ii++;
			}
			
			//number of '/' in all line (0, n or 2*n)
			for (i=0; i<strlen(buffer);i++)
			{
				if(buffer[i] == '/')
					iii++;
			}
			
			//number of vertices - n = iii/ii or if ii==0 then number of numbers in buffer string
			if (ii) iii = iii/ii;
			else
			{	
				i=iii=0; 
				while (i < strlen(buffer))
        	                {
                	                if ((buffer[i]<'0')||(buffer[i]>'9'))
                        	        { i++; continue;}
                                	iii++;
          	                      while ((buffer[i]>='0')&&(buffer[i])<='9') i++;
                	        }


			}
						
			/*
			**	Allocate the indices for the vertices of this face
			*/

			pf->m_aVertexIndices	= (unsigned int*)malloc( iii * sizeof(unsigned int) );
			assert(pf->m_aVertexIndices);
			pf->m_iVertexCount = iii;

			
			i=0;
			if (ii==0) // this way: v
			{	
				pf->m_aTexCoordIndicies = NULL;
				pf->m_aNormalIndices = NULL;
                                pSplitString = strtok((buffer+2)," \t\n");
                                do {
                                        sscanf(pSplitString, "%d",
                                        &pf->m_aVertexIndices[i] );
                                        /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                                        --pf->m_aVertexIndices[i];
                                        i++;
                                }
                                while (pSplitString = strtok(NULL," \t\n"));
  
			}
			else if (ii==1) // this way: v/vt 	
			{
				pf->m_aTexCoordIndicies = (unsigned int*)malloc( iii * sizeof(unsigned int) );
			 	assert( pf->m_aTexCoordIndicies );
				pf->m_aNormalIndices = NULL;
				//for(i=0; i<iii; i++) 
                         	pSplitString = strtok((buffer+2)," \t\n");
                        	do {
               		          	sscanf(pSplitString, "%d/%d",
					&pf->m_aVertexIndices   [i],
                                	&pf->m_aTexCoordIndicies[i] );
                      		        /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
					--pf->m_aTexCoordIndicies[i];
					--pf->m_aVertexIndices[i];
					i++;
				}
				while (pSplitString = strtok(NULL," \t\n"));
			}
			else if ( ii == 2 )
			{
				if ( buffer2[strlen(buffer2)-1] == '/' )    //this way: v/vt/
					{
						pf->m_aTexCoordIndicies = (unsigned int*)malloc( iii * sizeof(unsigned int) );
						assert( pf->m_aTexCoordIndicies );
						pf->m_aNormalIndices = NULL;
						//for(i=0; i<iii; i++) 
						pSplitString = strtok((buffer+2)," \t\n");
						do {
							sscanf(pSplitString, "%d/%d/",
	                                                &pf->m_aVertexIndices   [i],
        	                                        &pf->m_aTexCoordIndicies[i] );
                	                              /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                        	                        --pf->m_aTexCoordIndicies[i];
							--pf->m_aVertexIndices[i];
							i++;
						}
						while (pSplitString = strtok(NULL," \t\n"));

					}
				else if ( strstr(buffer2, "//") )    //this way: v//vn
				{
					pf->m_aNormalIndices    = (unsigned int*)malloc( iii * sizeof(unsigned int) );
					assert( pf->m_aNormalIndices );
					pf->m_aTexCoordIndicies = NULL;
                                        //for(i=0; i<iii; i++) 
                                        pSplitString = strtok((buffer+2)," \t\n");
                                        do {
                                                sscanf(pSplitString, "%d//%d",
                                                &pf->m_aVertexIndices   [i],
                                                &pf->m_aNormalIndices   [i] );
                                                /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                                                --pf->m_aVertexIndices[i];
                                                --pf->m_aNormalIndices[i];
                                                i++;
                                        }
                                        while (pSplitString = strtok(NULL," \t\n"));
					
				}
				else	//this way: v/vt/vn
				{
					pf->m_aTexCoordIndicies = (unsigned int*)malloc( iii * sizeof(unsigned int) );
					assert( pf->m_aTexCoordIndicies );
					pf->m_aNormalIndices    = (unsigned int*)malloc( iii * sizeof(unsigned int) );
					assert( pf->m_aNormalIndices );
					
				        //for(i=0; i<iii; i++) 
                                        pSplitString = strtok((buffer+2)," \t\n");
                                        do {
                                        	sscanf(pSplitString, "%d/%d/%d",
                                                &pf->m_aVertexIndices   [i],
                                                &pf->m_aTexCoordIndicies[i],
						&pf->m_aNormalIndices   [i] );
                                                /* need to reduce the indices by 1 because array indices start at 0, obj starts at 1  */
                                                --pf->m_aTexCoordIndicies[i];
                                                --pf->m_aVertexIndices[i];
						--pf->m_aNormalIndices[i];
                                                i++;
                                        }
                                        while (pSplitString = strtok(NULL," \t\n"));
					
				}
			}
			++fc;
		}

		//special cuda-space comments
		//for light, only 1 (first) in this program implementation	
		//#spacelight vx vy vz
		if( strncmp("#spacelight ",buffer,12) == 0)
		{
			if (!pMesh->m_aLights)
			{
				pMesh->m_aLights = (ObjVertex*) malloc(sizeof(ObjVertex));
				assert(pMesh->m_aLights);
				sscanf(buffer+11,"%f%f%f",
                                                        &pMesh->m_aLights[ 0 ].x,
                                                        &pMesh->m_aLights[ 0 ].y,
                                                        &pMesh->m_aLights[ 0 ].z);
				if (GlobalLight == NULL)
		                {
                	        	GlobalLight = gluNewQuadric();
                        		assert(GlobalLight);
                		}

			}
		}

		//special cuda-space comments
		//for only one light - only one array of face types
		//#spacetypes fn t
		if( strncmp("#spacetypes ",buffer,12) == 0)
		{
			unsigned int i, t;
			char *pSplitString = strtok((buffer+11)," \t\n");
			sscanf(pSplitString, "%d", &i);
			i--; // need to reduce the indices by 1 because array indices start at 0, obj starts at 1 
			if ((i>=0) && (i<pMesh->m_iNumberOfFaces))
			{
				
				pSplitString = strtok(NULL," \t\n");
				sscanf(pSplitString, "%d", &t);
				if ((t>=0) && (t<=4))
				{
					pMesh->m_aTypesOfFaces[i] = t;
				}
                        }
		}
	}

	fclose(fp);

	return pMesh->m_iMeshID;
}

//Save OBJ file with all informatin about faces: first or second type. In comments: #
void SaveOBJ(const ObjFile id, const char *filename)
{
	FILE *fp = NULL;
	fp = fopen(filename, "w");
	assert(fp);
	
        ObjMesh *pMesh = g_LinkedListHead;
        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

	if (pMesh != NULL)
	{
		unsigned int i, j;
		ObjFace *pf;
		for (i=0; i < pMesh->m_iNumberOfVertices;i++)
		{
			fprintf(fp, "v %f %f %f\n", pMesh->m_aVertexArray[i].x, pMesh->m_aVertexArray[i].y, pMesh->m_aVertexArray[i].z);
		}
		for (i=0; i < pMesh->m_iNumberOfTexCoords; i++)
		{
			fprintf(fp, "vt %f %f\n", pMesh->m_aTexCoordArray[i].u, pMesh->m_aTexCoordArray[i].v);
		}
		for (i=0; i < pMesh->m_iNumberOfNormals; i++)
		{
			fprintf(fp, "vn %f %f %f\n", pMesh->m_aNormalArray[i].x, pMesh->m_aNormalArray[i].y, pMesh->m_aNormalArray[i].z);
		}
		for (i=0, pf=pMesh->m_aFaces; i < pMesh->m_iNumberOfFaces; i++, pf=&pMesh->m_aFaces[i])
		{
			fprintf(fp,"f");
			if (( pf->m_aTexCoordIndicies != NULL )&&( pf->m_aNormalIndices != NULL))
			{	
				for (j=0; j < pf->m_iVertexCount; j++)
				{
					fprintf(fp, " %d/%d/%d", pf->m_aVertexIndices[j]+1, pf->m_aTexCoordIndicies[j]+1, pf->m_aNormalIndices[j]+1);
				}
			}
			else if (( pf->m_aTexCoordIndicies != NULL )&&( pf->m_aNormalIndices == NULL))
			{
				for (j=0; j < pf->m_iVertexCount; j++)
                                {
                                        fprintf(fp, " %d/%d", pf->m_aVertexIndices[j]+1, pf->m_aTexCoordIndicies[j]+1);
                                }
			}
			else if (( pf->m_aTexCoordIndicies == NULL )&&( pf->m_aNormalIndices != NULL))
			{
				for (j=0; j < pf->m_iVertexCount; j++)
                                {
                                        fprintf(fp, " %d//%d", pf->m_aVertexIndices[j]+1, pf->m_aNormalIndices[j]+1);
                                }
			}
			else if (( pf->m_aTexCoordIndicies == NULL )&&( pf->m_aNormalIndices == NULL))
                        {
                                for (j=0; j < pf->m_iVertexCount; j++)
                                {
                                        fprintf(fp, " %d", pf->m_aVertexIndices[j]+1);
                                }
			} 
			fprintf(fp, "\n");
		}
		if (pMesh->m_aLights)
		{
			fprintf(fp, "#spacelight %f %f %f\n", pMesh->m_aLights[0].x, pMesh->m_aLights[0].y, pMesh->m_aLights[0].z);
		}
  	        for (i=0; i < pMesh->m_iNumberOfFaces; i++)
		{ 
			fprintf(fp, "#spacetypes %d %d\n",i+1, pMesh->m_aTypesOfFaces[i]);	
		}
	}	
	
	fclose(fp);
}

void DrawOBJ(const ObjFile id)
{
	float StandartColor[3] = {0.5f, 0.5f, 0.5f},
		FirstVisionColor[3] = {0.5f, 0.0f, 0.0f},
		SecondVisionColor[3] = {0.0f, 0.5f, 0.0f},
		DoubleVisionColor[3] = {0.5f, 0.0f, 0.5f},
		OtherVisionColor[3] = {1.0f, 1.0f, 1.0f};

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
			glPushMatrix();

			unsigned int i;
			for(i=0; i < pMesh->m_iNumberOfFaces; i++)
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
					if ((pMesh->m_aTexCoordArray) && (pf->m_aTexCoordIndicies))
						glTexCoord2f( pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].u,
								  pMesh->m_aTexCoordArray[ pf->m_aTexCoordIndicies[j] ].v);
					if ((pMesh->m_aNormalArray) && (pf->m_aNormalIndices))
						glNormal3f( pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].x,
								pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].y,
								pMesh->m_aNormalArray[ pf->m_aNormalIndices[j] ].z);
					if ((pMesh->m_aVertexArray) && (pf->m_aVertexIndices))
						glVertex3f( pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].x,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].y,
								pMesh->m_aVertexArray[ pf->m_aVertexIndices[j] ].z);
				}
				glEnd();
				glColor3fv(StandartColor);

			}

			glPopMatrix();			
	}
}

/*
** Add Lights (only 1 light in this program implementation)
*/
void AddLight(ObjVertex vertex, ObjFile id)
{
        /*
        **      Because the meshes are on a linked list, we first need to find the
        **      mesh with the specified ID number so traverse the list.
        */
        ObjMesh *pMesh = g_LinkedListHead;

        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

        /*
        **      Check to see if the mesh ID is valid.
        */
        if (pMesh != NULL)
        {
		if (pMesh->m_aLights == NULL)
		{
			pMesh->m_aLights = (ObjVertex*) malloc(sizeof(ObjVertex));
			assert(pMesh->m_aLights);
		}
                if (GlobalLight == NULL)
                {
                        GlobalLight = gluNewQuadric();
                        assert(GlobalLight);
                }
		pMesh->m_aLights[0].x = vertex.x;
		pMesh->m_aLights[0].y = vertex.y;
		pMesh->m_aLights[0].z = vertex.z;
	}	
}


/*
** Draw Lights (only 1 light in this program implementation
*/
void DrawLights(const ObjFile id)
{
        float LightColor[3] = {1.0f, 1.0f, 0.0f};

        /*
        **      Because the meshes are on a linked list, we first need to find the
        **      mesh with the specified ID number so traverse the list.
        */
        ObjMesh *pMesh = g_LinkedListHead;

        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

        /*
        **      Check to see if the mesh ID is valid.
        */
        if(pMesh != NULL)
        {
		if ((pMesh->m_aLights != NULL) && (GlobalLight != NULL))
		{
			glPushMatrix();	
			glTranslated(pMesh->m_aLights[0].x, pMesh->m_aLights[0].y, pMesh->m_aLights[0].z);
			gluQuadricDrawStyle(GlobalLight, GLU_FILL);
  			glColor3fv(LightColor);
  			gluSphere(GlobalLight, 20,10,10);
			glPopMatrix();
		}
	}
}


/*
** Remove Light (only 1 light can remove in this program implementation)
*/
void RemoveLight(ObjFile id)
{
        ObjMesh *pMesh = g_LinkedListHead;

        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

        if (pMesh != NULL)
        {
                if (pMesh->m_aLights != NULL)
                {        
			free(pMesh->m_aLights);
			pMesh->m_aLights = NULL;
                }
		if (GlobalLight != NULL)
		{
			gluDeleteQuadric(GlobalLight);
                        GlobalLight = NULL;
		}
        }		
}


/*
**	only one light in this program implementation
*/
ObjVertex* GetLights(ObjFile id)
{
        ObjMesh *pMesh = g_LinkedListHead;

        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

        if (pMesh != NULL)
        {
		return pMesh->m_aLights;
	}
	return NULL;
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
		
		if (pMesh->m_aLights)
		{
			free(pMesh->m_aLights);
			pMesh->m_aLights = NULL;
		}
		
		if (GlobalLight)
		{
			gluDeleteQuadric(GlobalLight);
                        GlobalLight = NULL;
		}
						
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


//
void FlushTypesOfFaces(ObjFile id)
{
        ObjMesh *pMesh = g_LinkedListHead;

        while(pMesh && pMesh->m_iMeshID != id)
        {
                pMesh = pMesh->m_pNext;
        }

        if (pMesh != NULL)
        {
                if (pMesh->m_aTypesOfFaces != NULL)
                {
			unsigned int i;
			for (i=0; i < pMesh->m_iNumberOfFaces; i++)
			{
				pMesh->m_aTypesOfFaces[i] = 0;				
			}
		}
	}	
}

