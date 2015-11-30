#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "shader.h"
#include "glHelper.h"
#include "camera.h"
#include "mesh.h"

const GLuint BLUR_DOWNSAMPLE = 2;

class Graphics {
public:

    Mesh* cube;
    Mesh* guy;

    Graphics(sf::RenderWindow& window);
    ~Graphics();

    void renderScene(Camera& cam, bool drawDudes);
    void resize(int width, int height);
    //void renderUI();
    void postProcess();
    void buildShaders();
    void buildBuffers();

    GLuint genColorBuffer(Mesh& mesh, std::vector<glm::vec3>& colors);
    GLuint genModelBuffer(Mesh& mesh, std::vector<glm::mat4>& models);

    // should make gen buffers functions above just build buffer given a max_size
    // then use below functions to update it during runtime
    //void updateColorBuffer(GLuint buffer, std::vector<glm::vec3>& colors);
    //void updateModelBuffer(GLuint buffer, std::vector<glm::mat4>& models);

private:

    void initGL(sf::RenderWindow& window);
    void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iters, Shader screen, Shader blur);
    void renderQuad();
    void deleteShaders();

    Shader buildingShader;
    Shader blurShader;
    Shader screenShader;
    Shader blendShader;

    FBO sceneBuffer;
    FBO blurResult;
    FBO blurBuffers[2];
    GLuint quadVAO;

    GLint projLoc;
    GLint viewLoc;

    GLuint tex;

};