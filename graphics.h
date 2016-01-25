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

    static void setMeshVisible(GLuint id, bool value);

private:

    static void genColorBuffer(Mesh* mesh, std::vector<glm::vec3>& colors);
    static void genModelBuffer(Mesh* mesh, std::vector<glm::mat4>& models);
    static bool isValidMeshID(GLuint id);

    void initGL(sf::RenderWindow& window);
    void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iters, Shader screen, Shader blur);
    void renderQuad();

    FBO sceneBuffer;
    FBO blurResult;
    FBO blurBuffers[2];
    GLuint quadVAO;

    Mesh* floorMesh;
};