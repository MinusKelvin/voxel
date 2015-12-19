#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GLFW/glfw3.h>

#include "world.h"

static int collide(World *world) {
	int lowx = (int) floor(world->player.x);
	int highx = (int) ceil(world->player.x-0.5);
	int lowy = (int) floor(world->player.y);
	int highy = (int) ceil(world->player.y+0.8);
	int lowz = (int) floor(world->player.z);
	int highz = (int) ceil(world->player.z-0.5);
	int x,y,z;
	for (x = lowx; x <= highx; x++) {
		for (y = lowy; y <= highy; y++) {
			for (z = lowz; z <= highz; z++) {
				const BlockData *data = getBlockDataOf(world, x,y,z);
				if (!(data && data->isTransparent))
					return 1;
			}
		}
	}
	return 0;
}

void tickPlayer(World *world, Context *context) {
	int direction = world->player.xvel > 0.0 ? 1 : world->player.xvel < 0.0 ? -1 : 0;
	int i;
	for (i = 0; i < direction * world->player.xvel; i++) {
		if (world->player.xvel-i*direction >= 1.0)
			world->player.x += direction;
		else
			world->player.x += world->player.xvel-i*direction;
		if (collide(world)) {
			if (direction == 1) {
				world->player.x = floor(world->player.x+0.5)-0.5;
				world->player.xvel = 0;
			} else {
				world->player.x = floor(world->player.x+1);
				world->player.xvel = 0;
			}
			break;
		}
	}

	direction = world->player.zvel > 0.0 ? 1 : world->player.zvel < 0.0 ? -1 : 0;
	for (i = 0; i < direction * world->player.zvel; i++) {
		if (world->player.zvel-i*direction >= 1.0)
			world->player.z += direction;
		else
			world->player.z += world->player.zvel-i*direction;
		if (collide(world)) {
			if (direction == 1) {
				world->player.z = floor(world->player.z+0.5)-0.5;
				world->player.zvel = 0;
			} else {
				world->player.z = floor(world->player.z+1);
				world->player.zvel = 0;
			}
			break;
		}
	}

	int onGround = 0;
	direction = world->player.yvel > 0.0 ? 1 : world->player.yvel < 0.0 ? -1 : 0;
	for (i = 0; i < direction * world->player.yvel; i++) {
		if (world->player.yvel-i*direction >= 1.0)
			world->player.y += direction;
		else
			world->player.y += world->player.yvel-i*direction;
		if (collide(world)) {
			if (direction == 1) {
				world->player.y = floor(world->player.y+0.5)-0.5;
				world->player.yvel = 0;
			} else {
				world->player.y = floor(world->player.y+1);
				world->player.yvel = 0;
				onGround = 1;
			}
			break;
		}
	}

	if (glfwGetKey(context->window, GLFW_KEY_W)) {
		world->player.xvel += sin(world->player.yaw) * 0.05;
		world->player.zvel -= cos(world->player.yaw) * 0.05;
	}
	if (glfwGetKey(context->window, GLFW_KEY_S)) {
		world->player.xvel -= sin(world->player.yaw) * 0.05;
		world->player.zvel += cos(world->player.yaw) * 0.05;
	}
	if (glfwGetKey(context->window, GLFW_KEY_D)) {
		world->player.xvel += cos(world->player.yaw) * 0.05;
		world->player.zvel += sin(world->player.yaw) * 0.05;
	}
	if (glfwGetKey(context->window, GLFW_KEY_A)) {
		world->player.xvel -= cos(world->player.yaw) * 0.05;
		world->player.zvel -= sin(world->player.yaw) * 0.05;
	}
	if (/**/onGround &&/**/ glfwGetKey(context->window, GLFW_KEY_SPACE)) {
		world->player.yvel /*+= 0.05;/*/= 0.2;
	}
//	if (glfwGetKey(context->window, GLFW_KEY_LEFT_SHIFT))
//		world->player.yvel -= 0.05;

	world->player.yvel -= 0.014;
//	world->player.yvel *= 0.6;

	world->player.xvel *= 0.6;
	world->player.zvel *= 0.6;
}
