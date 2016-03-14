#pragma once
#include "glHelper.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "resources.h"

class Skybox {
public:
    Skybox();
    //~Skybox();

    void render(glm::mat4 view, glm::mat4 proj);

private:
    GLuint skyboxVAO, skyboxVBO;
};