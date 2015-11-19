#include "camera.h"

// vector constructor
Camera::Camera(glm::vec3 pos, glm::vec3 up, GLfloat yaw, GLfloat pitch)
    :speed(SPEED), mouseSensitivity(SENSITIVITY) {
    this->pos = pos;
    this->worldUp = up;
    this->yaw = yaw;
    this->pitch = pitch;
    updateCameraVectors();
}

// scalar constructor
Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch)
    :speed(SPEED), mouseSensitivity(SENSITIVITY) {
    this->pos = glm::vec3(posX, posY, posZ);
    this->worldUp = glm::vec3(upX, upY, upZ);
    this->yaw = yaw;
    this->pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(pos, pos + forward, up);
}

glm::mat4 Camera::getProjMatrix(GLuint w, GLuint h) {
    return glm::perspective(45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 2000.0f);
}

void Camera::updateCameraVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward = glm::normalize(f);
    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));
}

void Camera::update(glm::vec3 input, GLfloat mdx, GLfloat mdy, GLfloat delta) {
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

    updateCameraVectors();

    // movement controls
    glm::vec3 xzforward = glm::normalize(glm::cross(worldUp, right));
    if (input != glm::vec3(0.0f, 0.0f, 0.0f)) {
        input = glm::normalize(input);
    }

    if (flying) {
        vel = (right * input.x + xzforward * input.z + worldUp * input.y) * speed * 8.0f;
    } else {
        input.y = 0.0f; // ignore this part of input
        if (vel.y != 0.0f) {
            grounded = false;
        }

        GLfloat oldy = vel.y;
        GLfloat accel = grounded ? 10.0f : 2.0f;
        accel *= speed * delta;
        vel.y = 0.0f;
        vel += (right * input.x + xzforward * input.z) * accel;
        if (glm::dot(vel, vel) > speed * speed) {
            vel = glm::normalize(vel) * speed;
        }

        // if no input then apply drag
        if (input == glm::vec3(0.0f)) {
            vel *= (grounded ? .8f : .95f);
            vel *= .9f;
        }
        // gravity 9.81 not right for some reason
        vel.y = oldy - GRAVITY * delta;
    }
}

void Camera::jump() {
    // check if grounded
    if (grounded && !flying) {
        vel.y = JUMPSPEED;
        grounded = false;
    }
}