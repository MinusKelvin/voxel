#ifndef _VOXEL_WORLD_H_
#define _VOXEL_WORLD_H_

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "simplex.h"
#include "glstuff.h"
#include "math/mat4.h"

typedef struct Player {
	float x,y,z;
	float xvel,yvel,zvel;
	float yaw, pitch;
} Player;

typedef struct BlockData {
	int isTransparent;
	int topTextureID;
	int bottomTextureID;
	int northTextureID;
	int southTextureID;
	int westTextureID;
	int eastTextureID;
} BlockData;

typedef struct Block {
	int id;
} Block;

#define VERTEX_BUF 0
#define ELEMENT_BUF 1
typedef struct Chunk {
	Block blocks[32][32][32];
	GLuint buffers[2];
	int verts;
	int x;
	int y;
	int z;
} Chunk;

#define CHUNK_X 32
#define CHUNK_Y 8
#define CHUNK_Z 32
#define WORLD_X (32*CHUNK_X)
#define WORLD_Z (32*CHUNK_Z)
#define WORLD_Y (32*CHUNK_Y)
typedef struct World {
	// This is X Z Y
	Chunk chunks[CHUNK_X][CHUNK_Z][CHUNK_Y];
	Player player;
	SimplexInstance *simplex[2];
} World;

typedef struct Context {
	float *yaw, *pitch;
	int isGrabbed;
	Mat4 perspective;
	Shaders *shaders;
	Textures *textures;
	GLFWwindow *window;
	int width, height;
	float sunx, suny, sunz;
} Context;

extern const BlockData blockdata[16];

World *createWorld();
void tickWorld(World *world, Context *context);
void tickPlayer(World *world, Context *context);
const BlockData *getBlockDataOf(World *world, int x, int y, int z);

void renderWorld(World *world, Context *context);
void updateChunkVBO(World *world, Chunk *chunk);

int max(int a, int b);
int min(int a, int b);

#endif
