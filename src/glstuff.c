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
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec3 tex;\n"
	"layout(location = 2) in vec3 norm;\n"
	"layout(location = 3) in vec2 quadI;\n"
	"layout(location = 4) in vec4 aoFactors;\n"
	"\n"
	"uniform mat4 proj;\n"
	"uniform mat4 sproj;\n"
	"\n"
	"out vec3 texcoord;\n"
	"out vec3 normal;\n"
	"out vec2 quad;\n"
	"flat out vec4 ao;\n"
	"out vec3 shadowpos;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = proj * vec4(position, 1.0);\n"
	"    texcoord = tex;\n"
	"    normal = norm;\n"
	"    ao = aoFactors;\n"
	"    quad = quadI;\n"
	"    shadowpos = (sproj * vec4(position, 1.0)).xyz * 0.5 + 0.5;\n"
	"}";

static const char *basicFragmentSource =
	"#version 330 core\n"
	"#extension GL_ARB_texture_gather : enable\n"
	"\n"
	"in vec3 texcoord;\n"
	"in vec3 normal;\n"
	"in vec2 quad;\n"
	"flat in vec4 ao;\n"
	"in vec3 shadowpos;\n"
	"\n"
	"uniform sampler2DArray tex;\n"
	"uniform sampler2D shadow;\n"
	"uniform vec4 sundir;\n"
	"\n"
	"out vec4 color;\n"
	"\n"
	"void main() {\n"
	"    color = texture(tex, texcoord);\n"
	"    float shadowValue = float(shadowpos.x > 0.0 && shadowpos.x < 1.0 && shadowpos.y > 0.0 && shadowpos.y < 1.0);\n"
	"    shadowValue *= dot(vec4(step(shadowpos.z, textureGather(shadow, shadowpos.xy).r),"
	"                           step(shadowpos.z, textureGather(shadow, shadowpos.xy).g),"
	"                           step(shadowpos.z, textureGather(shadow, shadowpos.xy).b),"
	"                           step(shadowpos.z, textureGather(shadow, shadowpos.xy).a)), vec4(0.25));\n"
	"    shadowValue = 1.0 - shadowValue;\n"
	"    color.rgb *= min(1.0, 0.5 + min(max(0.0, dot(sundir.xyz, normalize(normal.xyz))), shadowValue)) * sundir.w;\n"
	"    color.rgb *= mix(mix(ao.x, ao.z, quad.x), mix(ao.y, ao.w, quad.x), quad.y);\n"
	"}";

static const char *shadowVertexSource =
	"#version 330 core\n"
	"\n"
	"layout(location = 0) in vec3 position;\n"
	"\n"
	"uniform mat4 proj;\n"
	"\n"
	"void main() {"
	"    gl_Position = proj * vec4(position, 1.0);\n"
	"}";

static const char *shadowFragmentSource =
	"#version 330 core\n"
	"\n"
	"void main() {}";

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
	s->basicSundirLoc = glGetUniformLocation(s->basic, "sundir");
	s->basicSProjLoc = glGetUniformLocation(s->basic, "sproj");

	vertex = glCreateShader(GL_VERTEX_SHADER);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	result = GL_FALSE;

	glShaderSource(vertex, 1, &shadowVertexSource, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint infologLength;
		char *infolog;
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &infologLength);
		infolog = malloc(infologLength+1);
		glGetShaderInfoLog(vertex, infologLength, NULL, infolog);
		fprintf(stderr, "Error compiling shadow vertex shader:\n%s\n", infolog);
		exit(-1);
	}

	glShaderSource(fragment, 1, &shadowFragmentSource, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint infologLength;
		char *infolog;
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &infologLength);
		infolog = malloc(infologLength+1);
		glGetShaderInfoLog(fragment, infologLength, NULL, infolog);
		fprintf(stderr, "Error compiling shadow fragment shader:\n%s\n", infolog);
		exit(-1);
	}

	s->shadow = glCreateProgram();
	glAttachShader(s->shadow, vertex);
	glAttachShader(s->shadow, fragment);
	glLinkProgram(s->shadow);
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	s->shadowProjLoc = glGetUniformLocation(s->shadow, "proj");
	glUseProgram(s->basic);
	glUniform1i(glGetUniformLocation(s->basic, "shadow"), 1);

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
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &t->shadow);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, t->shadow);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 8192, 8192, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid*) 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0);

	glGenFramebuffers(1, &t->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, t->fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, t->shadow, 0);
	glDrawBuffer(GL_NONE);
	printf("%x %x\n",glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return t;
}
