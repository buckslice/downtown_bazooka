#include "camera.h"
#include <iostream>

Camera::Camera()
	: mouseSensitivity{SENSITIVITY}, yaw{0.0f}, pitch{0.0f}
{
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	updateCameraVectors();
}

// scalar constructor
Camera::Camera(GLfloat yaw, GLfloat pitch)
    : mouseSensitivity(SENSITIVITY) {
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->yaw = yaw;
    this->pitch = pitch;

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    glm::vec3 p = transform.getPos();
    return glm::lookAt(p, p + forward, up);
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
    //std::cout << forward.x << " " << forward.y << " " << forward.z << std::endl;
}

void Camera::update(GLfloat mdx, GLfloat mdy) {
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
}