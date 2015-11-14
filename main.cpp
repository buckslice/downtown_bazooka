
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

#include "shader.h"
#include "camera.h"
#include "cityGenerator.h"
#include "glHelper.h"
#include "physics.h"
#include "menu.h"
#include "game.h"


const GLuint WIDTH = 1440, HEIGHT = 960;
const sf::Vector2i center(WIDTH / 2, HEIGHT / 2);

FBO blurBuffers[2];
const GLuint BLUR_DOWNSAMPLE = 2;

glm::vec3 getMovementDir() {
	// calculate movement direction
	glm::vec3 dir(0.0f, 0.0f, 0.0f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		dir.z += 1.0f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::O) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		dir.z -= 1.0f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
		dir.x -= 1.0f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
		dir.x += 1.0f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
		dir.y -= 1.0f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		dir.y += 1.0f;
	}
	return dir;
}

sf::Vector2i getMouseMovement(sf::Window& window) {
	sf::Vector2i mouseMove;
	// get mouse movement
	if (window.hasFocus()) {
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		mouseMove = mousePos - center;

		// if the mouse has moved then set it back to center
		// needs to somehow prevent mouse going outside window in one frame
		// which is possible if you move fast enough
		if (mouseMove != sf::Vector2i(0, 0)) {
			sf::Mouse::setPosition(center, window);
		}
	}
	return mouseMove;
}

void checkForQuit(sf::Window& window, bool& running) {
	// check for events that will quit game
	sf::Event e;
	while (window.pollEvent(e)) {
		switch (e.type) {
		case sf::Event::Closed:
			running = false;
			break;
		case sf::Event::KeyPressed:
			if (e.key.code == sf::Keyboard::Escape) {
				running = false;
			}
			break;
		}
	}
}

// render a fullscreen quad
// used to render into frame buffers
GLuint quadVAO = 0;
void renderQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// blur a color buffer for a given number of iterations
void blurColorBuffer(GLuint sceneIn, GLuint frameOut, GLuint iterations, Shader screen, Shader blur) {

	// downsample into first blur buffer
	glViewport(0, 0, WIDTH / BLUR_DOWNSAMPLE, HEIGHT / BLUR_DOWNSAMPLE);
	screen.use();
	glBindTexture(GL_TEXTURE_2D, sceneIn);
	glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[0].frame);
	renderQuad();

	blur.use();
	GLboolean horizontal = true;
	GLuint radLoc = glGetUniformLocation(blur.program, "radius");
	GLuint resLoc = glGetUniformLocation(blur.program, "resolution");
	GLuint dirLoc = glGetUniformLocation(blur.program, "dir");

	if (iterations > 4) {
		return;
	}
	// nice weird numbers that dont line up lol
	int lookups[4] = { 1, 3, 7, 13 };
	//int lookups[4] = { 2, 5, 9, 17 };

	// iterations * 2 since does width then height
	// left rest in tho incase we want to blur directionally later
	for (GLuint i = 0; i < iterations * 2; i++) {
		glUniform1f(radLoc, lookups[i / 2]); //pow(2, (i / 2 + 1)) or pow(3,
		glUniform1f(resLoc, horizontal ? WIDTH : HEIGHT);
		GLfloat hz = horizontal ? 1.0f : 0.0f;
		glUniform2f(dirLoc, hz, 1.0f - hz);
		glBindTexture(GL_TEXTURE_2D, blurBuffers[!horizontal].color);
		glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[horizontal].frame);

		renderQuad();

		horizontal = !horizontal;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	screen.use();
	glBindTexture(GL_TEXTURE_2D, blurBuffers[!horizontal].color);
	glBindFramebuffer(GL_FRAMEBUFFER, frameOut);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);	//back to default framebuffer
}

sf::RenderWindow* initGL() {
	// window settings
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;

	//sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
	window->setFramerateLimit(60);

	// init glew (must be after window creation)
	glewExperimental = GL_TRUE;
	glewInit();

	// setup OpenGL options
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

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

	return window;
}

int main() {
	sf::RenderWindow* window = initGL();

	Menu menu(WIDTH, HEIGHT);

	Game game(1);

	// build and compile shaders
	// basic shader for buildings
	Shader buildingShader("assets/shaders/instanced.vert", "assets/shaders/default.frag");
	buildingShader.use();
	glUniform1i(glGetUniformLocation(buildingShader.program, "tex"), 0);

	// shader that blurs a colorbuffer
	Shader blurShader("assets/shaders/screen.vert", "assets/shaders/blur.frag");
	Shader screenShader("assets/shaders/screen.vert", "assets/shaders/screen.frag");
	screenShader.use();
	glUniform1i(glGetUniformLocation(screenShader.program, "screen"), 0);

	// shader that blends the blur with the scene
	Shader blendShader("assets/shaders/screen.vert", "assets/shaders/blend.frag");
	blendShader.use();
	glUniform1i(glGetUniformLocation(blendShader.program, "scene"), 0);
	glUniform1i(glGetUniformLocation(blendShader.program, "blur"), 1);

	GLuint tex = GLHelper::loadTexture("assets/images/grid.png");

	CityGenerator cg;
	Mesh mesh = cg.buildMesh(tex);

	// build main frame buffer
	// holds color and depth data
	FBO sceneBuffer = GLHelper::buildFBO(WIDTH, HEIGHT, true);
	FBO blurUpscaled = GLHelper::buildFBO(WIDTH, HEIGHT, true);

	// save matrix locations from shader
	GLint projLoc = glGetUniformLocation(buildingShader.program, "proj");
	GLint viewLoc = glGetUniformLocation(buildingShader.program, "view");

	// init camera and projection
	Camera cam(0.0f, 200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 270.0f, 0.0f);
	glm::mat4 proj = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 2000.0f);

	// set up some more stuff
	sf::Clock frameTime;
	sf::Clock animTime;
	sf::Mouse::setPosition(center, *window);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set black clear color
	Physics physics(cam);
	GLfloat flycd = 0.0f;

	// generate a random city
	cg.generateModelMatrices(true);
	cg.uploadModelMatrices(mesh);
	physics.addObjects(cg.boxes);

	bool running = true;
	while (running) {
		GLfloat deltaTime = frameTime.restart().asSeconds();

		checkForQuit(*window, running);

		// generate new square city
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
			physics.clearObjects();
			cg.generateModelMatrices(true);
			physics.addObjects(cg.boxes);
			cg.uploadModelMatrices(mesh);
		}
		// generate new circular city
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
			physics.clearObjects();
			cg.generateModelMatrices(false);
			physics.addObjects(cg.boxes);
			cg.uploadModelMatrices(mesh);
		}

		sf::Vector2i mouseMove = getMouseMovement(*window);

		// update camera
		if (game.hasStarted()) {
			cam.update(getMovementDir(), mouseMove.x, mouseMove.y, deltaTime);
		}
		else {
			//TODO make camera AI to scroll around the city
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			cam.jump();
		}
		flycd -= deltaTime;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && flycd < 0.0f) {
			cam.flying = !cam.flying;
			flycd = 1.0f;
		}

		// update physics
		physics.update(deltaTime);

		// RENDER SCENE TO FRAMEBUFFER
		glBindFramebuffer(GL_FRAMEBUFFER, sceneBuffer.frame);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		buildingShader.use();
		// set projection and view matrices
		glm::mat4 view = cam.getViewMatrix();
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// draw instanced mesh a bunch of times (sets model matrix internally)
		mesh.draw(buildingShader, NUMBER_OF_MESHES);

		// BLUR PASS
		glDisable(GL_DEPTH_TEST);	//dont need this now
		blurColorBuffer(sceneBuffer.color, blurUpscaled.frame, 4, screenShader, blurShader);

		// FINAL PASS (combines blur buffer with original scene buffer)
		glClear(GL_COLOR_BUFFER_BIT);
		blendShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneBuffer.color);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, blurUpscaled.color);
		// blur strength is how bright the blur is
		glUniform1f(glGetUniformLocation(blendShader.program, "blurStrength"), 3.0f);
		renderQuad();

		glBindVertexArray(0);
		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		window->resetGLStates();

		menu.update(*window, running, game);
		menu.draw(*window);

		// swap buffers
		window->display();
	}

	// clean up
	glDeleteTextures(1, &tex);
	glDeleteProgram(buildingShader.program);
	glDeleteProgram(blurShader.program);
	glDeleteProgram(blendShader.program);
	cg.destroy();
	//delete window;

	return 0;
}