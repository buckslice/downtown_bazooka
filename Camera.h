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
    bool flying = false;

    // vector constructor
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH)
        :speed(SPEED), mouseSensitivity(SENSITIVITY) {
        this->pos = pos;
        this->worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        updateCameraVectors();
    }

    // scalar constructor
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch)
        :speed(SPEED), mouseSensitivity(SENSITIVITY) {
        this->pos = glm::vec3(posX, posY, posZ);
        this->worldUp = glm::vec3(upX, upY, upZ);
        this->yaw = yaw;
        this->pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(pos, pos + forward, up);
    }

    void updateCameraVectors() {
        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward = glm::normalize(f);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
    }

    void update(glm::vec3 input, GLfloat mdx, GLfloat mdy, GLfloat delta) {
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

    void jump() {
        // check if grounded
        if (grounded && !flying) {
            vel.y = JUMPSPEED;
            grounded = false;
        }
    }

};

