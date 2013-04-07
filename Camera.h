//UserInterface - is implementation on OpenGl some of interface feachers:
//camera, keyboard and mouse reactions, display, etc
//Dependences: glut.h, glu.h, gl.h (OpenGL uses)

#ifndef _CAMERA_H_
#define _CAMERA_H_

#ifdef __cplusplus
extern "C" {
#endif

	//Camera must be initialized before using: View != Position
	typedef struct  {
		float Position[3],		//it is point - where the hero is situated?
			  View[3],			//it is vector - where the hero is looks?
			  Up[3];			//it is vector - from down to up
		int SpeedLevel;			//-inf, ..., -1, 0, 1, ..., +inf  = it is a step length (in levels)
		int SpeedRotation[2];   //-inf, ..., -1, 0, 1, ..., +inf  = it is a angle of rotation in mousemotion (in levels); [0] - for X, [1] - for Y
	} Camera;

	//if the keyboard was presed
	void StepForward(Camera* camera);
	void StepBack(Camera* camera);
	void StepLeft(Camera* camera);
	void StepRight(Camera* camera);
	void StepUp(Camera* camera);
	void StepDown(Camera* camera);

	//if the mouse was moved: deltaX>0 => lookAtDown, deltaX<0 => lookAtUp, deltaY>0 => lookAtLeft, deltaY<0 => lookAtRight
	void MouseMoved(Camera* pCamera, float deltaX, float deltaY);


#ifdef __cplusplus
}
#endif

#endif


