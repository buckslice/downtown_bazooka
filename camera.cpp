#include "camera.h"
#include <iostream>
#include <algorithm>

Camera::Camera()
    : mouseSensitivity{ SENSITIVITY } {
    yaw = 0.0f;
    pitch = 0.0f;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camDist = 0.0f;
    autoSpin = false;

    updateCameraVectors();
}

// scalar constructor
Camera::Camera(GLfloat yaw, GLfloat pitch, bool firstPerson)
    : mouseSensitivity(SENSITIVITY) {
    this->yaw = yaw;
    this->pitch = pitch;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camDist = 0.0f;
    autoSpin = false;

    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    glm::vec3 p = transform.getPos();
    p = p - forward * camDist;
    return glm::lookAt(p, p + forward, up);
}

glm::mat4 Camera::getProjMatrix(GLuint w, GLuint h) {
    return glm::perspective(45.0f, (GLfloat)w / (GLfloat)h, NEAR_PLANE, FAR_PLANE);
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
    //glm::vec3 cp = transform.getPos();
    //std::cout << cp.x << " " << cp.y << " " << cp.z << std::endl;
}

void Camera::update(GLint mdx, GLint mdy, GLfloat delta) {
    if (autoSpin) {
        pitch = 0.0f;
        yaw += 5.0f * delta;
    } else {
        // update yaw and pitch from mouse deltas
        yaw += mdx * mouseSensitivity;
        if (yaw > 360.0f) {
            yaw -= 360.0f;
        }
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }
        pitch -= mdy * mouseSensitivity;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }
    // lerp camDist towards target
    GLfloat t = std::max(std::min(delta*10.0f, 1.0f), 0.0f);
    camDist = (1.0f - t) * camDist + t * camDistTarget;

    updateCameraVectors();
}

void Camera::setAutoSpin(bool value) {
    autoSpin = value;
}

void Camera::updateCameraDistance(GLfloat deltaScroll, GLfloat deltaTime) {
    camDistTarget += deltaScroll;
    camDistTarget = std::max(std::min(camDistTarget, 50.0f), 0.0f);
}