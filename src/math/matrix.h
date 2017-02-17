#ifndef HOHEX_MATRIX_H
#define HOHEX_MATRIX_H
#include "vector.h"

typedef struct{
  float matrix[4][4];
}mat4;

mat4 make_ortho(float left, float right, float bottom, float top)
{
	mat4 result;
	result.matrix[0][0] = 2.0f / (right - left);	result.matrix[0][1] = 0;						result.matrix[0][2] = 0;		result.matrix[0][3] = -((right + left) / (right - left));
	result.matrix[1][0] = 0;						result.matrix[1][1] = 2.0f / (top - bottom);	result.matrix[1][2] = 0;		result.matrix[1][3] = -((top + bottom) / (top - bottom));
	result.matrix[2][0] = 0;						result.matrix[2][1] = 0;						result.matrix[2][2] = -1.0f;	result.matrix[2][3] = 0;
	result.matrix[3][0] = 0;						result.matrix[3][1] = 0;						result.matrix[3][2] = 0;		result.matrix[3][3] = 1;
	return result;
}

mat4 make_ident()
{
  mat4 result;
  result.matrix[0][0] = 1; result.matrix[0][1] = 0; result.matrix[0][2] = 0; result.matrix[0][3] = 0;
  result.matrix[1][0] = 0; result.matrix[1][1] = 1; result.matrix[1][2] = 0; result.matrix[1][3] = 0;
  result.matrix[2][0] = 0; result.matrix[2][1] = 0; result.matrix[2][2] = 1; result.matrix[2][3] = 0;
  result.matrix[3][0] = 0; result.matrix[3][1] = 0; result.matrix[3][2] = 0; result.matrix[3][3] = 1;
  return result;
}

mat4 translate(vec3 pos)
{
	mat4 result;
	result.matrix[0][0] = 1; result.matrix[0][1] = 0; result.matrix[0][2] = 0; result.matrix[0][3] = pos.x;
	result.matrix[1][0] = 0; result.matrix[1][1] = 1; result.matrix[1][2] = 0; result.matrix[1][3] = pos.y;
	result.matrix[2][0] = 0; result.matrix[2][1] = 0; result.matrix[2][2] = 1; result.matrix[2][3] = pos.z;
	result.matrix[3][0] = 0; result.matrix[3][1] = 0; result.matrix[3][2] = 0; result.matrix[3][3] = 1;
	return result;
}

mat4 scale(float f)
{
	mat4 result;
	result.matrix[0][0] = f; result.matrix[0][1] = 0; result.matrix[0][2] = 0; result.matrix[0][3] = 0;
	result.matrix[1][0] = 0; result.matrix[1][1] = f; result.matrix[1][2] = 0; result.matrix[1][3] = 0;
	result.matrix[2][0] = 0; result.matrix[2][1] = 0; result.matrix[2][2] = f; result.matrix[2][3] = 0;
	result.matrix[3][0] = 0; result.matrix[3][1] = 0; result.matrix[3][2] = 0; result.matrix[3][3] = 1;
	return result;
}

#endif // HOHEX_MATRIX_H
