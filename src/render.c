#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GLEW_NO_GLU
#include <GL/glew.h>

#include "world.h"

static void renderChunk(Chunk *chunk) {
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
	if (!chunk->verts)
		return;
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, chunk->verts);
}

void renderWorld(World *world, Context *context) {
	Mat4 transform;
	mat4Identity(&transform);
	mat4Rotate(&transform, world->player.yaw, 0, 1, 0, &transform);
	mat4Rotate(&transform, world->player.pitch, cos(world->player.yaw), 0, sin(world->player.yaw), &transform);
	mat4Translate(&transform, -0.25-world->player.x, -1.7-world->player.y, -0.25-world->player.z, &transform);
	mat4Mul(&context->perspective, &transform, &transform);

	glUniformMatrix4fv(context->shaders->basicProjLoc, 1, GL_FALSE, (GLfloat*) &transform);

	int i, j, k;
	for (i = 0; i < 64; i++)
		for (j = 0; j < 16; j++)
			for (k = 0; k < 64; k++)
				renderChunk(&world->chunks[i][j][k]);
}

static void buildWestFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildEastFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildNorthFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildSouthFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildTopFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildBottomFace(float *mem, int *size, int *verts, float x, float y, float z, float id);

void updateChunkVBO(World *world, Chunk *chunk) {
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
	float *mem = malloc(16*16*16*48*3*4);
	int size = 0;
	chunk->verts = 0;

	int i,j,k;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			for (k = 0; k < 16; k++) {
				Block *block = &chunk->blocks[i][j][k];
				if (!blockdata[block->id].isTransparent) {
					const BlockData *bd = getBlockDataOf(world, i+chunk->x, j+chunk->y, k+chunk->z-1);
					if (bd != NULL && bd->isTransparent)
						buildNorthFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].southTextureID);

					bd = getBlockDataOf(world, i+chunk->x, j+chunk->y, k+chunk->z+1);
					if (bd != NULL && bd->isTransparent)
						buildSouthFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].northTextureID);

					bd = getBlockDataOf(world, i+chunk->x-1, j+chunk->y, k+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildWestFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].westTextureID);

					bd = getBlockDataOf(world, i+chunk->x+1, j+chunk->y, k+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildEastFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].eastTextureID);

					bd = getBlockDataOf(world, i+chunk->x, j+chunk->y-1, k+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildBottomFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].bottomTextureID);

					bd = getBlockDataOf(world, i+chunk->x, j+chunk->y+1, k+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildTopFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].topTextureID);
				}
			}
		}
	}
	glBufferData(GL_ARRAY_BUFFER, size*4, (GLvoid*) mem, GL_STATIC_DRAW);

	free(mem);
}


static void buildWestFace(float *mem, int *size, int *verts, float x, float y, float z, float id) {
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;

	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*verts += 6;
}

static void buildEastFace(float *mem, int *size, int *verts, float x, float y, float z, float id) {
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;

	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*verts += 6;
}

static void buildNorthFace(float *mem, int *size, int *verts, float x, float y, float z, float id) {
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;

	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*verts += 6;
}

static void buildSouthFace(float *mem, int *size, int *verts, float x, float y, float z, float id) {
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;

	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*verts += 6;
}

static void buildTopFace(float *mem, int *size, int *verts, float x, float y, float z, float id) {
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;

	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y + 1.0;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*verts += 6;
}

static void buildBottomFace(float *mem, int *size, int *verts, float x, float y, float z, float id) {
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;

	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z;
	*(mem + (*size)++) = id;
	*(mem + (*size)++) = x + 1.0;
	*(mem + (*size)++) = y;
	*(mem + (*size)++) = z + 1.0;
	*(mem + (*size)++) = id;
	*verts += 6;
}
