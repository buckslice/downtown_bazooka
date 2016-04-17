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

    GLuint gridTex;
    GLuint terrainTex;
    GLuint solidTex;
    GLuint triangleTex;
    GLuint skyboxTex;
    GLuint noiseTex;

    Shader instanceShader;
    Shader instanceTexShader;
    Shader terrainShader;
    Shader blurShader;
    Shader screenShader;
    Shader blendShader;
    Shader skyboxShader;

    sf::Font font;

    sf::Music mainTrack;
    sf::Music menuTrack;

    sf::SoundBuffer jumpSound;
    sf::SoundBuffer shootSound;
    sf::SoundBuffer damageSound;
    sf::SoundBuffer itemGetSound;
    sf::SoundBuffer menuSelectSound;
    sf::SoundBuffer menuMoveSound;
    sf::SoundBuffer explosionSound;
    sf::SoundBuffer burningSound;
    sf::SoundBuffer healingSound;

    void buildShaders();
    void loadTextures(bool mipmapped);
    void loadAudio(std::vector<std::pair<sf::SoundBuffer*, std::string>>& soundPaths);

    // singleton: delete copy constructor and assignment operator
    Resources(Resources const&) = delete;
    Resources& operator=(Resources const&) = delete;
private:
    Resources();
    ~Resources();

    void deleteShaders();
    void deleteTextures();
};
