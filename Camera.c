//UserIntBerface - is implementation on OpenGl some of interface feachers:
//camera, keyboard and mouse reactions, display, etc
//Dependences: glut.h, glu.h, gl.h (OpenGL uses)


#include <assert.h>
#include <math.h>

#include "MathCamera.h"
#include "Camera.h"


//normalize (vector = View - Position vectors)
float vectorDirection[3];
//temp value for all procedures
int i;

//step by vector (float[3])
//vector must be normalized: ||vector|| == 1
void StepByVector(Camera* pCamera, const float *vector)
{
	for (i=0; i<3; i++)
	{
		pCamera->Position[i] += exp(log(1.2)*pCamera->SpeedLevel) * vector[i];
	}
}


void StepForward(Camera* pCamera)
{
	StepByVector(pCamera, pCamera->View);
}


void StepBack(Camera* pCamera)
{
	for (i=0; i<3; i++) {
		vectorDirection[i] = (-1) * pCamera->View[i];
	}
	Normalize(vectorDirection);
	StepByVector(pCamera, vectorDirection);
}

void StepRight(Camera* pCamera)
{
	vectorDirection[0] = pCamera->View[1] * pCamera->Up[2] - pCamera->View[2] * pCamera->Up[1];
	vectorDirection[1] = - pCamera->View[0] * pCamera->Up[2] + pCamera->View[2] * pCamera->Up[0];
	vectorDirection[2] = pCamera->View[0] * pCamera->Up[1] + pCamera->View[1] * pCamera->Up[0];
	Normalize(vectorDirection);
	StepByVector(pCamera, vectorDirection);
}

void StepLeft(Camera* pCamera)
{
	vectorDirection[0] = (-1) * (pCamera->View[1] * pCamera->Up[2] - pCamera->View[2] * pCamera->Up[1]);
	vectorDirection[1] = (-1) * (- pCamera->View[0] * pCamera->Up[2] + pCamera->View[2] * pCamera->Up[0]);
	vectorDirection[2] = (-1) * (pCamera->View[0] * pCamera->Up[1] + pCamera->View[1] * pCamera->Up[0]);
	Normalize(vectorDirection);
	StepByVector(pCamera, vectorDirection);
}

void StepUp(Camera* pCamera)
{
	StepByVector(pCamera, pCamera->Up);
}

void StepDown(Camera* pCamera)
{
	for (i=0; i<3; i++) {
		vectorDirection[i] = (-1) * pCamera->Up[i];
	}
	Normalize(vectorDirection);
	StepByVector(pCamera, vectorDirection);
}

//if the mouse was moved
void MouseMoved(Camera* pCamera, float deltaX, float deltaY)
{
	pCamera->View[0] += 0.01*exp(log(1.1)*pCamera->SpeedRotation[0])*deltaX;
	pCamera->View[1] += 0.01*exp(log(1.1)*pCamera->SpeedRotation[1])*(-1)*deltaY;
}
