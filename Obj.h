//Obj - is a model.

#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_


//different ways for calculating
#define ON_GPU 1
#define ON_CPU 0

#ifdef __cplusplus
extern "C" {
#endif

	//ObjFile descriptor (ID)
	typedef unsigned int ObjFile;

	//global variable about light-point - electro-magnetic arzhan
	typedef struct Light
	{
		float x, y, z;
	};

	/*
	**	func	:	LoadOBJ
	**	params	:	filename	-	the name of the file you wish to load
	**	returns	:	a reference ID for this file. Will return 0 if the file failed to load
	**	notes	:	loads the *.obj file up and stores all the data internally to make the
	**				source nice and easy to use. Each objfile loaded is stored as a node on
	**				a linked list and uses a single unsigned integer as an ID to allow you
	**				to reference it.
	*/
	ObjFile LoadOBJ(const char *filename);
	
	/*	notes	:	Save OBJ to file *.obj with inforamtion about first and second faces in
	**			comments. 
	*/
	void SaveOBJ(const ObjFile id, const char *filename);
	
	/*
	**	func	:	DrawOBJ
	**	params	:	id	-	a reference ID to the mesh you wish to draw
	**	returns	:	nothing
	**	notes	:	Gets the node related to the reference number and draws its data.
	*/
	void DrawOBJ(ObjFile id);

	/*
	**	func	:	DeleteOBJ
	**	params	:	id	-	a reference ID to the mesh you wish to delete
	**	returns	:	nothing
	**	notes	:	Deletes the requested mesh node from the linked list, and frees all the data
	*/
	void DeleteOBJ(ObjFile id);

	/*
	**	func	:	CleanUpOBJ
	**	returns	:	nothing
	**	notes	:	Deletes all the currently loaded data. Handy for cleaning up all data easily
	*/
	void CleanUpOBJ();



	/*
	** Return pointer to ObjMesh structure by his ID ObjFile.
	** Used for creating CalcMesh
	*/
	//ObjMesh* ReturnObjMesh(ObjFile objFile);



#ifdef __cplusplus
}
#endif

#endif
