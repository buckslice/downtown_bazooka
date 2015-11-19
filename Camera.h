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
    bool grounded = false;
    bool flying = false;

    // vector constructor
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH);
    // scalar constructor
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjMatrix(GLuint w, GLuint h);

    void updateCameraVectors();
    void update(glm::vec3 input, GLfloat mdx, GLfloat mdy, GLfloat delta);

    void jump();

};

