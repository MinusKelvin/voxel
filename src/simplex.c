#include <stdlib.h>

#include "simplex.h"

// C port of OpenSimplex Noise. Only includes 2D noise, because that's all we need.

static const double STRETCH_CONSTANT_2D = -0.211324865405187;    //(1/Math.sqrt(2+1)-1)/2;
static const double SQUISH_CONSTANT_2D = 0.366025403784439;      //(Math.sqrt(2+1)-1)/2;
static const double NORM_CONSTANT_2D = 47;

static char gradients2D[] = {
	 5,  2,  2,  5,
	-5,  2, -2,  5,
	 5, -2,  2, -5,
	-5, -2, -2, -5,
};

//Generates a proper permutation (i.e. doesn't merely perform N successive pair swaps on a base array)
//Uses a simple 64-bit LCG.
SimplexInstance *initSimplex(long long seed) {
	SimplexInstance *simplex = malloc(sizeof(SimplexInstance));
	short source[256];
	int i;
	for (i = 0; i < 256; i++)
		source[i] = i;
	seed = seed * 6364136223846793005l + 1442695040888963407l;
	seed = seed * 6364136223846793005l + 1442695040888963407l;
	seed = seed * 6364136223846793005l + 1442695040888963407l;
	for (i = 255; i >= 0; i--) {
		seed = seed * 6364136223846793005l + 1442695040888963407l;
		int r = ((seed + 31) % (i + 1));
		if (r < 0)
			r += i + 1;
		simplex->perm[i] = source[r];
		source[r] = source[i];
	}
	return simplex;
}

static int fastFloor(double v) {
	int vi = (int) v;
	return v < vi ? vi - 1 : vi;
}

static double extrapolate(SimplexInstance *simplex, int xsb, int ysb, double dx, double dy) {
	int index = simplex->perm[(simplex->perm[xsb & 0xff] + ysb) & 0xff] & 0x0e;
	return gradients2D[index] * dx + gradients2D[index + 1] * dy;
}

//2D OpenSimplex Noise.
double simplexEval(SimplexInstance *simplex, double x, double y) {
	//Place input coordinates onto grid.
	double stretchOffset = (x + y) * STRETCH_CONSTANT_2D;
	double xs = x + stretchOffset;
	double ys = y + stretchOffset;
	
	//Floor to get grid coordinates of rhombus (stretched square) super-cell origin.
	int xsb = fastFloor(xs);
	int ysb = fastFloor(ys);
	
	//Skew out to get actual coordinates of rhombus origin. We'll need these later.
	double squishOffset = (xsb + ysb) * SQUISH_CONSTANT_2D;
	double xb = xsb + squishOffset;
	double yb = ysb + squishOffset;
	
	//Compute grid coordinates relative to rhombus origin.
	double xins = xs - xsb;
	double yins = ys - ysb;
	
	//Sum those together to get a value that determines which region we're in.
	double inSum = xins + yins;

	//Positions relative to origin point.
	double dx0 = x - xb;
	double dy0 = y - yb;
	
	//We'll be defining these inside the next block and using them afterwards.
	double dx_ext, dy_ext;
	int xsv_ext, ysv_ext;
	
	double value = 0;

	//Contribution (1,0)
	double dx1 = dx0 - 1 - SQUISH_CONSTANT_2D;
	double dy1 = dy0 - 0 - SQUISH_CONSTANT_2D;
	double attn1 = 2 - dx1 * dx1 - dy1 * dy1;
	if (attn1 > 0) {
		attn1 *= attn1;
		value += attn1 * attn1 * extrapolate(simplex, xsb + 1, ysb + 0, dx1, dy1);
	}

	//Contribution (0,1)
	double dx2 = dx0 - 0 - SQUISH_CONSTANT_2D;
	double dy2 = dy0 - 1 - SQUISH_CONSTANT_2D;
	double attn2 = 2 - dx2 * dx2 - dy2 * dy2;
	if (attn2 > 0) {
		attn2 *= attn2;
		value += attn2 * attn2 * extrapolate(simplex, xsb + 0, ysb + 1, dx2, dy2);
	}
	
	if (inSum <= 1) { //We're inside the triangle (2-Simplex) at (0,0)
		double zins = 1 - inSum;
		if (zins > xins || zins > yins) { //(0,0) is one of the closest two triangular vertices
			if (xins > yins) {
				xsv_ext = xsb + 1;
				ysv_ext = ysb - 1;
				dx_ext = dx0 - 1;
				dy_ext = dy0 + 1;
			} else {
				xsv_ext = xsb - 1;
				ysv_ext = ysb + 1;
				dx_ext = dx0 + 1;
				dy_ext = dy0 - 1;
			}
		} else { //(1,0) and (0,1) are the closest two vertices.
			xsv_ext = xsb + 1;
			ysv_ext = ysb + 1;
			dx_ext = dx0 - 1 - 2 * SQUISH_CONSTANT_2D;
			dy_ext = dy0 - 1 - 2 * SQUISH_CONSTANT_2D;
		}
	} else { //We're inside the triangle (2-Simplex) at (1,1)
		double zins = 2 - inSum;
		if (zins < xins || zins < yins) { //(0,0) is one of the closest two triangular vertices
			if (xins > yins) {
				xsv_ext = xsb + 2;
				ysv_ext = ysb + 0;
				dx_ext = dx0 - 2 - 2 * SQUISH_CONSTANT_2D;
				dy_ext = dy0 + 0 - 2 * SQUISH_CONSTANT_2D;
			} else {
				xsv_ext = xsb + 0;
				ysv_ext = ysb + 2;
				dx_ext = dx0 + 0 - 2 * SQUISH_CONSTANT_2D;
				dy_ext = dy0 - 2 - 2 * SQUISH_CONSTANT_2D;
			}
		} else { //(1,0) and (0,1) are the closest two vertices.
			dx_ext = dx0;
			dy_ext = dy0;
			xsv_ext = xsb;
			ysv_ext = ysb;
		}
		xsb += 1;
		ysb += 1;
		dx0 = dx0 - 1 - 2 * SQUISH_CONSTANT_2D;
		dy0 = dy0 - 1 - 2 * SQUISH_CONSTANT_2D;
	}
	
	//Contribution (0,0) or (1,1)
	double attn0 = 2 - dx0 * dx0 - dy0 * dy0;
	if (attn0 > 0) {
		attn0 *= attn0;
		value += attn0 * attn0 * extrapolate(simplex, xsb, ysb, dx0, dy0);
	}
	
	//Extra Vertex
	double attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
	if (attn_ext > 0) {
		attn_ext *= attn_ext;
		value += attn_ext * attn_ext * extrapolate(simplex, xsv_ext, ysv_ext, dx_ext, dy_ext);
	}
	
	return value / NORM_CONSTANT_2D;
}
