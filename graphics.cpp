#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics.h"
#include "resources.h"
#include "hsbColor.h"
#include "mathutil.h"
#include "glm/ext.hpp"

static MemPool<Transform> boxes(20000);
//static std::vector<Mesh<Vertex>*> meshes;

static std::vector<glm::mat4> smodels;
static std::vector<glm::vec3> scolors;
static std::vector<glm::mat4> gmodels;
static std::vector<glm::vec3> gcolors;
static std::vector<glm::mat4> pmodels;
static std::vector<glm::vec3> pcolors;

bool Graphics::DEBUG = false;

// if you dont have this then breakpoints dont work! lol
Graphics::Graphics() {
}

Transform* Graphics::registerTransform(Shape shape) {
    Transform* t = boxes.alloc();
    t->shape = shape;
    return t;
}

void Graphics::returnTransform(Transform* transform) {
    boxes.free(transform);
}

Transform* Graphics::getTransform(int id) {
    return boxes.get(id);
}

Graphics::Graphics(sf::RenderWindow& window) {
    WIDTH = window.getSize().x;
    HEIGHT = window.getSize().y;

    initGL(window);

    solidStream = new PIMesh(PIMesh::cubeVertices, cubeElements);
    //gridStream = new TIMesh(TIMesh::pyramidVertices, pyramidElements, Resources::get().triangleTex);
    gridStream = new TIMesh(TIMesh::cubeVertices, cubeElements, Resources::get().gridTex);
    pyrStream = new TIMesh(TIMesh::pyramidVertices, pyramidElements, Resources::get().triangleTex);

    skybox = new Skybox();

    // testing out new transforms by setting up some arrows for the axes
    Transform* xbox = registerTransform();
    Transform* xboxl = registerTransform();
    Transform* xboxr = registerTransform();
    Transform* xx1 = registerTransform();
    Transform* xx2 = registerTransform();

    Transform* zbox = registerTransform();
    Transform* zboxl = registerTransform();
    Transform* zboxr = registerTransform();
    Transform* zz1 = registerTransform();
    Transform* zz2 = registerTransform();
    Transform* zz3 = registerTransform();

    glm::vec3 blue = glm::vec3(0.0f, 0.0f, 1.0f);
    xbox->setPos(10.0f, 0.0f, 0.0f);
    xbox->setScale(20.0f, 1.0f, 1.0f);
    xbox->color = blue;

    xboxl->setPos(18.0f, 0.0f, 2.0f);
    xboxl->setRot(0.0f, 45.0f, 0.0f);
    xboxl->setScale(6.0f, 1.0f, 1.0f);
    xboxl->color = blue;

    xboxr->setPos(18.0f, 0.0f, -2.0f);
    xboxr->setRot(0.0f, -45.0f, 0.0f);
    xboxr->setScale(6.0f, 1.0f, 1.0f);
    xboxr->color = blue;

    xx1->setPos(30.0f, 0.0f, 0.0f);
    xx2->setPos(30.0f, 0.0f, 0.0f);
    xx1->setRot(0.0f, 45.0f, 0.0f);
    xx2->setRot(0.0f, -45.0f, 0.0f);
    xx1->setScale(12.0f, 1.0f, 1.0f);
    xx2->setScale(12.0f, 1.0f, 1.0f);
    xx1->color = blue;
    xx2->color = blue;

    glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
    zbox->setPos(0.0f, 0.0f, 10.0f);
    zbox->setScale(1.0f, 1.0f, 20.0f);
    zbox->color = red;

    zboxl->setPos(2.0f, 0.0f, 18.0f);
    zboxl->setRot(0.0f, -45.0f, 0.0f);
    zboxl->setScale(1.0f, 1.0f, 6.0f);
    zboxl->color = red;

    zboxr->setPos(-2.0f, 0.0f, 18.0f);
    zboxr->setRot(0.0f, 45.0f, 0.0f);
    zboxr->setScale(1.0f, 1.0f, 6.0f);
    zboxr->color = red;

    zz1->setPos(0.0f, 0.0f, 26.0f);
    zz2->setPos(0.0f, 0.0f, 30.0f);
    zz3->setPos(0.0f, 0.0f, 34.0f);
    zz1->setScale(8.0f, 1.0f, 1.0f);
    zz2->setScale(1.0f, 1.0f, 11.0f);
    zz3->setScale(8.0f, 1.0f, 1.0f);
    zz2->setRot(0.0f, 45.0f, 0.0f);
    zz1->color = red;
    zz2->color = red;
    zz3->color = red;

    Transform* center = registerTransform();
    center->setPos(0.0f, 400.0f, 0.0f);

    // TODO figure out zclipping issues with this system
    // maybe just randomize the scale of each box used? based on hash?
    center->color = glm::vec3(0.0f, 0.0f, 0.0f);
    center->parentAll(xbox, xboxl, xboxr, zbox, zboxl, zboxr, xx1, xx2, zz1, zz2, zz3);
    center->setScale(glm::vec3(3.0f));
}

void Graphics::uploadTransforms() {
    for (Transform* t = nullptr; boxes.next(t);) {
        if (!t->shouldDraw()) {
            continue;
        }
        glm::vec3 color = DEBUG ? glm::vec3(1.0f, 0.0f, 1.0f) : t->color;
        Graphics::addToStream(DEBUG ? Shape::CUBE_SOLID : t->shape, t->getModelMatrix(), color);
    }
}

void Graphics::initGL(sf::RenderWindow& window) {
    // init glew (must be after window creation)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR::GLEW_INIT_FAILED!!!" << std::endl;
    }

    // setup OpenGL options
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    buildBuffers();

    // initialize quad for framebuffer rendering
    GLfloat quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    quadVAO = 0;
    glGenVertexArrays(1, &quadVAO);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glBindVertexArray(0);


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set black clear color
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

void Graphics::resize(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, WIDTH, HEIGHT);
    // rebuild buffers
    destroyBuffers();
    buildBuffers();
}

void Graphics::buildBuffers() {
    // initialize buffers
    for (GLuint i = 0; i < 2; i++) {
        // doesnt need depth if / when redo FBOs to have optional depth
        blurBuffers[i] = GLHelper::buildFBO(WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE);
    }
    sceneBuffer = GLHelper::buildFBO(WIDTH, HEIGHT);
    blurResult = GLHelper::buildFBO(WIDTH, HEIGHT);
}

void Graphics::destroyBuffers() {
    blurBuffers[0].destroy();
    blurBuffers[1].destroy();
    sceneBuffer.destroy();
    blurResult.destroy();
}

// render a fullscreen quad
// used to render into frame buffers
void Graphics::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// renders main scene (optionally to the scene buffer if blurring)
void Graphics::renderScene(Camera& cam, Terrain& terrain, bool toFrameBuffer) {
    Resources& r = Resources::get();
    if (toFrameBuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer.frame);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // set projection and view matrices
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 proj = cam.getProjMatrix(WIDTH, HEIGHT);

    // use solid cube shader
    r.instanceShader.use();
    glUniformMatrix4fv(glGetUniformLocation(r.instanceShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(r.instanceShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    if (DEBUG) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // upload boxes to corresponding streams
    uploadTransforms();

    // set and draw solid streams
    solidStream->setModels(smodels, true);
    solidStream->setColors(scolors, true);
    solidStream->render();

    // draw terrain using default shader and uploads building and trees to textured streams
    terrain.render(view, proj);

    // use instanced textured shader
    r.instanceTexShader.use();
    glUniformMatrix4fv(glGetUniformLocation(r.instanceTexShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(r.instanceTexShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    gridStream->setModels(gmodels, true);
    gridStream->setColors(gcolors, true);
    gridStream->render();

    pyrStream->setModels(pmodels, true);
    pyrStream->setColors(pcolors, true);
    pyrStream->render();

    // clear stream vectors
    smodels.clear();
    scolors.clear();
    gmodels.clear();
    gcolors.clear();
    pmodels.clear();
    pcolors.clear();

    // clear states
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void Graphics::finalProcessing(Camera& cam, bool blurring) {
    Resources& r = Resources::get();

    if (blurring) {
        // BLUR PASS
        glDisable(GL_DEPTH_TEST);
        blurColorBuffer(sceneBuffer.color, blurResult.frame, 4, r.screenShader, r.blurShader);
        glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer.frame);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // render skybox
    glEnable(GL_DEPTH_TEST);
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 proj = cam.getProjMatrix(WIDTH, HEIGHT);
    skybox->render(view, proj);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    if (!blurring) {
        return;
    }

    // FINAL PASS (combines blur buffer with scene buffer)
    glClear(GL_COLOR_BUFFER_BIT);
    r.blendShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneBuffer.color);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurResult.color);
    glActiveTexture(GL_TEXTURE0);
    // blur strength is how bright the blur is
    glUniform1f(glGetUniformLocation(r.blendShader.program, "blurStrength"), 3.0f);
    renderQuad();
}

// blur a color buffer for a given number of iterations
void Graphics::blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iterations, Shader screen, Shader blur) {

    // downsample into first blur buffer
    screen.use();
    glViewport(0, 0, WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE);
    glBindTexture(GL_TEXTURE_2D, sceneIn);
    glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[0].frame);
    renderQuad();

    blur.use();
    GLboolean horizontal = true;
    GLuint radLoc = glGetUniformLocation(blur.program, "radius");
    GLuint resLoc = glGetUniformLocation(blur.program, "resolution");
    GLuint dirLoc = glGetUniformLocation(blur.program, "dir");

    // nice weird numbers that dont line up lol
    GLuint lookups[4] = { 1, 3, 7, 13 };
    //GLuint lookups[4] = { 2, 5, 9, 17 };

    // iterations * 2 since does width then height
    // could have simplified below code but left it
    // incase we want directional blurs later
    for (GLuint i = 0; i < iterations * 2; i++) {
        GLuint blurRadius = i / 2 + 1;
        //blurRadius = pow(2, (i / 2 + 1)); //or pow(3,
        if (iterations <= 4) {
            blurRadius = lookups[i / 2];
        }

        glUniform1f(radLoc, static_cast<GLfloat>(blurRadius));
        glUniform1f(resLoc, horizontal ? static_cast<GLfloat>(WIDTH) : static_cast<GLfloat>(HEIGHT));
        GLfloat hz = horizontal ? 1.0f : 0.0f;
        glUniform2f(dirLoc, hz, 1.0f - hz);
        glBindTexture(GL_TEXTURE_2D, blurBuffers[!horizontal].color);
        glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[horizontal].frame);

        renderQuad();

        horizontal = !horizontal;
    }

    // upsample into blur result buffer
    screen.use();
    glViewport(0, 0, WIDTH, HEIGHT);
    glBindTexture(GL_TEXTURE_2D, blurBuffers[!horizontal].color);
    glBindFramebuffer(GL_FRAMEBUFFER, frameOut);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);	//back to default framebuffer
}


Graphics::~Graphics() {
    //for (size_t i = 0, len = meshes.size(); i < len; ++i) {
    //    delete meshes[i];
    //}
    delete skybox;
    destroyBuffers();
}

void Graphics::printGLErrors() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}

void Graphics::addToStream(Shape shape, glm::mat4& model, glm::vec3& color) {
    switch (shape) {
    case Shape::CUBE_SOLID:
        smodels.push_back(model);
        scolors.push_back(color);
        return;
    case Shape::CUBE_GRID:
        gmodels.push_back(model);
        gcolors.push_back(color);
        return;
    case Shape::PYRAMID:
        pmodels.push_back(model);
        pcolors.push_back(color);
        return;
    }
}

void Graphics::addToStream(Shape shape, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors) {
    int len = models.size();
    if (len == 0 || len != colors.size()) {
        return;
    }

    switch (shape) {
    case Shape::CUBE_SOLID:
        smodels.reserve(smodels.size() + len);
        smodels.insert(smodels.end(), models.begin(), models.end());
        scolors.reserve(scolors.size() + len);
        scolors.insert(scolors.end(), colors.begin(), colors.end());
        return;
    case Shape::CUBE_GRID:
        gmodels.reserve(gmodels.size() + len);
        gmodels.insert(gmodels.end(), models.begin(), models.end());
        gcolors.reserve(gcolors.size() + len);
        gcolors.insert(gcolors.end(), colors.begin(), colors.end());
        return;
    case Shape::PYRAMID:
        pmodels.reserve(pmodels.size() + len);
        pmodels.insert(pmodels.end(), models.begin(), models.end());
        pcolors.reserve(pcolors.size() + len);
        pcolors.insert(pcolors.end(), colors.begin(), colors.end());
        return;
    }
}
