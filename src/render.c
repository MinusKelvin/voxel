#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "world.h"

static void renderChunk(Chunk *chunk) {
	if (!chunk->verts)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, chunk->buffers[VERTEX_BUF]);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 44, (GLvoid*) 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 44, (GLvoid*) 12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 44, (GLvoid*) 24);
	glVertexAttribPointer(3, 2, GL_UNSIGNED_SHORT, GL_FALSE, 44, (GLvoid*) 36);
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, 44, (GLvoid*) 40);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glDrawArrays(GL_TRIANGLES, 0, chunk->verts);
}

static void renderShadowChunk(Chunk *chunk) {
	if (!chunk->verts)
		return;

	glBindBuffer(GL_ARRAY_BUFFER, chunk->buffers[VERTEX_BUF]);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 44, (GLvoid*) 0);
	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);

	glDrawArrays(GL_TRIANGLES, 0, chunk->verts);
}

void renderWorld(World *world, Context *context) {
	glUseProgram(context->shaders->shadow);
	glViewport(0,0,8192,8192);
	glDepthFunc(GL_GREATER);

	Mat4 shadowProj, transform;
	mat4LookAlong(context->sunx, -context->suny, context->sunz, 0, 1, 0, &transform);
	mat4Translate(&transform, ((int) (-0.25-world->player.x)/32)*32-16, 0, ((int) (-0.25-world->player.z)/32)*32-16, &transform);
	mat4Identity(&shadowProj);
	shadowProj.m00 = 1/128.0;
	shadowProj.m11 = 1/128.0;
	shadowProj.m22 = 1/512.0;
	mat4Mul(&shadowProj, &transform, &shadowProj);

	glUniformMatrix4fv(context->shaders->shadowProjLoc, 1, GL_FALSE, (GLfloat*) &shadowProj);

	glCullFace(GL_FRONT);

	int i, j, k;
	for (i = max((int) (world->player.x-128) / 32, 0); i < min((int) (world->player.x+128) / 32, CHUNK_X); i++)
		for (j = max((int) (world->player.z-128) / 32, 0); j < min((int) (world->player.z+128) / 32, CHUNK_X); j++)
			for (k = 0; k < CHUNK_Y; k++)
				renderShadowChunk(&world->chunks[i][j][k]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(context->shaders->basic);
	glViewport(0,0,context->width, context->height);
	glDepthFunc(GL_LEQUAL);
	glUniform4f(context->shaders->basicSundirLoc, context->sunx, context->suny, context->sunz, 1);

	mat4Identity(&transform);
	mat4Rotate(&transform, world->player.yaw, 0, 1, 0, &transform);
	mat4Rotate(&transform, world->player.pitch, cos(world->player.yaw), 0, sin(world->player.yaw), &transform);
	mat4Translate(&transform, -0.25-world->player.x, -1.7-world->player.y, -0.25-world->player.z, &transform);
	mat4Mul(&context->perspective, &transform, &transform);

	glUniformMatrix4fv(context->shaders->basicProjLoc, 1, GL_FALSE, (GLfloat*) &transform);
	glUniformMatrix4fv(context->shaders->basicSProjLoc, 1, GL_FALSE, (GLfloat*) &shadowProj);

	glCullFace(GL_BACK);

	for (i = 0; i < CHUNK_X; i++)
		for (j = 0; j < CHUNK_Z; j++)
			for (k = 0; k < CHUNK_Y; k++)
				renderChunk(&world->chunks[i][j][k]);
}

static void buildWestFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id);
static void buildEastFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id);
static void buildNorthFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id);
static void buildSouthFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id);
static void buildTopFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id);
static void buildBottomFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id);

void updateChunkVBO(World *world, Chunk *chunk) {
	glBindBuffer(GL_ARRAY_BUFFER, chunk->buffers[VERTEX_BUF]);
	int vbufsize = 256;
	float *vertexmem = malloc(vbufsize*66*4);
	int vsize = 0;
	chunk->verts = 0;

	int i,j,k;
	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j++) {
			for (k = 0; k < 32; k++) {
				Block *block = &chunk->blocks[i][j][k];
				if (!blockdata[block->id].isTransparent) {
					if (vsize > vbufsize*66-66*6) {
						vbufsize *= 2;
						vertexmem = realloc(vertexmem, vbufsize*66*4);
						assert(vertexmem != NULL);
					}

					const BlockData *bd = getBlockDataOf(world, i+chunk->x, k+chunk->y, j+chunk->z-1);
					if (bd != NULL && bd->isTransparent)
						buildNorthFace(world, vertexmem, &vsize, &chunk->verts, i+chunk->x, k+chunk->y, j+chunk->z, blockdata[block->id].southTextureID);

					bd = getBlockDataOf(world, i+chunk->x, k+chunk->y, j+chunk->z+1);
					if (bd != NULL && bd->isTransparent)
						buildSouthFace(world, vertexmem, &vsize, &chunk->verts, i+chunk->x, k+chunk->y, j+chunk->z, blockdata[block->id].northTextureID);

					bd = getBlockDataOf(world, i+chunk->x-1, k+chunk->y, j+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildWestFace(world, vertexmem, &vsize, &chunk->verts, i+chunk->x, k+chunk->y, j+chunk->z, blockdata[block->id].westTextureID);

					bd = getBlockDataOf(world, i+chunk->x+1, k+chunk->y, j+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildEastFace(world, vertexmem, &vsize, &chunk->verts, i+chunk->x, k+chunk->y, j+chunk->z, blockdata[block->id].eastTextureID);

					bd = getBlockDataOf(world, i+chunk->x, k+chunk->y-1, j+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildBottomFace(world, vertexmem, &vsize, &chunk->verts, i+chunk->x, k+chunk->y, j+chunk->z, blockdata[block->id].bottomTextureID);

					bd = getBlockDataOf(world, i+chunk->x, k+chunk->y+1, j+chunk->z);
					if (bd != NULL && bd->isTransparent)
						buildTopFace(world, vertexmem, &vsize, &chunk->verts, i+chunk->x, k+chunk->y, j+chunk->z, blockdata[block->id].topTextureID);
				}
			}
		}
	}
	//printf("%x\n", vsize*4);
	glBufferData(GL_ARRAY_BUFFER, vsize*4, (GLvoid*) vertexmem, GL_STATIC_DRAW);

	free(vertexmem);
}

static unsigned char vertexAO(int s1, int s2, int c) {
	int factor;
	if (s1 && s2)
		factor = 3;
	factor = (s1 != 0) + (s2 != 0) + (c != 0);
	return 255 - (factor * 64);
}

static void buildWestFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id) {
	unsigned char vals[4] = {255,255,255,255};
	unsigned int bitmap = 0;
	const BlockData *bd = getBlockDataOf(world, x-1, y+1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 1;
	bd = getBlockDataOf(world, x-1, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 2;
	bd = getBlockDataOf(world, x-1, y, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 4;
	bd = getBlockDataOf(world, x-1, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 8;
	bd = getBlockDataOf(world, x-1, y-1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 16;
	bd = getBlockDataOf(world, x-1, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 32;
	bd = getBlockDataOf(world, x-1, y, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 64;
	bd = getBlockDataOf(world, x-1, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 128;
	
	vals[0] = vertexAO(bitmap & 64, bitmap & 16, bitmap & 32);
	vals[1] = vertexAO(bitmap & 64, bitmap & 1, bitmap & 128);
	vals[2] = vertexAO(bitmap & 4, bitmap & 16, bitmap & 8);
	vals[3] = vertexAO(bitmap & 4, bitmap & 1, bitmap & 2);

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	unsigned short *shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 0;
	unsigned char *chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*verts += 6;
}

static void buildEastFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id) {
	unsigned char vals[4] = {255,255,255,255};
	unsigned int bitmap = 0;
	const BlockData *bd = getBlockDataOf(world, x+1, y+1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 1;
	bd = getBlockDataOf(world, x+1, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 2;
	bd = getBlockDataOf(world, x+1, y, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 4;
	bd = getBlockDataOf(world, x+1, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 8;
	bd = getBlockDataOf(world, x+1, y-1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 16;
	bd = getBlockDataOf(world, x+1, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 32;
	bd = getBlockDataOf(world, x+1, y, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 64;
	bd = getBlockDataOf(world, x+1, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 128;
	
	vals[0] = vertexAO(bitmap & 64, bitmap & 16, bitmap & 32);
	vals[1] = vertexAO(bitmap & 64, bitmap & 1, bitmap & 128);
	vals[2] = vertexAO(bitmap & 4, bitmap & 16, bitmap & 8);
	vals[3] = vertexAO(bitmap & 4, bitmap & 1, bitmap & 2);

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	unsigned short *shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 0;
	unsigned char *chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*verts += 6;
}

static void buildNorthFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id) {
	unsigned char vals[4] = {255,255,255,255};
	unsigned int bitmap = 0;
	const BlockData *bd = getBlockDataOf(world, x, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 1;
	bd = getBlockDataOf(world, x+1, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 2;
	bd = getBlockDataOf(world, x+1, y, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 4;
	bd = getBlockDataOf(world, x+1, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 8;
	bd = getBlockDataOf(world, x, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 16;
	bd = getBlockDataOf(world, x-1, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 32;
	bd = getBlockDataOf(world, x-1, y, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 64;
	bd = getBlockDataOf(world, x-1, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 128;
	
	vals[0] = vertexAO(bitmap & 64, bitmap & 16, bitmap & 32);
	vals[1] = vertexAO(bitmap & 64, bitmap & 1, bitmap & 128);
	vals[2] = vertexAO(bitmap & 4, bitmap & 16, bitmap & 8);
	vals[3] = vertexAO(bitmap & 4, bitmap & 1, bitmap & 2);

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	unsigned short *shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 0;
	unsigned char *chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*verts += 6;
}

static void buildSouthFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id) {
	unsigned char vals[4] = {255,255,255,255};
	unsigned int bitmap = 0;
	const BlockData *bd = getBlockDataOf(world, x, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 1;
	bd = getBlockDataOf(world, x-1, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 2;
	bd = getBlockDataOf(world, x-1, y, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 4;
	bd = getBlockDataOf(world, x-1, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 8;
	bd = getBlockDataOf(world, x, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 16;
	bd = getBlockDataOf(world, x+1, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 32;
	bd = getBlockDataOf(world, x+1, y, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 64;
	bd = getBlockDataOf(world, x+1, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 128;
	
	vals[0] = vertexAO(bitmap & 64, bitmap & 16, bitmap & 32);
	vals[1] = vertexAO(bitmap & 64, bitmap & 1, bitmap & 128);
	vals[2] = vertexAO(bitmap & 4, bitmap & 16, bitmap & 8);
	vals[3] = vertexAO(bitmap & 4, bitmap & 1, bitmap & 2);

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	unsigned short *shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 0;
	unsigned char *chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*verts += 6;
}

static void buildTopFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id) {
	unsigned char vals[4] = {255,255,255,255};
	unsigned int bitmap = 0;
	const BlockData *bd = getBlockDataOf(world, x, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 1;
	bd = getBlockDataOf(world, x+1, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 2;
	bd = getBlockDataOf(world, x+1, y+1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 4;
	bd = getBlockDataOf(world, x+1, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 8;
	bd = getBlockDataOf(world, x, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 16;
	bd = getBlockDataOf(world, x-1, y+1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 32;
	bd = getBlockDataOf(world, x-1, y+1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 64;
	bd = getBlockDataOf(world, x-1, y+1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 128;
	
	vals[0] = vertexAO(bitmap & 64, bitmap & 16, bitmap & 32);
	vals[1] = vertexAO(bitmap & 64, bitmap & 1, bitmap & 128);
	vals[2] = vertexAO(bitmap & 4, bitmap & 16, bitmap & 8);
	vals[3] = vertexAO(bitmap & 4, bitmap & 1, bitmap & 2);

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	unsigned short *shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 0;
	unsigned char *chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y + 1.0;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr = vals[3];

	*verts += 6;
}

static void buildBottomFace(World *world, float *vmem, int *vsize, int *verts, float x, float y, float z, float id) {
	unsigned char vals[4] = {255,255,255,255};
	unsigned int bitmap = 0;
	const BlockData *bd = getBlockDataOf(world, x, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 1;
	bd = getBlockDataOf(world, x-1, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 2;
	bd = getBlockDataOf(world, x-1, y-1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 4;
	bd = getBlockDataOf(world, x-1, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 8;
	bd = getBlockDataOf(world, x, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 16;
	bd = getBlockDataOf(world, x+1, y-1, z-1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 32;
	bd = getBlockDataOf(world, x+1, y-1, z);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 64;
	bd = getBlockDataOf(world, x+1, y-1, z+1);
	if (bd != NULL && !bd->isTransparent)
		bitmap |= 128;
	
	vals[0] = vertexAO(bitmap & 64, bitmap & 16, bitmap & 32);
	vals[1] = vertexAO(bitmap & 64, bitmap & 1, bitmap & 128);
	vals[2] = vertexAO(bitmap & 4, bitmap & 16, bitmap & 8);
	vals[3] = vertexAO(bitmap & 4, bitmap & 1, bitmap & 2);

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	unsigned short *shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 0;
	unsigned char *chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 0;
	*shrt = 1;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*(vmem + (*vsize)++) = x + 1.0;
	*(vmem + (*vsize)++) = y;
	*(vmem + (*vsize)++) = z + 1.0;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = 1.0;
	*(vmem + (*vsize)++) = id;
	*(vmem + (*vsize)++) = 0.0;
	*(vmem + (*vsize)++) = -1.0;
	*(vmem + (*vsize)++) = 0.0;
	shrt = (unsigned short*) (vmem + (*vsize)++);
	*shrt++ = 1;
	*shrt = 0;
	chr = (unsigned char*) (vmem + (*vsize)++);
	*chr++ = vals[0];
	*chr++ = vals[1];
	*chr++ = vals[2];
	*chr++ = vals[3];

	*verts += 6;
}
