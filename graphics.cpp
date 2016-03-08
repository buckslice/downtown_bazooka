#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics.h"
#include "resources.h"
#include "hsbColor.h"
#include "mathutil.h"
#include "glm/ext.hpp"

// so can declare at bottom of file
extern std::vector<Vertex> regVerts;
extern std::vector<Vertex> offsetVerts;
extern std::vector<GLuint> elems;

// have to do statics in implementation i guess??
static std::vector<Mesh*> meshes;
static Pool<Transform>* boxes;

static std::vector<glm::mat4> smodels;
static std::vector<glm::vec3> scolors;
static std::vector<glm::mat4> gmodels;
static std::vector<glm::vec3> gcolors;

// if you dont have this then breakpoints dont work! lol
Graphics::Graphics() {
}

int Graphics::registerTransform(bool solid) {
    int i = boxes->get();
    getTransform(i)->reset()->solid = solid;
    //std::cout << boxes->getFreeList().size() << std::endl;
    return i;
}

Transform* Graphics::getTransform(int id) {
    return boxes->getData(id);
}

// testing
Transform* sun;
Transform* planet;
Transform* moon;

Graphics::Graphics(sf::RenderWindow& window) {
    WIDTH = window.getSize().x;
    HEIGHT = window.getSize().y;

    initGL(window);

    solidBox = new Mesh(regVerts, elems, Resources::get().solidTex);    // TODO change to not use diff shader without texture
    gridBox = new Mesh(regVerts, elems, Resources::get().gridTex);

    boxes = new Pool<Transform>(10000);

    // testing out new transforms by setting up some arrows for the axes
    Transform* xbox = getTransform(registerTransform(false));
    Transform* xboxl = getTransform(registerTransform(false));
    Transform* xboxr = getTransform(registerTransform(false));
    Transform* xx1 = getTransform(registerTransform(false));
    Transform* xx2 = getTransform(registerTransform(false));

    Transform* zbox = getTransform(registerTransform(false));
    Transform* zboxl = getTransform(registerTransform(false));
    Transform* zboxr = getTransform(registerTransform(false));
    Transform* zz1 = getTransform(registerTransform(false));
    Transform* zz2 = getTransform(registerTransform(false));
    Transform* zz3 = getTransform(registerTransform(false));

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

    Transform* center = getTransform(registerTransform(false));
    center->setPos(0.0f, 400.0f, 0.0f);

    // TODO figure out zclipping issues with this system
    // maybe just randomize the scale of each box used? based on hash?
    center->color = glm::vec3(0.0f, 0.0f, 0.0f);
    center->parentAll(xbox, xboxl, xboxr, zbox, zboxl, zboxr, xx1, xx2, zz1, zz2, zz3);
    center->setScale(glm::vec3(3.0f));


    // now gonna make a cool "planet" demo lol
    sun = getTransform(registerTransform(false));
    sun->setScale(20.0f, 20.0f, 20.0f);
    sun->color = glm::vec3(1.0f, 1.0f, 0.3f);
    sun->setPos(100.0f, 150.0f, 100.0f);

    planet = getTransform(registerTransform(false));
    planet->setScale(glm::vec3(8.0f));
    planet->setPos(50.0f, 0.0f, 0.0f);
    planet->color = glm::vec3(0.0f, 0.6f, 0.4f);

    sun->parentAll(planet);

    moon = getTransform(registerTransform(false));
    moon->setScale(glm::vec3(4.0f));
    moon->setPos(0.0f, 15.0f, 0.0f);
    moon->color = glm::vec3(0.4f);

    planet->parentAll(moon);
}

void Graphics::uploadBoxes() {
    // the direction from child to parent has to be perpendicular to the 
    // axis of rotation of parent to have normal looking orbit
    // also without seperate transforms each child is basically tidally locked to parent but whatever lol
    sun->rotate(0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    planet->rotate(2.0f, glm::vec3(1.0f, 0.0f, 1.0f));
    moon->rotate(1.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    auto bx = boxes->getObjects();  // should make Pool class iterator
    for (size_t i = 0, len = bx.size(); i < len; ++i) {
        if (bx[i].id < 0 || !bx[i].data.shouldDraw()) { // iterator could do this check in ++ operator
            continue;
        }
        Transform& t = bx[i].data;
        // if debug rendering then set color to pink to show these are just the models (no collider info)
        glm::vec3 color = dstreamSize > 0 ? glm::vec3(1.0f, 0.0f, 1.0f) : t.color;
        Graphics::addToStream(t.solid, t.getModelMatrix(), color);
    }

}

void Graphics::initGL(sf::RenderWindow& window) {
    // init glew (must be after window creation)
    glewExperimental = GL_TRUE;
    glewInit();

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

    floorMesh = new Mesh(regVerts, elems, Resources::get().gridTex);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set black clear color
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

void Graphics::resize(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, WIDTH, HEIGHT);
    // destroy buffers and then rebuild
    blurBuffers[0].destroy();
    blurBuffers[1].destroy();
    sceneBuffer.destroy();
    blurResult.destroy();

    buildBuffers();
}

void Graphics::buildBuffers() {
    // initialize buffers
    for (GLuint i = 0; i < 2; i++) {
        blurBuffers[i] = GLHelper::buildFBO(WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE, false);
    }
    sceneBuffer = GLHelper::buildFBO(WIDTH, HEIGHT, true);
    blurResult = GLHelper::buildFBO(WIDTH, HEIGHT, true);
}

// render a fullscreen quad
// used to render into frame buffers
void Graphics::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// RENDER SCENE TO FRAMEBUFFER
void Graphics::renderScene(Camera& cam, Terrain& tg, bool toFrameBuffer) {
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

    r.instanceShader.use();
    // set projection and view matrices
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 proj = cam.getProjMatrix(WIDTH, HEIGHT);

    // draw all instanced cube meshes
    glUniformMatrix4fv(glGetUniformLocation(r.instanceShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(r.instanceShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    if (dstreamSize > 0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        solidBox->visible = true;
        solidBox->setInstanceAmount(dstreamSize);
        if (!solidBox->builtModels) {
            Graphics::genModelBuffer(solidBox);
        }
        if (!solidBox->builtColors) {
            Graphics::genColorBuffer(solidBox);
        }
        glBindBuffer(GL_ARRAY_BUFFER, solidBox->modelBuffer);
        glBufferData(GL_ARRAY_BUFFER, dstreamSize * sizeof(glm::mat4), &(*dmodels)[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, solidBox->colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, dstreamSize * sizeof(glm::vec3), &(*dcolors)[0], GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        solidBox->draw();

    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (size_t i = 0, len = meshes.size(); i < len; ++i) {
            if (meshes[i]->visible) {
                meshes[i]->draw();
            }
        }
    }

    uploadBoxes();

    // should just make solidbox a different model without textures
    Graphics::setStream(solidBox, smodels, scolors);
    Graphics::setStream(gridBox, gmodels, gcolors);
    if (solidBox->visible) {
        solidBox->draw();
    }
    if (gridBox->visible) {
        gridBox->draw();
    }

    // draw terrain
    r.terrainShader.use();
    glUniformMatrix4fv(glGetUniformLocation(r.terrainShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(r.terrainShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    tg.render();

    // clear stream vectors
	smodels.clear();
    scolors.clear();
    gmodels.clear();
    gcolors.clear();

    // clear states
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void Graphics::postProcess() {
    Resources& r = Resources::get();

    // BLUR PASS
    glDisable(GL_DEPTH_TEST);	//dont need this now
    blurColorBuffer(sceneBuffer.color, blurResult.frame, 4, r.screenShader, r.blurShader);

    // FINAL PASS (combines blur buffer with original scene buffer)
    glClear(GL_COLOR_BUFFER_BIT);
    r.blendShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneBuffer.color);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurResult.color);
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
    for (size_t i = 0, len = meshes.size(); i < len; ++i) {
        glDeleteBuffers(1, &meshes[i]->colorBuffer);
        glDeleteBuffers(1, &meshes[i]->modelBuffer);
        delete meshes[i];
    }
    meshes.erase(meshes.begin(), meshes.end());

    delete floorMesh;
}

void Graphics::addToStream(bool solid, glm::mat4& model, glm::vec3& color) {
    if (solid) {
        smodels.push_back(model);
        scolors.push_back(color);
    } else {
        gmodels.push_back(model);
        gcolors.push_back(color);
    }
}

void Graphics::addToStream(bool solid, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors) {
    int len = models.size();
    if (len == 0 || len != colors.size()) {
        return;
    }

    if (solid) {
        smodels.reserve(smodels.size() + len);
        smodels.insert(smodels.end(), models.begin(), models.end());
        scolors.reserve(scolors.size() + len);
        scolors.insert(scolors.end(), colors.begin(), colors.end());
    } else {
        gmodels.reserve(gmodels.size() + len);
        gmodels.insert(gmodels.end(), models.begin(), models.end());
        gcolors.reserve(gcolors.size() + len);
        gcolors.insert(gcolors.end(), colors.begin(), colors.end());
    }

}

void Graphics::setDebugStream(GLuint size, std::vector<glm::mat4>* models, std::vector<glm::vec3>* colors) {
    dstreamSize = size;
    this->dmodels = models;
    this->dcolors = colors;
}

void Graphics::setStream(Mesh* m, std::vector<glm::mat4>& models, std::vector<glm::vec3>& colors) {
    int len = models.size();
    if (len == 0 || len != colors.size()) {
        m->visible = false;
        return;
    }
    m->visible = true;

    m->setInstanceAmount(len);
    if (!m->builtModels) {
        Graphics::genModelBuffer(m);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m->modelBuffer);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(glm::mat4), &models[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m->builtColors) {
        Graphics::genColorBuffer(m);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m->colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(glm::vec3), &colors[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Graphics::setColors(GLuint mesh_id, std::vector<glm::vec3>& colors) {
    if (colors.size() == 0) {
        return;
    }

    Mesh* m = meshes[mesh_id];
    if (!m->builtColors) {
        Graphics::genColorBuffer(m);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m->colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Graphics::setModels(GLuint mesh_id, std::vector<glm::mat4>& models) {
    Mesh* m = meshes[mesh_id];
    if (models.size() == 0) {
        m->visible = false;
        return;
    }
    m->visible = true;

    m->setInstanceAmount(models.size());
    if (!m->builtModels) {
        Graphics::genModelBuffer(m);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m->modelBuffer);
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), &models[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// builds color buffer
void Graphics::genColorBuffer(Mesh* mesh) {
    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBindVertexArray(mesh->getVAO());
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
    glVertexAttribDivisor(2, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->colorBuffer = colorBuffer;
    mesh->builtColors = true;
}

// builds model buffer for instanced mesh
void Graphics::genModelBuffer(Mesh* mesh) {
    GLuint modelBuffer;
    glGenBuffers(1, &modelBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
    glBindVertexArray(mesh->getVAO());
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->modelBuffer = modelBuffer;
    mesh->builtModels = true;
}

GLuint Graphics::registerMesh() {
    return registerMesh(Resources::get().gridTex);
}

// no way to delete meshes currently but can just set
// visibility to false if you need
// really no need to delete mesh ever
// these methods are pretty retarded actually wtf am i doing?
GLuint Graphics::registerMesh(GLuint tex) {

    meshes.push_back(new Mesh(offsetVerts, elems, tex));

    return meshes.size() - 1;

}

void Graphics::setMeshVisible(GLuint id, bool value) {
    if (!isValidMeshID(id)) {
        return;
    }
    meshes[id]->visible = value;
}

bool Graphics::isValidMeshID(GLuint id) {
    if (id < 0 || id >= meshes.size()) {
        std::cout << "ERROR::MESH_ID_INVALID" << std::endl;
        return false;
    }
    return true;
}


// helps make square texture look better on buildings
GLfloat _vn = 1.0f / 32.0f;

// each uv starts in bottom left (when looking at face) and progresses clockwise around
std::vector<Vertex> regVerts = {
    //front
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f)},
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    //back
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    //left
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    //right
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    //bottom
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    //top
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },

};

std::vector<Vertex> offsetVerts = {
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },

    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },

};

std::vector<GLuint> elems = {
    0,1,2,
    2,3,0,

    4,5,6,
    6,7,4,

    8,9,10,
    10,11,8,

    12,13,14,
    14,15,12,

    16,17,18,
    18,19,16,

    20,21,22,
    22,23,20
};