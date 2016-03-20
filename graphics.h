#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "shader.h"
#include "glHelper.h"
#include "camera.h"
#include "mesh.h"
#include "terrain.h"
#include "pool.h"
#include "skybox.h"


const GLuint BLUR_DOWNSAMPLE = 2;

class Graphics {
public:
    Graphics();
    Graphics(sf::RenderWindow& window);

    ~Graphics();

    void renderScene(Camera& cam, Terrain& tg, bool toFrameBuffer);

    void resize(int width, int height);
    //void renderUI();
    void finalProcessing(Camera& cam, bool blurring);

    // should later add options for mesh type
    static GLuint registerMesh();   // because i cant figure out how to give default argument to static function
    static GLuint registerMesh(GLuint tex);

    static void addToStream(bool solid, glm::mat4& model, glm::vec3& color);
    static void addToStream(bool solid, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    static void setMeshVisible(GLuint id, bool value);

    void setDebugStream(GLuint size, std::vector<glm::mat4>* models, std::vector<glm::vec3>* colors);

    static int registerTransform(bool solid = true);
    static Transform* getTransform(int id);

private:
    GLuint WIDTH;
    GLuint HEIGHT;

    void uploadBoxes();

    static bool isValidMeshID(GLuint id);

    void initGL(sf::RenderWindow& window);
    void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iters, Shader screen, Shader blur);
    void renderQuad();

    void buildBuffers();
    void destroyBuffers();

    void printGLErrors();

    FBO sceneBuffer;
    FBO blurResult;
    FBO blurBuffers[2];
    GLuint quadVAO;

    // debug stream
    GLuint dstreamSize;
    std::vector<glm::mat4>* dmodels;
    std::vector<glm::vec3>* dcolors;

    PIMesh* solidStream;
    TIMesh* gridStream;
    TIMesh* buildingCube;

    Skybox* skybox;
};