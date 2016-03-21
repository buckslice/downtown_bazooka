#include "resources.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Resources::Resources() {
    if (!font.loadFromFile("assets/fonts/OCRAEXT.ttf")) {
        std::cout << "ERROR::FONT::LOAD_FAILURE";
    }

    loadTextures(true);

    std::vector<std::string> faces;
    faces.push_back("assets/images/skybox/nightsky_rt.tga");
    faces.push_back("assets/images/skybox/nightsky_lf.tga");
    faces.push_back("assets/images/skybox/nightsky_up.tga");
    faces.push_back("assets/images/skybox/nightsky_dn.tga");
    faces.push_back("assets/images/skybox/nightsky_bk.tga");
    faces.push_back("assets/images/skybox/nightsky_ft.tga");
    skyboxTex = GLHelper::loadCubeMap(faces);

    buildShaders();
}

Resources::~Resources() {
    deleteTextures();
    deleteShaders();

    glDeleteTextures(1, &skyboxTex);
}

bool loadedTexturesBefore = false;
void Resources::deleteTextures() {
    if (!loadedTexturesBefore) {
        return;
    }
    glDeleteTextures(1, &gridTex);
    glDeleteTextures(1, &terrainTex);
    glDeleteTextures(1, &solidTex);
}

void Resources::loadTextures(bool mipmapped) {
    deleteTextures();
    gridTex = GLHelper::loadTexture("assets/images/grid.png", mipmapped);
    terrainTex = GLHelper::loadTexture("assets/images/grid2.png", mipmapped);
    solidTex = GLHelper::loadTexture("assets/images/solid.png", mipmapped);

    loadedTexturesBefore = true;
}


bool loadedShadersBefore = false;

void Resources::buildShaders() {
    bool success = true;

    // instanceShader
    success &= instanceShader.build("assets/shaders/instanced.vert", "assets/shaders/default.frag");

    // instancedTexShader
    success &= instanceTexShader.build("assets/shaders/instanced_textured.vert", "assets/shaders/default_textured.frag");
    instanceTexShader.use();
    glUniform1i(glGetUniformLocation(instanceTexShader.program, "tex"), 0);

    // terrainShader
    success &= defaultShader.build("assets/shaders/default_textured.vert", "assets/shaders/default_textured.frag");
    defaultShader.use();
    glUniform1i(glGetUniformLocation(defaultShader.program, "tex"), 0);
    glm::mat4 model = glm::mat4();
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(defaultShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));

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

    // skyboxShader
    success &= skyboxShader.build("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    skyboxShader.use();
    glUniform1i(glGetUniformLocation(skyboxShader.program, "skybox"), 0);


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
    glDeleteProgram(instanceTexShader.program);
    glDeleteProgram(defaultShader.program);
    glDeleteProgram(blurShader.program);
    glDeleteProgram(screenShader.program);
    glDeleteProgram(blendShader.program);
    glDeleteProgram(skyboxShader.program);
}