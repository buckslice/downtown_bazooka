#include "resources.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Resources::Resources() {
    gridTex = GLHelper::loadTexture("assets/images/grid.png");
    terrainTex = gridTex;//Masana: I got a runtime error here when it tried to load grid.ping again //GLHelper::loadTexture("assets/images/grid.png");
    solidTex = GLHelper::loadTexture("assets/images/solid.png");

}

Resources::~Resources() {
    glDeleteTextures(1, &gridTex);
    glDeleteTextures(1, &terrainTex);
    glDeleteTextures(1, &solidTex);
    deleteShaders();
}

bool loadedShadersBefore = false;
// would be better to only rebuild shaders if they have been changed
void Resources::buildShaders() {
    bool success = true;

    // instancedShader
    success &= instanceShader.build("assets/shaders/instanced.vert", "assets/shaders/default.frag");
    instanceShader.use();
    glUniform1i(glGetUniformLocation(instanceShader.program, "tex"), 0);

    // tiledShader
    success &= tiledShader.build("assets/shaders/default.vert", "assets/shaders/tiled.frag");
    tiledShader.use();
    glUniform1i(glGetUniformLocation(tiledShader.program, "tex"), 0);
    //glUniform3f(glGetUniformLocation(tiledShader.program, "Color"), 1.0f, 0.0f, 0.0f);
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2000.0f, 10.0f, 2000.0f));  // too lazy to import citygen for CITY_SIZE lol
    glUniformMatrix4fv(glGetUniformLocation(tiledShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // terrainShader
    success &= terrainShader.build("assets/shaders/colormesh.vert", "assets/shaders/default.frag");
    terrainShader.use();
    glUniform1i(glGetUniformLocation(terrainShader.program, "tex"), 0);
    model = glm::mat4();
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(terrainShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // blurShader and screenShader
    success &= blurShader.build("assets/shaders/screen.vert", "assets/shaders/blur.frag");
    success &= screenShader.build("assets/shaders/screen.vert", "assets/shaders/screen.frag");
    screenShader.use();
    glUniform1i(glGetUniformLocation(screenShader.program, "screen"), 0);

    // blendShader
    // blends the blur with the scene
    success &= blendShader.build("assets/shaders/screen.vert", "assets/shaders/blend.frag");
    blendShader.use();
    glUniform1i(glGetUniformLocation(blendShader.program, "scene"), 0);
    glUniform1i(glGetUniformLocation(blendShader.program, "blur"), 1);

    if (success && loadedShadersBefore) {
        std::cout << "SHADERS::RECOMPILE::SUCCESS" << std::endl;
    }
    loadedShadersBefore = true;
}

void Resources::deleteShaders() {
    if (!loadedShadersBefore) {
        return;
    }
    glDeleteProgram(instanceShader.program);
    glDeleteProgram(tiledShader.program);
    glDeleteProgram(terrainShader.program);
    glDeleteProgram(blurShader.program);
    glDeleteProgram(screenShader.program);
    glDeleteProgram(blendShader.program);
}