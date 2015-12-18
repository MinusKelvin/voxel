#ifndef _VOXEL_SIMPLEX_H_
#define _VOXEL_SIMPLEX_H_

typedef struct SimplexInstance {
	short perm[256];
} SimplexInstance;

SimplexInstance *initSimplex(long long seed);
double simplexEval(SimplexInstance *simplex, double x, double y);

#endif
