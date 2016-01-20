#ifndef _VOXEL_MAT4_H_
#define _VOXEL_MAT4_H_

typedef struct Mat4 {
	float m00;
	float m01;
	float m02;
	float m03;
	float m10;
	float m11;
	float m12;
	float m13;
	float m20;
	float m21;
	float m22;
	float m23;
	float m30;
	float m31;
	float m32;
	float m33;
} Mat4;

Mat4 *mat4Identity(Mat4 *dest);
Mat4 *mat4Zero(Mat4 *dest);
Mat4 *mat4Mul(Mat4 *left, Mat4 *right, Mat4 *dest);
Mat4 *mat4Rotate(Mat4 *left, float angle, float x, float y, float z, Mat4 *dest);
Mat4 *mat4Perspective(float fovy, float aspect, float near, float far, Mat4 *dest);
Mat4 *mat4Translate(Mat4 *left, float x, float y, float z, Mat4 *dest);
Mat4 *mat4LookAlong(float dirX, float dirY, float dirZ, float upX, float upY, float upZ, Mat4 *dest);

#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif
