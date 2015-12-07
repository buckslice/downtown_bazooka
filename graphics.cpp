#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics.h"
#include "resources.h"

// so can declare at bottom of file
//extern GLfloat vertices[];
//extern GLuint elements[];

extern std::vector<Vertex> regVerts;
extern std::vector<Vertex> offsetVerts;
extern std::vector<GLuint> elems;

GLuint WIDTH;
GLuint HEIGHT;

// have to do statics in implementation i guess??
static std::vector<Mesh*> meshes;

Graphics::Graphics(sf::RenderWindow& window) {
    WIDTH = window.getSize().x;
    HEIGHT = window.getSize().y;

    initGL(window);
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

    // more stuff
    buildShaders();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set black clear color

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
void Graphics::renderScene(Camera& cam) {
    glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer.frame);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    instanceShader.use();
    // set projection and view matrices
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 proj = cam.getProjMatrix(WIDTH, HEIGHT);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    for (int i = 0; i < meshes.size(); i++) {
        if (meshes[i]->visible) {
            meshes[i]->draw();
        }
    }

    tiledShader.use();
    glUniformMatrix4fv(glGetUniformLocation(tiledShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(tiledShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    floorMesh->draw();

    // clear states
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Graphics::postProcess() {
    // BLUR PASS
    glDisable(GL_DEPTH_TEST);	//dont need this now
    blurColorBuffer(sceneBuffer.color, blurResult.frame, 4, screenShader, blurShader);

    // FINAL PASS (combines blur buffer with original scene buffer)
    glClear(GL_COLOR_BUFFER_BIT);
    blendShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneBuffer.color);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurResult.color);
    // blur strength is how bright the blur is
    glUniform1f(glGetUniformLocation(blendShader.program, "blurStrength"), 3.0f);
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

        glUniform1f(radLoc, blurRadius);
        glUniform1f(resLoc, horizontal ? WIDTH : HEIGHT);
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


bool loadedShadersBefore = false;

void Graphics::buildShaders() {
    bool success = true;
    success &= instanceShader.build("assets/shaders/instanced.vert", "assets/shaders/default.frag");
    instanceShader.use();
    glUniform1i(glGetUniformLocation(instanceShader.program, "tex"), 0);
    // save matrix locations from shader
    projLoc = glGetUniformLocation(instanceShader.program, "proj");
    viewLoc = glGetUniformLocation(instanceShader.program, "view");

    success &= tiledShader.build("assets/shaders/default.vert", "assets/shaders/tiled.frag");
    tiledShader.use();
    glUniform1i(glGetUniformLocation(tiledShader.program, "tex"), 0);
    //glUniform3f(glGetUniformLocation(tiledShader.program, "Color"), 1.0f, 0.0f, 0.0f);
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2000.0f, 10.0f, 2000.0f));  // too lazy to import citygen for CITY_SIZE lol
    glUniformMatrix4fv(glGetUniformLocation(tiledShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // shaders that blurs a colorbuffer
    success &= blurShader.build("assets/shaders/screen.vert", "assets/shaders/blur.frag");
    success &= screenShader.build("assets/shaders/screen.vert", "assets/shaders/screen.frag");
    screenShader.use();
    glUniform1i(glGetUniformLocation(screenShader.program, "screen"), 0);

    // shader that blends the blur with the scene
    success &= blendShader.build("assets/shaders/screen.vert", "assets/shaders/blend.frag");
    blendShader.use();
    glUniform1i(glGetUniformLocation(blendShader.program, "scene"), 0);
    glUniform1i(glGetUniformLocation(blendShader.program, "blur"), 1);

    if (success && loadedShadersBefore) {
        std::cout << "SHADERS::RECOMPILE::SUCCESS" << std::endl;
    }
    loadedShadersBefore = true;
}

void Graphics::deleteShaders() {
    if (!loadedShadersBefore) {
        return;
    }
    glDeleteProgram(instanceShader.program);
    glDeleteProgram(tiledShader.program);
    glDeleteProgram(blurShader.program);
    glDeleteProgram(screenShader.program);
    glDeleteProgram(blendShader.program);
}

Graphics::~Graphics() {
    deleteShaders();
    for (int i = 0; i < meshes.size(); i++) {
        glDeleteBuffers(1, &meshes[i]->colorBuffer);
        glDeleteBuffers(1, &meshes[i]->modelBuffer);
        delete meshes[i];
    }
    meshes.erase(meshes.begin(), meshes.end());

    delete floorMesh;
}

// should eventually just reuse same buffer lol
void Graphics::setColors(GLuint mesh_id, std::vector<glm::vec3>& colors) {
    if (colors.size() == 0) {
        return;
    }

    Mesh* m = meshes[mesh_id];
    if (m->builtColors) {
        glDeleteBuffers(1, &m->colorBuffer);
    }
    m->colorBuffer = Graphics::genColorBuffer(m, colors);
}

void Graphics::setModels(GLuint mesh_id, std::vector<glm::mat4>& models) {
    Mesh* m = meshes[mesh_id];
    if (models.size() == 0) {
        m->visible = false;
        return;
    }
    m->visible = true;

    if (m->builtModels) {
        glDeleteBuffers(1, &m->modelBuffer);
    }
    m->modelBuffer = Graphics::genModelBuffer(m, models);
}

GLuint Graphics::genColorBuffer(Mesh* mesh, std::vector<glm::vec3>& colors) {
    mesh->builtColors = true;
    //mesh.setInstanceAmount(colors.size());

    GLuint VAO = mesh->getVAO();
    GLuint colorBuffer;
    glBindVertexArray(VAO);

    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
    return colorBuffer;
}

GLuint Graphics::genModelBuffer(Mesh* mesh, std::vector<glm::mat4>& models) {
    mesh->builtModels = true;
    mesh->setInstanceAmount(models.size());
    GLuint VAO = mesh->getVAO();
    GLuint modelBuffer;
    glBindVertexArray(VAO);

    //glDeleteBuffers(1, &modelBuffer);
    glGenBuffers(1, &modelBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), &models[0], GL_STATIC_DRAW);
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
    return modelBuffer;
}

//void Graphics::updateColorBuffer(GLuint buffer, std::vector<glm::vec3>& colors) {
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glBufferSubData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
//}
//
//void Graphics::updateModelBuffer(GLuint buffer, std::vector<glm::mat4>& models) {
//
//}

GLuint Graphics::registerMesh() {
    return registerMesh(Resources::get().gridTex);
}

// no way to delete meshes currently but can just set
// visibility to false if you need
// really no need to delete mesh ever
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
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    //back
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    //left
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    //right
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    //bottom
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    //top
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },

};

std::vector<Vertex> offsetVerts = {
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, _vn) },

    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },

    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, _vn) },

    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },

    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },

    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, _vn) },

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