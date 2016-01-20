#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
		0,  3,  4,  5,  6,  7,  8,
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

//static double smoothstep(double edge0, double edge1, double t) {
//	t = (t - edge0) / (edge1 - edge0);
//	t = t > 1.0 ? 1.0 : t < 0.0 ? 0.0 : t;
//	return t*t*(3 - 2*t);
//}

static void initChunk(SimplexInstance *simplex[], Chunk *chunk, int x, int y, int z) {
	int i, j, k;
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j++) {
			double h1 = simplexEval(simplex[0], (x+i) / 100.0, (j+z) / 100.0) * 1.55 + 0.18;
			h1 *= h1*h1;
			double h2 = simplexEval(simplex[1], (x+i) / 15.0, (j+z) / 15.0) / 10.0;
			int h = (int) ((h1 + h2 + 2.0) * 20.0) + 55;
			for (k = 0; k < 32; k++) {
				chunk->blocks[i][j][k].id = k+y > h ? 0 : k+y < h ? 2 : 1;
			}
		}
	}
	glGenBuffers(2, chunk->buffers);
	chunk->x = x;
	chunk->y = y;
	chunk->z = z;
	chunk->verts = 0;
}

World *createWorld() {
	World *world = malloc(sizeof(World));
	world->simplex[0] = initSimplex(5);
	world->simplex[1] = initSimplex(4);
	int i, j, k;
	for (i = 0; i < CHUNK_X; i++)
		for (j = 0; j < CHUNK_Z; j++)
			for (k = 0; k < CHUNK_Y; k++)
				initChunk(world->simplex, &world->chunks[i][j][k], i*32, k*32, j*32);
	world->chunks[CHUNK_X/2][CHUNK_Z/2][3].blocks[0][11][1].id = 9;
	for (i = 0; i < CHUNK_X; i++)
		for (j = 0; j < CHUNK_Z; j++)
			for (k = 0; k < CHUNK_Y; k++)
				updateChunkVBO(world, &world->chunks[i][j][k]);
	world->player.x = WORLD_X/2;
	for (i = 0; i < WORLD_Y; i++) {
		if (getBlockDataOf(world, WORLD_X/2, i, WORLD_Z/2)->isTransparent) {
			world->player.y = i;
			break;
		}
	}
	world->player.z = WORLD_Z/2;
	return world;
}

void tickWorld(World *world, Context *context) {
	tickPlayer(world, context);
}

const BlockData *getBlockDataOf(World *world, int x, int y, int z) {
	if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y || z < 0 || z >= WORLD_Z)
		return NULL;
	return &blockdata[world->chunks[x>>5][z>>5][y>>5].blocks[x&0x1f][z&0x1f][y&0x1f].id];
}

