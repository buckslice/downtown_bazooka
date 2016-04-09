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

const float DEFAULT_CAMDISTTARGET = 10.0f;
enum class CameraMode{
	NORMAL,
	AUTOSPIN,
	DEATH
};
class Camera {

public:
    Transform* transform;
	CameraMode behavior;
    glm::vec3 forward, up, right, worldUp;
    
	Camera();
    Camera(GLfloat yaw, GLfloat pitch, bool firstPerson = true);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjMatrix(GLuint w, GLuint h);

    void update(GLint mdx, GLint mdy, GLfloat delta);

    void updateCameraDistance(GLfloat deltaScroll);

    GLfloat getCamDist();

private:
    GLfloat yaw, pitch, mouseSensitivity, camDist, camDistTarget = DEFAULT_CAMDISTTARGET;

    void updateCameraVectors();
};

