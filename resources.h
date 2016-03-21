#pragma once
#include <GL/glew.h>
#include "glHelper.h"
#include "shader.h"

// follows meyers singleton pattern
class Resources {
public:
    static Resources& get() {
        static Resources instance;
        return instance;
    }

    GLuint gridTex;
    GLuint terrainTex;
    GLuint solidTex;

    GLuint skyboxTex;

    Shader instanceShader;
    Shader instanceTexShader;
    Shader defaultShader;
    Shader blurShader;
    Shader screenShader;
    Shader blendShader;
    Shader skyboxShader;

    sf::Font font;

    void buildShaders();
    void loadTextures(bool mipmapped);

    // singleton: delete copy constructor and assignment operator
    Resources(Resources const&) = delete;
    Resources& operator=(Resources const&) = delete;
private:
    Resources();
    ~Resources();

    void deleteShaders();
    void deleteTextures();
};
