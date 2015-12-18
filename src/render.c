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
	for (i = 0; i < 16; i++)
		for (j = 0; j < 16; j++)
			for (k = 0; k < 16; k++)
				renderChunk(&world->chunks[i][j][k]);
}

static void buildWestFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildEastFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildNorthFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildSouthFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildTopFace(float *mem, int *size, int *verts, float x, float y, float z, float id);
static void buildBottomFace(float *mem, int *size, int *verts, float x, float y, float z, float id);

void updateChunkVBO(Chunk *chunk) {
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
					if (k == 0 || blockdata[chunk->blocks[i][j][k-1].id].isTransparent)
						buildNorthFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].southTextureID);

					if (k == 15 || blockdata[chunk->blocks[i][j][k+1].id].isTransparent)
						buildSouthFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].northTextureID);

					if (i == 0 || blockdata[chunk->blocks[i-1][j][k].id].isTransparent)
						buildWestFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].westTextureID);

					if (i == 15 || blockdata[chunk->blocks[i+1][j][k].id].isTransparent)
						buildEastFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].eastTextureID);

					if (j == 0 || blockdata[chunk->blocks[i][j-1][k].id].isTransparent)
						buildBottomFace(mem, &size, &chunk->verts, i+chunk->x, j+chunk->y, k+chunk->z, blockdata[block->id].bottomTextureID);

					if (j == 15 || blockdata[chunk->blocks[i][j+1][k].id].isTransparent)
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
