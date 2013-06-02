//It has the program startup point (Main)
//There is main part of user interface definiton: window, mouse and keyboard controller, promt, etc.
//Dependences: glut.h, glu.h, gl.h (OpenGL uses)

#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <math.h>
#include <stdio.h>

#include "Bool.h"
//#include "FileDialog.h"
#include "Obj.h"
#include "Camera.h"
#include "Bmp.h"

//just temp, delete after
#include "MathLinear.cuh"

//const promt
const char* instructionList = "\nInstructions:\n"
		"	L - load *.obj file\n"
		"	V - save *.obj file\n"
		"	A - left\n"
		"	D - right\n"
		"	S - back\n"
		"	W - front\n"
		"	Q - up\n"
		"	E - down\n"
		"	Left Mouse Button - Rotate Camera\n"
		"	'+' - increment moving step\n"
		"	'-' - decrement moving step\n"
		"	H or I - show instruction list\n"
		"	X - exit from program\n"
		"	'[' - add light\n" 			//only 1 light in this implementation
		"	']' - remove light\n"
		"	N - move to the next light\n" 		//only 1 light not Previous light command
		"	C - count first, second, double vision faces\n"
		"	M - to save Sphere Map picture into bitmap file\n"		 
		"\n";

//just a bug hack
ObjMesh* ReturnObjMesh(ObjFile objFile);

//global variables
Camera camera;

struct Mouse {
	Bool LeftButton, MiddleButton;
	int CursorLastPosition[2];
} mouse;

//object for calculatings
ObjFile object;

//input variables
int Argc;
char **Argv;
char *Filename=NULL;

//procedures
void InitSettings() {
	glClearColor(0.0f, 0.0f, 0.08f, 0.0f); //clear screen and take color
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING); //Light-Mode Enable
	glEnable(GL_LIGHT0); //Light0 object Enable
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL); //Enable the color of material and textures

	//	Setup Material Porperties & colours for the light
	{
		float Ambient[] = { 0.2f, 0.2f, 0.2f, 0.0f };
		float Diffuse[] = { 0.3f, 0.3f, 0.3f, 0.0f };
		glMaterialfv(GL_FRONT, GL_AMBIENT, Ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, Diffuse);
	}
	{
		float Ambient[] = { 0.2f, 0.2f, 0.2f, 0.0f };
		float Diffuse[] = { 0.3f, 0.3f, 0.3f, 0.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
	}
	{
		float ambient[4] = { 0.1, 0.1, 0.1, 1 };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	}

	// Setup camera settings
	camera.Position[0] = camera.Position[1] = camera.Position[3] = 0.0f;
	camera.View[0] = camera.View[1] = 0.0f;
	camera.View[2] = -1.0f;
	camera.Up[0] = camera.Up[2] = 0.0f;
	camera.Up[1] = 1.0f;
	camera.SpeedLevel = camera.SpeedRotation[0] = camera.SpeedRotation[1] = 0;

	//Setup mouse settings
	mouse.LeftButton = mouse.MiddleButton = False;

	// user first message to promt
	printf("Press 'I' or 'H' to show list of functions.\n");
	printf(instructionList);

	//temp
	if (Filename)
	{
		//char* filename = "D:\\Projects\\old space\\space\\model\\elephav.obj";
		//char* filename = "elepham.obj";
		//char* filename = "D:\\Projects\\old space\\space\\model\\soyz.obj";
		printf("%s\n", Filename);
		printf("Loading file... Please Wait...\n");
		object = LoadOBJ(Filename);
		printf("File loaded.\n");
	}
	else Keyboard('L', 0,0);
}
;

CalcVertex LinePoint1, LinePoint2;
CalcMesh *mesh;
CalcVertex light;
ObjVertex oLight;
Bool TryOne = 0;
int iter, it;
double eps, tempo, EPS = 0;

//debug function. delete after
void FirstInit() {
	int time;

	if (!TryOne) {
	        //oLight.x = light.x = 100.0f;
	        //oLight.y = oLight.z = light.y = light.z = 50.0f;
		//AddLight(oLight, object);
		
		//Keyboard('C', 0,0);
	

/*	mesh = CreateCalcMesh(ReturnObjMesh(object));
		printf("DEBUG CreatedCalcMesh\n");

		GPU_example(mesh);
		printf("DEBUG GPU exampled\n");*/
		//ToCountFirstFaces(&light, mesh);
		//time = GetTickCount();

		//ToCountSecondAndDoubleFaces(mesh);
		//time = GetTickCount() - time;		

	/*	CopyResults(mesh, ReturnObjMesh(object));
		printf("DEBUG Copied Results\n");*/
		TryOne = 1;
		/*		for (iter=0; iter < mesh->NumberOfFaces; iter++)
		 {
		 eps = 0;
		 for (it=0; it < mesh->Faces[it].VertexCount; it++)
		 {
		 tempo =
		 (mesh->Faces[iter].PlaneCoefficients[0]) * (mesh->Faces[iter].VertexArray[ mesh->Faces[iter].VertexIndices[it] ].x) +
		 (mesh->Faces[iter].PlaneCoefficients[1]) * (mesh->Faces[iter].VertexArray[ mesh->Faces[iter].VertexIndices[it] ].y) +
		 (mesh->Faces[iter].PlaneCoefficients[2]) * (mesh->Faces[iter].VertexArray[ mesh->Faces[iter].VertexIndices[it] ].z) +
		 mesh->Faces[iter].PlaneCoefficients[3];
		 if (tempo > eps) eps = tempo;
		 }
		 if (eps > EPS) EPS = eps;
		 printf("%d face		%f eps\n",iter, eps);
		 }
		 printf("main eps: %f", EPS);*/
		//printf("seconds: %i\n", time);
	}
}

//	GLUT display callback function
void Display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camera.Position[0], camera.Position[1], camera.Position[2],
			camera.Position[0] + camera.View[0],
			camera.Position[1] + camera.View[1],
			camera.Position[2] + camera.View[2], camera.Up[0], camera.Up[1],
			camera.Up[2]);

	//	Draw the model
	//glRotatef(camera.Rotation[0],1.0f,0.0f,0.0f);
	//glRotatef(camera.Rotation[1],0.0f,1.0f,0.0f); //what about Z?
	//glTranslatef(camera.Position[0],camera.Position[1],camera.Position[2]);
	//glLightfv(GL_LIGHT0,GL_POSITION,camera.Position);

	
	FirstInit();
	
	DrawOBJ(object);
	DrawLights(object);
	DrawSphere(object);

	//if (obj!=NULL)
	//if (obj->TypesOfFaces != NULL)
	//	for (integer i= 0; i<obj->NumberOfFaces; i++)
	//	{
	//		if (obj->TypesOfFaces[i] <= 1)
	//		{
	//			glBegin(GL_LINES);
	//			glVertex3f(0,0,0);
	//			glVertex3f(
	//				obj->VertexArray[obj->Faces[i].VertexIndices[0]].x + (2/3) * ((obj->VertexArray[obj->Faces[i].VertexIndices[1]].x + obj->VertexArray[obj->Faces[i].VertexIndices[2]].x)/2 - obj->VertexArray[obj->Faces[i].VertexIndices[0]].x),
	//				obj->VertexArray[obj->Faces[i].VertexIndices[0]].y + (2/3) * ((obj->VertexArray[obj->Faces[i].VertexIndices[1]].y + obj->VertexArray[obj->Faces[i].VertexIndices[2]].y)/2 - obj->VertexArray[obj->Faces[i].VertexIndices[0]].y),
	//				obj->VertexArray[obj->Faces[i].VertexIndices[0]].z + (2/3) * ((obj->VertexArray[obj->Faces[i].VertexIndices[1]].z + obj->VertexArray[obj->Faces[i].VertexIndices[2]].z)/2 - obj->VertexArray[obj->Faces[i].VertexIndices[0]].z)
	//				);
	//			glEnd();
	//		}
	//	}

	glutSwapBuffers();
}

//	GLUT callback function to deal with resizing of the window
void Reshape(int w, int h) {
	/* prevent a crash when minimising */
	if (h == 0)
		h = 1;
	glViewport(0, (GLsizei) 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float) (w / ((h == 0) ? 1 : h)), 0.01f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
 **	GLUT Callback function to deal with keyboard movement
 */
void Keyboard(unsigned char Key, int x, int y) {
	//Load file
	if (Key == 'L' || Key == 'l') {

		/* used to store the returned filename from the file dialog */
		char filename[256];

		printf("Enter path to the file:\n");
		scanf("%s", filename);

		/* get a file dialog to find the file to load */
		//if (OpenFileDialog(filename)) {
			//printf("%s\n", filename);
			printf("Loading file... Please Wait...\n");
			/* attempt to load the file */
			object = LoadOBJ(filename);
			printf("File loaded.\n");
		//}
	}

	if (Key == 'x' || Key == 'X')
		exit(0);

	if (Key == 'w' || Key == 'W')
		StepForward(&camera);
	//printf("\nPosition: (%f, %f, %f)\n",camera.Position[0],camera.Position[1],camera.Position[2]);
	//printf("View: (%f, %f, %f)\n", camera.View[0], camera.View[1], camera.View[2]);
	//printf("Up: (%f, %f, %f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);

	//camera.Position[2] += exp((float)camera.SpeedLevel);

	if (Key == 's' || Key == 'S')
		StepBack(&camera);
	//	printf("\nPosition: (%f, %f, %f)\n",camera.Position[0],camera.Position[1],camera.Position[2]);
	//printf("View: (%f, %f, %f)\n", camera.View[0], camera.View[1], camera.View[2]);
	//printf("Up: (%f, %f, %f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);}
	//camera.Position[2] -= exp((float)camera.SpeedLevel);

	if (Key == 'd' || Key == 'D')
		StepRight(&camera);
	//	printf("\nPosition: (%f, %f, %f)\n",camera.Position[0],camera.Position[1],camera.Position[2]);
	//printf("View: (%f, %f, %f)\n", camera.View[0], camera.View[1], camera.View[2]);
	//printf("Up: (%f, %f, %f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);}

	//glTranslatef(-exp((float)camera.SpeedLevel), 0, 0);

	if (Key == 'a' || Key == 'A')
		StepLeft(&camera);
	//	printf("\nPosition: (%f, %f, %f)\n",camera.Position[0],camera.Position[1],camera.Position[2]);
	//printf("View: (%f, %f, %f)\n", camera.View[0], camera.View[1], camera.View[2]);
	//printf("Up: (%f, %f, %f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);}

	//glTranslatef(exp((float)camera.SpeedLevel), 0, 0);

	if (Key == 'q' || Key == 'Q')
		StepUp(&camera);
	//	printf("\nPosition: (%f, %f, %f)\n",camera.Position[0],camera.Position[1],camera.Position[2]);
	//printf("View: (%f, %f, %f)\n", camera.View[0], camera.View[1], camera.View[2]);
	//printf("Up: (%f, %f, %f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);}
	//glTranslatef(0, exp((float)camera.SpeedLevel), 0);

	if (Key == 'e' || Key == 'E')
		StepDown(&camera);
	//	printf("\nPosition: (%f, %f, %f)\n",camera.Position[0],camera.Position[1],camera.Position[2]);
	//printf("View: (%f, %f, %f)\n", camera.View[0], camera.View[1], camera.View[2]);
	//printf("Up: (%f, %f, %f)\n", camera.Up[0], camera.Up[1], camera.Up[2]);}

	//glTranslatef(0, -exp((float)camera.SpeedLevel), 0);

	if (Key == '+') {
		camera.SpeedLevel++;
		printf("Step is %f (level:%d).\n", exp(log(1.2) * camera.SpeedLevel),
				camera.SpeedLevel);
	}

	if (Key == '-') {
		camera.SpeedLevel--;
		printf("Step is %f (level:%d).\n", exp(log(1.2) * camera.SpeedLevel),
				camera.SpeedLevel);
	}

	if (Key == 'h' || Key == 'i' || Key == 'H' || Key == 'I')
		printf(instructionList);

	if (Key == 'z' || Key == 'Z')
		printf("Camera position: (%f, %f, %f)\n", camera.Position[0],
				camera.Position[1], camera.Position[2]);

	if (Key == 'v' || Key == 'V') {
		char filename[256];

                printf("Enter path to the new file:\n");
                scanf("%s", filename);
		printf("Saving file... Please Wait...\n");
                SaveOBJ(object, filename);
                printf("File saved.\n");

	}
	
	if (Key == '[') {
		printf("Add light:\n");
		printf("Enter the point of the light in standard 'Float Float Float'.\n");
		scanf("%f %f %f", &oLight.x, &oLight.y, &oLight.z);
		AddLight(oLight, object);
		printf("Light (%f, %f, %f) added.\n", oLight.x, oLight.y, oLight.z);
	}

	if (Key == ']') {
		RemoveLight(object);
		FlushTypesOfFaces(object);
		FlushSphere(object);
		printf("Light removed.\n");
	}	
	
	if (Key == 'N' || Key == 'n') {
		ObjVertex *light = GetLights(object);
		if (light)
		{
			camera.Position[0] = light->x;
			camera.Position[1] = light->y;
			camera.Position[2] = light->z;
			printf("Moved to the light (%f, %f, %f)\n", light->x, light->y, light->z);
		}
		else
		{
			printf("No lights.\n");
		}
	}
	
	if (Key == 'C' || Key == 'c') {
		if (GetLights(object))
		{
			printf("Counting first, second and double vision faces. Please wait...\n");		
			if (mesh) 
			{
				DeleteCalcMesh(mesh);
			}
		        mesh = CreateCalcMesh(ReturnObjMesh(object));
        	        GPU_example(mesh);
                	CopyResults(mesh, ReturnObjMesh(object));
			printf("Counted.\n");
		}
		else
		{
			printf("There is no lights! Please, enter light!\n");
		}
	}

	if (Key == 'M' || Key == 'm') {
                char filename[256];

                printf("Enter path to the new file:\n");
                scanf("%s", filename);
                printf("Saving file... Please Wait...\n");
		Bitmap *bitmap = CreateBMP(400,200);
                if (bitmap)
		{
			GetSphereBitmap(object, bitmap);
			SaveBMPtoFile(bitmap, filename);
			DestroyBMP(bitmap);
			printf("File saved.\n");
		}
		else printf("Error in file saving!\n");
	}
	
	if (Key == 'I'|| Key == 'i') {
		camera.View[0] = -camera.View[0];
		camera.View[1] = -camera.View[1];
		camera.View[2] = -camera.View[2];
	}
	
	glutPostRedisplay();
}

/*
 **	GLUT Callback function to deal with button presses and releases of the mouse
 it is define which button is pressed.
 */
void ButtonPress(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		mouse.CursorLastPosition[0] = x;
		mouse.CursorLastPosition[1] = y;
		if (button == GLUT_LEFT_BUTTON)
			mouse.LeftButton = True;
	} else {
		mouse.LeftButton = mouse.MiddleButton = False;
	}
	glutPostRedisplay();
}

/*
 **	GLUT Callback function to deal with mouse movement
 */
void MouseMotion(int x, int y) {
	if (mouse.LeftButton) {
		MouseMoved(&camera, x - mouse.CursorLastPosition[0],
				y - mouse.CursorLastPosition[1]);
	}
	mouse.CursorLastPosition[0] = x;
	mouse.CursorLastPosition[1] = y;
	glutPostRedisplay();
}


/* 
** procedure for load interactive graphical mode
*/

void GraphicalMode()
{
        glutInit(&Argc, Argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(640, 480);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("Space");

        glutDisplayFunc(Display);
        glutReshapeFunc(Reshape);
        glutKeyboardFunc(Keyboard);
        glutMouseFunc(ButtonPress);
        glutMotionFunc(MouseMotion);

        InitSettings();
        glutMainLoop();
}


/*
** procedure for load non-interactive calculate program mode
*/
void CalculateMode(char *output)
{	
	object = LoadOBJ(Filename);
	if (GetLights(object))
        {
		mesh = CreateCalcMesh(ReturnObjMesh(object));
		GPU_example(mesh);
		CopyResults(mesh, ReturnObjMesh(object));	
		SaveOBJ(object, output);
		exit(0);
	}
	else
	{
		printf("There is no lights! Please, enter light!\n");
		exit(1);
	}
}


int main(int argc, char **argv) {
	Argc = argc;
	Argv = argv;
	
	int i;

	//filename (key -f or --file)
	for (i=1; i<Argc; i++)
	{	
		if (( strcmp(Argv[i], "-f")==0 )||( strcmp(Argv[i], "--file")==0 ))
		{
			if (i+1<Argc) 
			{
				Filename=Argv[i+1];
			}
			else
			{ 
				printf("Error in input arguments!\n");
			}
		}
	}
	
	//calculate mode or interactive mode
	for (i=1; i<Argc; i++)
	{
		if (( strcmp(Argv[i], "-c")==0 )||( strcmp(Argv[i], "--calculate")==0 ))
		{	
			if ((i+1<Argc)&&(Filename!=NULL))
			{
				CalculateMode(Argv[i+1]);
			}
			else
			{
				printf("Error in input arguments!\n");
			}
		}
	}
	
	GraphicalMode();

	return 0;
}
