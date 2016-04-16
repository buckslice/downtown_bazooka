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
extern sf::Clock gameTime;

class Graphics {
public:
    Graphics();
    Graphics(sf::RenderWindow& window);

    ~Graphics();

    void renderScene(Camera& cam, Terrain& terrain, bool toFrameBuffer);

    void resize(int width, int height);

    void finalProcessing(Camera& cam, bool blurring);

    static Transform* registerTransform(Shape shape = Shape::CUBE_GRID);
    static void returnTransform(Transform* transform);
    static Transform* getTransform(int id);

    // TODO generic mesh registration and handling
    static void registerMesh(Mesh<Vertex>* mesh);

    static void addToStream(Shape shape, glm::mat4& model, glm::vec3& color);
    static void addToStream(Shape shape, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors);

    static bool DEBUG;
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

    PIMesh* solidStream;
	TIMesh* gridStream;
    TIMesh* pyrStream;
	TIMesh* buildingCube;

    Skybox* skybox;
};