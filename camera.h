#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "transform.h"

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SENSITIVITY = 0.25f;

class Camera {

public:
    Transform transform;

    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    GLfloat yaw;
    GLfloat pitch;

    GLfloat mouseSensitivity;
    
	Camera();
    Camera(GLfloat yaw, GLfloat pitch);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjMatrix(GLuint w, GLuint h);

    void updateCameraVectors();
    void update(GLint mdx, GLint mdy);

};

