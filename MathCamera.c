#include <assert.h>
#include <math.h>
#include "MathCamera.h"



void Normalize(float *vector)
{
	float temp = vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2];
	assert(temp);
	vector[0] = vector[0] / sqrt(temp);
	vector[1] = vector[1] / sqrt(temp);
	vector[2] = vector[2] / sqrt(temp);
}

float Determinant2D(float *column1, float *column2)
{
	return column1[0] * column2[1] - column2[0] * column1[1];
}

void ResolveLinearSystem2DColumns(float *column1, float *column2, float *b_column, float *answer)
{
	float det = Determinant2D(column1, column2);
	assert(det);
	answer[0] = Determinant2D(b_column, column2) / det;
	answer[1] = Determinant2D(column1, b_column) / det;
}

void ResolveLinearSystem2DRows(float *row1, float *row2, float *answer)
{
	float b_column[2];
	b_column[0] = row1[2];
	b_column[1] = row1[2];
	ResolveLinearSystem2DColumns(row1, row2, b_column, answer);
}
