#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "shader.h"
#include "glHelper.h"
#include "camera.h"
#include "mesh.h"
#include "cityGenerator.h"
#include "terrain.h"
#include "pool.h"
#include "skybox.h"


const GLuint BLUR_DOWNSAMPLE = 2;

class Graphics {
public:
    Graphics();
    Graphics(sf::RenderWindow& window);

    ~Graphics();

    void renderScene(Camera& cam, CityGenerator& cityGen, Terrain& terrainGen, bool toFrameBuffer);

    void resize(int width, int height);

    void finalProcessing(Camera& cam, bool blurring);

    static int registerTransform(bool solid = true);
    static Transform* getTransform(int id);

    // TODO generic mesh registration and handling
    static void registerMesh(Mesh<Vertex>* mesh);

    static void addToStream(bool solid, glm::mat4& model, glm::vec3& color);
    static void addToStream(bool solid, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    void setDebugStream(GLuint size, std::vector<glm::mat4>* models, std::vector<glm::vec3>* colors);


private:
    GLuint WIDTH, HEIGHT;

    void initGL(sf::RenderWindow& window);
    void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iters, Shader screen, Shader blur);
    void renderQuad();

    void buildBuffers();
    void destroyBuffers();
    void uploadTransforms();

    void printGLErrors();

    FBO sceneBuffer, blurResult, blurBuffers[2];
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