#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "transform.h"

const GLfloat SENSITIVITY = 0.25f;
const GLfloat NEAR_PLANE = 0.1f;
const GLfloat FAR_PLANE = 2000.0f;
const GLfloat DEFAULT_CAMDIST = 20.0f;

enum class CameraMode {
    NORMAL,
    AUTOSPIN,
    DEATH
};
class Camera {

public:
    glm::vec3 forward, up, right, worldUp;
    Transform* transform;
    CameraMode behavior;

    Camera();
    Camera(GLfloat yaw, GLfloat pitch, bool firstPerson = true);

    void update(GLint mdx, GLint mdy, GLfloat delta);

    void updateCameraDistance(GLfloat deltaScroll);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjMatrix(GLuint w, GLuint h) const;
    GLfloat getCamDist() const;

private:
    GLfloat yaw;
    GLfloat pitch;
    GLfloat mouseSensitivity;
    GLfloat camDist;
    GLfloat targCamDist = DEFAULT_CAMDIST;

    void updateCameraVectors();
};

