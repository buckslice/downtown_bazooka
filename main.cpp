
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


const GLuint WIDTH = 1440, HEIGHT = 960;
const sf::Vector2i center(WIDTH / 2, HEIGHT / 2);

FBO blurBuffers[2];

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
GLuint blurColorBuffer(GLuint colorBuffer, GLuint iterations, Shader blur) {
	blur.use();
	GLboolean horizontal = true, first = true;
	GLuint radLoc = glGetUniformLocation(blur.program, "radius");
	GLuint resLoc = glGetUniformLocation(blur.program, "resolution");
	GLuint dirLoc = glGetUniformLocation(blur.program, "dir");
	for (GLuint i = 0; i < iterations; i++) {
		glUniform1f(radLoc, i*1.1f);
		glUniform1f(resLoc, horizontal ? WIDTH : HEIGHT);
		GLfloat hz = horizontal ? 1.0f : 0.0f;
		glUniform2f(dirLoc, hz, 1.0f - hz);
		glBindTexture(GL_TEXTURE_2D, first ? colorBuffer : blurBuffers[!horizontal].color);
		glBindFramebuffer(GL_FRAMEBUFFER, blurBuffers[horizontal].frame);

		renderQuad();

		horizontal = !horizontal;
		first = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	//back to default framebuffer
	return blurBuffers[!horizontal].color;
}

sf::Window* initGL() {
	// window settings
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;

	//sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
	sf::Window* window = new sf::Window(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
	window->setFramerateLimit(60);

	// init glew (must be after window creation)
	glewExperimental = GL_TRUE;
	glewInit();

	// setup OpenGL options
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

	// initialize blur color buffers
	for (GLuint i = 0; i < 2; i++) {
		blurBuffers[i] = GLHelper::buildFBO(WIDTH, HEIGHT, false);
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
	sf::Window* window = initGL();

	// build and compile shaders
	// basic shader for buildings
	Shader buildingShader("assets/shaders/instanced.vert", "assets/shaders/default.frag");
	buildingShader.use();
	glUniform1i(glGetUniformLocation(buildingShader.program, "tex"), 0);

	// shader that blurs a colorbuffer
	Shader blurShader("assets/shaders/screen.vert", "assets/shaders/blur.frag");

	// shader that blends the blur with the scene
	Shader blendShader("assets/shaders/screen.vert", "assets/shaders/blend.frag");
	blendShader.use();
	glUniform1i(glGetUniformLocation(blendShader.program, "scene"), 0);
	glUniform1i(glGetUniformLocation(blendShader.program, "blur"), 1);

	GLuint tex = GLHelper::loadTexture("assets/images/grid.png");

	CityGenerator cg;
	Mesh mesh = cg.buildMesh(tex);

	// generate a random city
	cg.generateModelMatrices(true);
	cg.uploadModelMatrices(mesh);

	// build main frame buffer
	// holds color and depth data
	FBO sceneBuffer = GLHelper::buildFBO(WIDTH, HEIGHT, true);

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
	physics.addObjects(cg.boxes);

	bool running = true;
	while (running) {
		GLfloat deltaTime = frameTime.restart().asSeconds();

		checkForQuit(*window, running);

		// generate new square city
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
			physics.clearObjects();
			cg.generateModelMatrices(true);
			cg.uploadModelMatrices(mesh);
		}
		// generate new circular city
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
			physics.clearObjects();
			cg.generateModelMatrices(false);
			cg.uploadModelMatrices(mesh);
		}

		sf::Vector2i mouseMove = getMouseMovement(*window);

		// update camera
		cam.update(getMovementDir(), mouseMove.x, mouseMove.y, deltaTime);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			cam.jump();
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
		GLuint blurred = blurColorBuffer(sceneBuffer.color, 10, blurShader);

		// FINAL PASS (combines blur buffer with original scene buffer)
		glClear(GL_COLOR_BUFFER_BIT);
		blendShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneBuffer.color);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, blurred);
		glUniform1f(glGetUniformLocation(blendShader.program, "blurStrength"), 3.0f);
		renderQuad();

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