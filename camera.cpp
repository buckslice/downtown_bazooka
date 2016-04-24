#include "camera.h"
#include <iostream>
#include <algorithm>

Camera::Camera()
    : mouseSensitivity{ SENSITIVITY } {
    yaw = 0.0f;
    pitch = 0.0f;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camDist = 0.0f;
    transform = new Transform();
    updateCameraVectors();
}

// scalar constructor
Camera::Camera(GLfloat yaw, GLfloat pitch, bool firstPerson)
    : mouseSensitivity(SENSITIVITY) {
    this->yaw = yaw;
    this->pitch = pitch;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camDist = 0.0f;
    transform = new Transform();
    updateCameraVectors();
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
    //glm::vec3 cp = transform->getWorldPos();
    //std::cout << cp.x << " " << cp.y << " " << cp.z << std::endl;
}

void Camera::update(GLint mdx, GLint mdy, GLfloat delta) {
    switch (behavior) {
    case CameraMode::AUTOSPIN:
        pitch = 0.0f;
        yaw += 5.0f * delta;
        camDist = 0.0f;
        camDistTarget = DEFAULT_CAMDISTTARGET;
        updateCameraVectors();
        return;
    case CameraMode::NORMAL:
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
        break;
    case CameraMode::DEATH:
        pitch = -90.0f;
        camDistTarget += 4.0f * delta;
        yaw += 5.0f * delta;
        break;
    }
    // lerp camDist towards target
    GLfloat t = std::max(std::min(delta*10.0f, 1.0f), 0.0f);
    camDist = (1.0f - t) * camDist + t * camDistTarget; // manual lerp what is this
    updateCameraVectors();
}

GLfloat scrollSpeed = 2.0f;
void Camera::updateCameraDistance(GLfloat deltaScroll) {
    camDistTarget += deltaScroll * scrollSpeed;
    camDistTarget = std::max(std::min(camDistTarget, 100.0f), 0.0f);
}

glm::mat4 Camera::getViewMatrix() const {
    glm::vec3 p = transform->getWorldPos();
    p = p - forward * camDist;
    return glm::lookAt(p, p + forward, up);
}

glm::mat4 Camera::getProjMatrix(GLuint w, GLuint h) const {
    return glm::perspective(45.0f, (GLfloat)w / (GLfloat)h, NEAR_PLANE, FAR_PLANE);
}

GLfloat Camera::getCamDist() const {
    return camDist;
}
