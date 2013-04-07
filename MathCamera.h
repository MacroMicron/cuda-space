//just temp file
// it should be include in MathCamera.h

#pragma once

void Normalize(float *vector);

//		| column1[0] column2[0] |
//		| column1[1] column2[1] |
float Determinant2D(float *column1, float *column2);

//		| column1[0] column2[0] | b_column[0] |
//		| column1[1] column2[1] | b_column[1] |
void ResolveLinearSystem2DColumns(float *column1, float *column2, float *b_column, float *answer);

//		|row1[0], row1[1] | row1[2] |
//		|row2[0], row2[1] | row2[2] |
void ResolveLinearSystem2DRows(float *row1, float *row2, float *answer);

