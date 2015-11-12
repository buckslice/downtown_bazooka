#pragma once


#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 15.0f;
const GLfloat SENSITIVITY = 0.25f;
const GLfloat GRAVITY = 30.0f;
const GLfloat JUMPSPEED = 25.0f;

class Camera {

public:
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	glm::vec3 vel;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat speed;
	GLfloat mouseSensitivity;
	GLfloat zoom;
	bool grounded;

	// vector constructor
	Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH)
		:speed(SPEED), mouseSensitivity(SENSITIVITY) {
		this->pos = pos;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
	}

	// scalar constructor
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch)
		:speed(SPEED), mouseSensitivity(SENSITIVITY) {
		this->pos = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
	}

	glm::mat4 getViewMatrix() {
		return glm::lookAt(pos, pos + forward, up);
	}

	void update(glm::vec3 dir, GLfloat mdx, GLfloat mdy, GLfloat delta) {
		// update yaw and pitch from mouse deltas
		mdx *= mouseSensitivity;
		mdy *= mouseSensitivity;

		yaw += mdx;
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}
		if (yaw < 0.0f) {
			yaw += 360.0f;
		}
		pitch -= mdy;
		pitch = glm::clamp(pitch, -89.0f, 89.0f);

		// update camera vectors
		glm::vec3 f;
		f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		f.y = sin(glm::radians(pitch));
		f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward = glm::normalize(f);
		right = glm::normalize(glm::cross(forward, worldUp));
		up = glm::normalize(glm::cross(right, forward));

		// calculate position from movement direction and speed
		// old flying mode
		/*GLfloat y = dir.y;
		dir.y = 0.0f;
		glm::vec3 d;
		if (dir != glm::vec3(0.0f, 0.0f, 0.0f)) {
			d = glm::normalize(dir);
		}
		d.y = y;
		d *= speed * delta;
		pos += right * d.x + forward * d.z + worldUp * d.y;*/

		// fps mode
		if (vel.y != 0.0f) {
			grounded = false;
		}
		glm::vec3 newf = glm::normalize(glm::cross(worldUp, right));

		GLfloat oldy = vel.y;
		//vel = glm::vec3(0.0f);
		if (dir != glm::vec3(0.0f, 0.0f, 0.0f)) {
			//dir.y = 0.0f;	// ignore y dir for fps controls
			dir = glm::normalize(dir);
		}
		vel.y = 0.0f;

		GLfloat accel = grounded ? 10.0f : 2.0f;
		accel *= speed * delta;
		vel += (right * dir.x + newf * dir.z) * accel;
		if (glm::dot(vel, vel) > speed * speed) {
			vel = glm::normalize(vel) * speed;
		}

		// if no input then apply drag
		if (dir == glm::vec3(0.0f)) {
			vel *= (grounded ? .8f : .95f);
			//vel *= .9f;
		}

		// gravity 9.81 not right for some reason
		vel.y = oldy - GRAVITY * delta;

	}

	void jump() {
		// check if grounded
		if (grounded) {
			vel.y = JUMPSPEED;
			grounded = false;
		}
	}


};

