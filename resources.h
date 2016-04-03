#pragma once
#include <GL/glew.h>
#include "glHelper.h"
#include "shader.h"
#include <SFML/Audio.hpp>

// follows meyers singleton pattern
class Resources {
public:
    static Resources& get() {
        static Resources instance;
        return instance;
    }

    GLuint gridTex, terrainTex, solidTex, skyboxTex;

    Shader instanceShader, instanceTexShader, defaultShader, blurShader, screenShader, blendShader, skyboxShader;

    sf::Font font;
	sf::Music mainTrack;
	sf::SoundBuffer jumpSound, shootSound, damageSound, itemSound, menuSelectSound, menuMoveSound, explosionSound;

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
