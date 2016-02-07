#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "shader.h"
#include "glHelper.h"
#include "camera.h"
#include "mesh.h"
#include "terrainGenerator.h"

const GLuint BLUR_DOWNSAMPLE = 2;

class Graphics {
public:
    Graphics();
    Graphics(sf::RenderWindow& window);

    ~Graphics();

    void renderScene(Camera& cam, TerrainGenerator& tg, bool toFrameBuffer);

    void resize(int width, int height);
    //void renderUI();
    void postProcess();
    void buildBuffers();

    // should later add options for mesh type
    static GLuint registerMesh();   // because i cant figure out how to give default argument to static function
    static GLuint registerMesh(GLuint tex);

    // builds or updates color buffer for given mesh
    static void setColors(GLuint mesh_id, std::vector<glm::vec3>& colors);

    // builds or updates model buffer for given mesh
    static void setModels(GLuint mesh_id, std::vector<glm::mat4>& models);

    static void addToStream(bool solid, glm::mat4& model, glm::vec3& color);
    static void addToStream(bool solid, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);


    static void setMeshVisible(GLuint id, bool value);

    void setDebugStream(GLuint size, std::vector<glm::mat4>* models, std::vector<glm::vec3>* colors);

private:

    static void genColorBuffer(Mesh* mesh);
    static void genModelBuffer(Mesh* mesh);

    void setStream(Mesh* m, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    static bool isValidMeshID(GLuint id);

    void initGL(sf::RenderWindow& window);
    void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iters, Shader screen, Shader blur);
    void renderQuad();

    FBO sceneBuffer;
    FBO blurResult;
    FBO blurBuffers[2];
    GLuint quadVAO;

    // this parts kind of dumb but i did it through trying to diagnose something else
    GLuint dstreamSize;
    std::vector<glm::mat4>* dmodels;
    std::vector<glm::vec3>* dcolors;

    Mesh* floorMesh;

    Mesh* solidBox;
    Mesh* gridBox;
};