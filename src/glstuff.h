#ifndef _VOXEL_GLSTUFF_H_
#define _VOXEL_GLSTUFF_H_

#define GLEW_NO_GLU
#include <GL/glew.h>

typedef struct Shaders {
	GLuint basic;
	GLint basicProjLoc;
	GLint basicSundirLoc;
	GLint basicSProjLoc;
	GLuint shadow;
	GLuint shadowProjLoc;
} Shaders;

typedef struct Textures {
	GLuint blocks;
	GLuint shadow;
	GLuint fbo;
} Textures;

Shaders *buildShaders();
Textures *loadTextures();

#endif
