#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "transform.h"

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SENSITIVITY = 0.25f;

const GLfloat NEAR_PLANE = 0.1f;
const GLfloat FAR_PLANE = 2000.0f;

class Camera {

public:
    Transform* transform;

    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
	Camera();
    Camera(GLfloat yaw, GLfloat pitch, bool firstPerson = true);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjMatrix(GLuint w, GLuint h);

    void update(GLint mdx, GLint mdy, GLfloat delta);

    void updateCameraDistance(GLfloat deltaScroll);

    GLfloat getCamDist();

    void setAutoSpin(bool value);

private:
    GLfloat yaw;
    GLfloat pitch;

    GLfloat mouseSensitivity;
    GLfloat camDist;
    GLfloat camDistTarget = 0.0f;

    bool autoSpin;

    void updateCameraVectors();
};

