#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLEW_NO_GLU
#include <GL/glew.h>

#include "glstuff.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static const char *basicVertexSource =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec4 position;\n"
	"\n"
	"uniform mat4 proj;\n"
	"\n"
	"out vec3 pos;\n"
	"out vec3 texcoord;\n"
	"\n"
	"const vec2[] texcoords = vec2[] (\n"
	"    vec2(1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 0.0),\n"
	"    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0)\n"
	");\n"
	"\n"
	"void main() {\n"
	"    gl_Position = proj * vec4(position.xyz, 1.0);\n"
	"    pos = position.xyz;\n"
	"    texcoord = vec3(texcoords[gl_VertexID % 6], position.w);\n"
	"}";

static const char *basicFragmentSource =
	"#version 330 core\n"
	"\n"
	"in vec3 pos;\n"
	"in vec3 texcoord;\n"
	"\n"
	"uniform sampler2DArray tex;\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"void main() {\n"
	"    color = texture(tex, texcoord);\n"
//	"    float y = fract(pos.y);\n"
//	"    float x = fract(pos.x);\n"
//	"    if ((x < 0.05 || x > 0.95) && (y < 0.05 || y > 0.95))\n"
//	"        color = vec4(0.0);\n"
//	"    float z = fract(pos.z);\n"
//	"    if ((z < 0.05 || z > 0.95) && (y < 0.05 || y > 0.95))\n"
//	"        color = vec4(0.0);\n"
//	"    if ((z < 0.05 || z > 0.95) && (x < 0.05 || x > 0.95))\n"
//	"        color = vec4(0.0);\n"
//	"    if ((z < 0.05 || z > 0.95) && (x < 0.05 || x > 0.95))\n"
//	"        color = vec4(0.0);\n"
	"}";

Shaders *buildShaders() {
	Shaders *s = malloc(sizeof(Shaders));
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	GLint result = GL_FALSE;

	glShaderSource(vertex, 1, &basicVertexSource, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint infologLength;
		char *infolog;
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &infologLength);
		infolog = malloc(infologLength+1);
		glGetShaderInfoLog(vertex, infologLength, NULL, infolog);
		fprintf(stderr, "Error compiling vertex shader:\n%s\n", infolog);
		exit(-1);
	}

	glShaderSource(fragment, 1, &basicFragmentSource, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint infologLength;
		char *infolog;
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &infologLength);
		infolog = malloc(infologLength+1);
		glGetShaderInfoLog(fragment, infologLength, NULL, infolog);
		fprintf(stderr, "Error compiling fragment shader:\n%s\n", infolog);
		exit(-1);
	}

	s->basic = glCreateProgram();
	glAttachShader(s->basic, vertex);
	glAttachShader(s->basic, fragment);
	glLinkProgram(s->basic);
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	s->basicProjLoc = glGetUniformLocation(s->basic, "proj");

	return s;
}

Textures *loadTextures() {
	Textures *t = malloc(sizeof(Textures));

	glGenTextures(1, &t->blocks);
	glBindTexture(GL_TEXTURE_2D_ARRAY, t->blocks);
	int w, h, n;
	unsigned char *image = stbi_load("res/blocks.png", &w, &h, &n, 4);
	unsigned char *data = malloc(w*h*4);

	// Apparently really complicated function. Image data is laid out in rows, need to lay out w/16 x h/16 tiles from it each image in a row... yeah.
	int bytesPerRow = w / 4;
	int i,j,k;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < h/16; j++) {
			for (k = 0; k < 16; k++) {
				memcpy(data + i*h*bytesPerRow + k*h/16*bytesPerRow + j*bytesPerRow, image + i*h*bytesPerRow + j*16*bytesPerRow + k*bytesPerRow, bytesPerRow);
			}
		}
	}

	stbi_image_free(image);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, w/16, h/16, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	free(data);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return t;
}
