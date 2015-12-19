#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GLEW_NO_GLU
#include <GL/glew.h>

#include "world.h"
#include "simplex.h"

const BlockData blockdata[16] = {
	{
		1, -1, -1, -1, -1, -1, -1,
	},
	{
		0,  2,  0,  1,  1,  1,  1,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  3,  3,  3,  3,  3,  3,
	},
	{
		0,  4,  4,  4,  4,  4,  4,
	},
	{
		0,  5,  5,  5,  5,  5,  5,
	},
	{
		0,  6,  6,  6,  6,  6,  6,
	},
	{
		0,  7,  7,  7,  7,  7,  7,
	},
	{
		0,  8,  8,  8,  8,  8,  8,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
	{
		0,  0,  0,  0,  0,  0,  0,
	},
};

static double smoothstep(double edge0, double edge1, double t) {
	t = (t - edge0) / (edge1 - edge0);
	t = t > 1.0 ? 1.0 : t < 0.0 ? 0.0 : t;
	return t*t*(3 - 2*t);
}

static void initChunk(SimplexInstance *simplex[], Chunk *chunk, int x, int y, int z) {
	int i, j, k;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			for (k = 0; k < 16; k++) {
				double h1 = simplexEval(simplex[0], (x+i) / 100.0, (k+z) / 100.0) * 1.55 + 0.18;
				h1 *= h1*h1;
				double h2 = simplexEval(simplex[1], (x+i) / 15.0, (k+z) / 15.0) / 10.0;
				int h = (int) ((h1 + h2 + 2.0) * 20.0) + 55;
				chunk->blocks[i][j][k].id = j+y > h ? 0 : j+y < h ? 2 : 1;
			}
		}
	}
	glGenBuffers(1, &chunk->vbo);
	chunk->x = x;
	chunk->y = y;
	chunk->z = z;
	chunk->verts = 0;
}

World *createWorld() {
	World *world = malloc(sizeof(World));
	world->simplex[0] = initSimplex(8);
	world->simplex[1] = initSimplex(3);
	int i, j, k;
	for (i = 0; i < 64; i++)
		for (j = 0; j < 16; j++)
			for (k = 0; k < 64; k++)
				initChunk(world->simplex, &world->chunks[i][j][k], i*16, j*16, k*16);
	for (i = 0; i < 64; i++)
		for (j = 0; j < 16; j++)
			for (k = 0; k < 64; k++)
				updateChunkVBO(world, &world->chunks[i][j][k]);
	world->player.x = 256.25;
	for (i = 0; i < 256; i++) {
		if (getBlockDataOf(world, 256, i, 256)->isTransparent) {
			world->player.y = i;
			break;
		}
	}
	world->player.z = 256.25;
	return world;
}

void tickWorld(World *world, Context *context) {
	tickPlayer(world, context);
}

const BlockData *getBlockDataOf(World *world, int x, int y, int z) {
	if (x < 0 || x >= 1024 || y < 0 || y >= 256 || z < 0 || z >= 1024)
		return NULL;
	return &blockdata[world->chunks[x>>4][y>>4][z>>4].blocks[x&0xf][y&0xf][z&0xf].id];
}

