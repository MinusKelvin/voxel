#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "world.h"

#include "glstuff.h"
#include "math/mat4.h"

static float clearColor[] = {
	0.1, 0.125, 1, 1.0,
};

int max(int a, int b) {
	return a > b ? a : b;
}

int min(int a, int b) {
	return a < b ? a : b;
}

void fbosize(GLFWwindow *win, int w, int h) {
	glViewport(0, 0, w, h);
	Context *context = glfwGetWindowUserPointer(win);
	mat4Perspective(PI/2, (float) w / h, 0.1, 1512.0, &context->perspective);
	context->width = w;
	context->height = h;
	char title[25];
	snprintf(title, 25, "Voxel: %i x %i", w, h);
	glfwSetWindowTitle(win, title);
}

void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	Context *context = glfwGetWindowUserPointer(win);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		if (context->isGrabbed) {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(win, 0, 0);
		}
		context->isGrabbed ^= 1;
	}
}

void mousePos(GLFWwindow *win, double x, double y) {
	Context *context = glfwGetWindowUserPointer(win);
	if (context->isGrabbed) {
		*context->yaw += x * 0.007;
		*context->pitch += y * 0.007;
		glfwSetCursorPos(win, 0, 0);
	}
}

int main(int argc, char **argv) {
	if (!glfwInit()) {
		fprintf(stderr, "Failed to init glfw\n");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(1280,720,"Voxel: 1280 x 720",NULL,NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window\n");
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to init glew");
		return 3;
	}

	static Context context;
	context.isGrabbed = 1;
	context.window = window;
	context.width = 1280;
	context.height = 720;
	context.suny = 0.440225;
	context.sunx = 0.880451;
	context.sunz = 0.17609;
	glfwSetWindowUserPointer(window, &context);
	glfwSwapInterval(0);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);

	glfwSetFramebufferSizeCallback(window, &fbosize);
	glfwSetKeyCallback(window, &keyCallback);
	glfwSetCursorPosCallback(window, &mousePos);

	glGetError(); // Clear error left behind from glewInit

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	context.shaders = buildShaders();
	glUseProgram(context.shaders->basic);
	context.textures = loadTextures();

	mat4Perspective(PI/2, 16/9.0, 0.1, 1512, &context.perspective);

	World *world = createWorld();
	context.yaw = &world->player.yaw;
	context.pitch = &world->player.pitch;
	*context.yaw = 0.0;
	*context.pitch = 0.0;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	float depthClear = 1;
	float v = 0;
	do {
		double t = glfwGetTime();
		if (context.isGrabbed) {
			v += 0.001;
			context.sunz = 0.5;
			context.suny = cos(v);
			context.sunz = sin(v);
			float len = sqrt(context.sunz * context.sunz + context.suny * context.suny + context.sunx * context.sunx);
			context.sunz /= len;
			context.suny /= len;
			context.sunx /= len;
			tickWorld(world, &context);
			printf("x: %f, y: %f, z: %f\n", world->player.x, world->player.y, world->player.z);
		}

		glClearBufferfv(GL_COLOR, 0, clearColor);
		glClearBufferfv(GL_DEPTH, 0, &depthClear);
		glBindFramebuffer(GL_FRAMEBUFFER, context.textures->fbo);
		float zero = 0;
		glClearBufferfv(GL_DEPTH, 0, &zero);

		renderWorld(world, &context);

		glfwSwapBuffers(window);
		glfwPollEvents();
		printf("%f\n", glfwGetTime() - t);
		//printf("%x\n", glGetError());
	} while (glfwWindowShouldClose(window) == 0);
	return 0;
}
