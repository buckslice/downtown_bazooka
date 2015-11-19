#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "shader.h"
#include "glHelper.h"
#include "camera.h"
#include "mesh.h"

const GLuint WIDTH = 1440, HEIGHT = 960;
const sf::Vector2i center(WIDTH / 2, HEIGHT / 2);
const GLuint BLUR_DOWNSAMPLE = 2;

class Graphics {
public:

    Mesh* buildingMesh;

    Graphics(sf::RenderWindow& window);
    ~Graphics();

    void renderScene(Camera& cam);
    //void renderUI();
    void postProcess();
    void buildShaders();

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

};
