#include "resources.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Resources::Resources() {
    if (!font.loadFromFile("assets/fonts/OCRAEXT.ttf")) {
        std::cout << "ERROR::FONT::LOAD_FAILURE";
    }

    std::vector<std::pair<sf::SoundBuffer*, std::string>> soundPaths = {
        { &jumpSound, "jump.wav" },
        { &shootSound, "shoot.wav" },
        { &damageSound, "damage.wav" },
        { &itemGetSound, "item_get.wav" },
        { &menuSelectSound, "menu_select.wav" },
        { &menuMoveSound, "menu_move.wav"},
        { &explosionSound, "explosion.wav"},
        { &burningSound, "burning.wav"},
        { &healingSound, "healing.wav"}
    };
    loadAudio(soundPaths);

    loadTextures(true);

    // load cubemap for skybox
    std::vector<std::string> faces{
        "assets/images/skybox/nightsky_rt.tga",
        "assets/images/skybox/nightsky_lf.tga",
        "assets/images/skybox/nightsky_up.tga",
        "assets/images/skybox/nightsky_dn.tga",
        "assets/images/skybox/nightsky_bk.tga",
        "assets/images/skybox/nightsky_ft.tga"
    };
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
    glDeleteTextures(1, &triangleTex);
}

void Resources::loadTextures(bool mipmapped) {
    deleteTextures();
    gridTex = GLHelper::loadTexture("assets/images/grid.png", mipmapped);
    terrainTex = GLHelper::loadTexture("assets/images/grid2.png", mipmapped);
    solidTex = GLHelper::loadTexture("assets/images/solid.png", mipmapped);
    triangleTex = GLHelper::loadTexture("assets/images/triangle.png", mipmapped);
    noiseTex = GLHelper::loadTexture("assets/images/noise4.jpg", mipmapped);

    loadedTexturesBefore = true;
}

void Resources::loadAudio(std::vector<std::pair<sf::SoundBuffer*, std::string>>& soundPaths) {
    // load music track	
    if (!menuTrack.openFromFile("assets/music/expl1.ogg")) {
        std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
    }
    //if (!mainTrack.openFromFile("assets/music/DOWN_WE_GO_2.ogg")) {
    //    std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
    //}
    //mainTrack.setLoop(true);

    menuTrack.setLoop(true);

    for (size_t i = 0; i < soundPaths.size(); ++i) {
        std::string path = "assets/sounds/" + soundPaths[i].second;
        if (!soundPaths[i].first->loadFromFile(path)) {
            std::cout << "ERROR::SOUND_LOAD_FAILURE::INVALID_PATH: \"" << path << "\"" << std::endl;
        }
    }
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
    success &= terrainShader.build("assets/shaders/default_textured.vert", "assets/shaders/terrain_textured.frag");
    terrainShader.use();
    glUniform1i(glGetUniformLocation(terrainShader.program, "tex"), 0);
    glUniform1i(glGetUniformLocation(terrainShader.program, "noise"), 1);

    glm::mat4 model = glm::mat4();
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
    glDeleteProgram(terrainShader.program);
    glDeleteProgram(blurShader.program);
    glDeleteProgram(screenShader.program);
    glDeleteProgram(blendShader.program);
    glDeleteProgram(skyboxShader.program);
}