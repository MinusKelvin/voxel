#include <math.h>
#include "mat4.h"

Mat4 *mat4Identity(Mat4 *dest) {
	dest->m00 = 1.0;
	dest->m01 = 0.0;
	dest->m02 = 0.0;
	dest->m03 = 0.0;
	dest->m10 = 0.0;
	dest->m11 = 1.0;
	dest->m12 = 0.0;
	dest->m13 = 0.0;
	dest->m20 = 0.0;
	dest->m21 = 0.0;
	dest->m22 = 1.0;
	dest->m23 = 0.0;
	dest->m30 = 0.0;
	dest->m31 = 0.0;
	dest->m32 = 0.0;
	dest->m33 = 1.0;
	return dest;
}

Mat4 *mat4Zero(Mat4 *dest) {
	dest->m00 = 0.0;
	dest->m01 = 0.0;
	dest->m02 = 0.0;
	dest->m03 = 0.0;
	dest->m10 = 0.0;
	dest->m11 = 0.0;
	dest->m12 = 0.0;
	dest->m13 = 0.0;
	dest->m20 = 0.0;
	dest->m21 = 0.0;
	dest->m22 = 0.0;
	dest->m23 = 0.0;
	dest->m30 = 0.0;
	dest->m31 = 0.0;
	dest->m32 = 0.0;
	dest->m33 = 0.0;
	return dest;
}

Mat4 *mat4Mul(Mat4 *left, Mat4 *right, Mat4 *dest) {
	float m00 = left->m00 * right->m00 + left->m10 * right->m01 + left->m20 * right->m02 + left->m30 * right->m03;
	float m01 = left->m01 * right->m00 + left->m11 * right->m01 + left->m21 * right->m02 + left->m31 * right->m03;
	float m02 = left->m02 * right->m00 + left->m12 * right->m01 + left->m22 * right->m02 + left->m32 * right->m03;
	float m03 = left->m03 * right->m00 + left->m13 * right->m01 + left->m23 * right->m02 + left->m33 * right->m03;
	float m10 = left->m00 * right->m10 + left->m10 * right->m11 + left->m20 * right->m12 + left->m30 * right->m13;
	float m11 = left->m01 * right->m10 + left->m11 * right->m11 + left->m21 * right->m12 + left->m31 * right->m13;
	float m12 = left->m02 * right->m10 + left->m12 * right->m11 + left->m22 * right->m12 + left->m32 * right->m13;
	float m13 = left->m03 * right->m10 + left->m13 * right->m11 + left->m23 * right->m12 + left->m33 * right->m13;
	float m20 = left->m00 * right->m20 + left->m10 * right->m21 + left->m20 * right->m22 + left->m30 * right->m23;
	float m21 = left->m01 * right->m20 + left->m11 * right->m21 + left->m21 * right->m22 + left->m31 * right->m23;
	float m22 = left->m02 * right->m20 + left->m12 * right->m21 + left->m22 * right->m22 + left->m32 * right->m23;
	float m23 = left->m03 * right->m20 + left->m13 * right->m21 + left->m23 * right->m22 + left->m33 * right->m23;
	float m30 = left->m00 * right->m30 + left->m10 * right->m31 + left->m20 * right->m32 + left->m30 * right->m33;
	float m31 = left->m01 * right->m30 + left->m11 * right->m31 + left->m21 * right->m32 + left->m31 * right->m33;
	float m32 = left->m02 * right->m30 + left->m12 * right->m31 + left->m22 * right->m32 + left->m32 * right->m33;
	float m33 = left->m03 * right->m30 + left->m13 * right->m31 + left->m23 * right->m32 + left->m33 * right->m33;
	dest->m00 = m00;
	dest->m01 = m01;
	dest->m02 = m02;
	dest->m03 = m03;
	dest->m10 = m10;
	dest->m11 = m11;
	dest->m12 = m12;
	dest->m13 = m13;
	dest->m20 = m20;
	dest->m21 = m21;
	dest->m22 = m22;
	dest->m23 = m23;
	dest->m30 = m30;
	dest->m31 = m31;
	dest->m32 = m32;
	dest->m33 = m33;
	return dest;
}

Mat4 *mat4Rotate(Mat4 *left, float angle, float x, float y, float z, Mat4 *dest) {
	float s = (float) sin(angle);
	float c = (float) cos(angle);
	float C = 1.0 - c;

	float xx = x * x;
	float xy = x * y;
	float xz = x * z;
	float yy = y * y;
	float yz = y * z;
	float zz = z * z;
	float rm00 = xx * C + c;
	float rm01 = xy * C + z * s;
	float rm02 = xz * C - y * s;
	float rm10 = xy * C - z * s;
	float rm11 = yy * C + c;
	float rm12 = yz * C + x * s;
	float rm20 = xz * C + y * s;
	float rm21 = yz * C - x * s;
	float rm22 = zz * C + c;

	float nm00 = left->m00 * rm00 + left->m10 * rm01 + left->m20 * rm02;
	float nm01 = left->m01 * rm00 + left->m11 * rm01 + left->m21 * rm02;
	float nm02 = left->m02 * rm00 + left->m12 * rm01 + left->m22 * rm02;
	float nm03 = left->m03 * rm00 + left->m13 * rm01 + left->m23 * rm02;
	float nm10 = left->m00 * rm10 + left->m10 * rm11 + left->m20 * rm12;
	float nm11 = left->m01 * rm10 + left->m11 * rm11 + left->m21 * rm12;
	float nm12 = left->m02 * rm10 + left->m12 * rm11 + left->m22 * rm12;
	float nm13 = left->m03 * rm10 + left->m13 * rm11 + left->m23 * rm12;
	dest->m20 = left->m00 * rm20 + left->m10 * rm21 + left->m20 * rm22;
	dest->m21 = left->m01 * rm20 + left->m11 * rm21 + left->m21 * rm22;
	dest->m22 = left->m02 * rm20 + left->m12 * rm21 + left->m22 * rm22;
	dest->m23 = left->m03 * rm20 + left->m13 * rm21 + left->m23 * rm22;
	dest->m00 = nm00;
	dest->m01 = nm01;
	dest->m02 = nm02;
	dest->m03 = nm03;
	dest->m10 = nm10;
	dest->m11 = nm11;
	dest->m12 = nm12;
	dest->m13 = nm13;
	dest->m30 = left->m30;
	dest->m31 = left->m31;
	dest->m32 = left->m32;
	dest->m33 = left->m33;

	return dest;
}

Mat4 *mat4Perspective(float fovy, float aspect, float near, float far, Mat4 *dest) {
	float h = (float) tan(fovy * 0.5) * near;
	float w = h * aspect;
	dest->m00 = near / w;
	dest->m01 = 0.0;
	dest->m02 = 0.0;
	dest->m03 = 0.0;
	dest->m10 = 0.0;
	dest->m11 = near / h;
	dest->m12 = 0.0;
	dest->m13 = 0.0;
	dest->m20 = 0.0;
	dest->m21 = 0.0;
	dest->m22 = -(far + near) / (far - near);
	dest->m23 = -1.0;
	dest->m30 = 0.0;
	dest->m31 = 0.0;
	dest->m32 = -2.0 * far * near / (far - near);
	dest->m33 = 0.0;
	return dest;
}

Mat4 *mat4Translate(Mat4 *left, float x, float y, float z, Mat4 *dest) {
	dest->m00 = left->m00;
	dest->m01 = left->m01;
	dest->m02 = left->m02;
	dest->m03 = left->m03;
	dest->m10 = left->m10;
	dest->m11 = left->m11;
	dest->m12 = left->m12;
	dest->m13 = left->m13;
	dest->m20 = left->m20;
	dest->m21 = left->m21;
	dest->m22 = left->m22;
	dest->m23 = left->m23;
	dest->m30 = left->m00 * x + left->m10 * y + left->m20 * z + left->m30;
	dest->m31 = left->m01 * x + left->m11 * y + left->m21 * z + left->m31;
	dest->m32 = left->m02 * x + left->m12 * y + left->m22 * z + left->m32;
	dest->m33 = left->m03 * x + left->m13 * y + left->m23 * z + left->m33;
	return dest;
}
