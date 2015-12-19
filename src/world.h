#ifndef _VOXEL_WORLD_H_
#define _VOXEL_WORLD_H_

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

typedef struct Chunk {
	Block blocks[16][16][16];
	int vbo;
	int verts;
	int x;
	int y;
	int z;
} Chunk;

typedef struct World {
	Chunk chunks[64][16][64];
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
} Context;

extern const BlockData blockdata[16];

World *createWorld();
void tickWorld(World *world, Context *context);
void tickPlayer(World *world, Context *context);
const BlockData *getBlockDataOf(World *world, int x, int y, int z);

void renderWorld(World *world, Context *context);
void updateChunkVBO(World *world, Chunk *chunk);

#endif
