#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "graphics.h"

Graphics::Graphics(sf::RenderWindow& window) {
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

    // initialize blur color buffers
    for (GLuint i = 0; i < 2; i++) {
        blurBuffers[i] = GLHelper::buildFBO(WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE, false);
    }

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

    // more stuff
    buildShaders();

    // build main frame buffer
    // holds color and depth data
    sceneBuffer = GLHelper::buildFBO(WIDTH, HEIGHT, true);
    blurResult = GLHelper::buildFBO(WIDTH, HEIGHT, true);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set black clear color
}

// render a fullscreen quad
// used to render into frame buffers
void Graphics::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Graphics::renderScene(Camera& cam) {
    // RENDER SCENE TO FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer.frame);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    buildingShader.use();
    // set projection and view matrices
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 proj = cam.getProjMatrix(WIDTH, HEIGHT);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // draw instanced mesh a bunch of times (sets model matrix internally)
    buildingMesh->draw(buildingShader);

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
    success = success && buildingShader.build("assets/shaders/instanced.vert", "assets/shaders/default.frag");
    buildingShader.use();
    glUniform1i(glGetUniformLocation(buildingShader.program, "tex"), 0);
    // save matrix locations from shader
    projLoc = glGetUniformLocation(buildingShader.program, "proj");
    viewLoc = glGetUniformLocation(buildingShader.program, "view");

    // shader that blurs a colorbuffer
    success = success && blurShader.build("assets/shaders/screen.vert", "assets/shaders/blur.frag");
    success = success && screenShader.build("assets/shaders/screen.vert", "assets/shaders/screen.frag");
    screenShader.use();
    glUniform1i(glGetUniformLocation(screenShader.program, "screen"), 0);

    // shader that blends the blur with the scene
    success = success && blendShader.build("assets/shaders/screen.vert", "assets/shaders/blend.frag");
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
    glDeleteProgram(buildingShader.program);
    glDeleteProgram(blurShader.program);
    glDeleteProgram(screenShader.program);
    glDeleteProgram(blendShader.program);
}

Graphics::~Graphics() {
    deleteShaders();
}