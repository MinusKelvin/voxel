#ifndef _VOXEL_GLSTUFF_H_
#define _VOXEL_GLSTUFF_H_

typedef struct Shaders {
	int basic;
	int basicProjLoc;
} Shaders;

typedef struct Textures {
	int blocks;
} Textures;

Shaders *buildShaders();
Textures *loadTextures();

#endif
