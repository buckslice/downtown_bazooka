
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

#include <stdlib.h>


#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "aabb.h"
#include "mathutil.h"


const GLuint WIDTH = 1440, HEIGHT = 960;
const GLuint NUMBER_OF_MESHES = 7500;
const GLuint NUMBER_OF_VERTICES = 120;
const GLuint FLOATS_PER_VERTEX = 5;
const float CITY_SIZE = 2000.0f;
const float BLEND_DISTANCE = 500.0f;
const sf::Vector2i center(WIDTH / 2, HEIGHT / 2);
glm::mat4* modelMatrices = new glm::mat4[NUMBER_OF_MESHES];
glm::vec3* colors = new glm::vec3[NUMBER_OF_MESHES];
GLuint blurColorBuffers[2];
GLuint blurFrameBuffers[2];

// helps make square texture look better on buildings
GLfloat vn = 1.0f / 32.0f;
GLfloat vertices[] = {
	// front
	-0.5f, -0.5f, -0.5f,  0.0f, vn,
	0.5f, -0.5f, -0.5f,  1.0f, vn,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f - vn,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f - vn,

	// back
	-0.5f, -0.5f,  0.5f,  0.0f, vn,
	0.5f, -0.5f,  0.5f,  1.0f, vn,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f - vn,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f - vn,

	// left
	-0.5f,  0.5f,  0.5f,  0.0f, vn,
	-0.5f,  0.5f, -0.5f,  1.0f, vn,
	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f - vn,
	-0.5f, -0.5f,  0.5f,  0.0f, 1.0f - vn,

	// right
	0.5f,  0.5f,  0.5f,  0.0f, vn,
	0.5f,  0.5f, -0.5f,  1.0f, vn,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f - vn,
	0.5f, -0.5f,  0.5f,  0.0f, 1.0f - vn,

	// bottom
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	// top
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
};

GLuint elements[] = {
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


GLuint loadTexture() {
	GLuint tex;
	glGenTextures(1, &tex);
	sf::Image image;
	image.loadFromFile("assets/images/grid.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

Mesh buildMesh(GLuint& tex) {
	std::vector<Vertex> verts;
	for (int i = 0; i < NUMBER_OF_VERTICES / FLOATS_PER_VERTEX; i++) {
		Vertex v;
		int j = i * FLOATS_PER_VERTEX;
		v.position = glm::vec3(vertices[j], vertices[j + 1], vertices[j + 2]);
		v.texcoord = glm::vec2(vertices[j + 3], vertices[j + 4]);

		verts.push_back(v);
	}
	std::vector<GLuint> tris(elements, elements + sizeof(elements) / sizeof(GLuint));
	return Mesh(verts, tris, tex);
}

// checks if box collides with any of the boxes on the list
bool collidesWithAny(AABB box, std::vector<AABB> boxes) {
	for (int i = 0; i < boxes.size(); i++) {
		if (box.getIntersect(boxes[i]).doesIntersect) {
			return true;
		}
	}
	return false;
}

bool tooClose(float minDist, glm::vec2 city, std::vector<glm::vec2> cities) {
	for (int i = 0; i < cities.size(); i++) {
		if (glm::distance(city, cities[i]) < minDist) {
			return true;
		}
	}
	return false;
}

float rnd() {
	return (double)rand() / RAND_MAX;
}

void generateModelMatrices(Shader& shader, bool square) {
	srand(time(NULL));

	std::vector<AABB> boxes;
	std::vector<glm::vec2> cities;

	// generate some random city centers
	const int numCitys = 8;
	for (GLuint i = 0; i < numCitys; i++) {
		glm::vec2 city;
		int tries = 0;
		do {
			if (square) {
				city = MathUtil::randomPointInSquare(CITY_SIZE);
			} else {
				city = MathUtil::randomPointInCircle(CITY_SIZE / 2.0f);
			}
			tries++;
		} while (tooClose(400.0f, city, cities) && tries < 100);

		cities.push_back(city);
	}

	// generate randomly located buildings
	// sized based on their distance to nearest city center
	for (GLuint i = 0; i < NUMBER_OF_MESHES; i++) {
		glm::mat4 model;
		glm::mat4 model1;

		float sx, sy, sz;
		int tries = 0;
		glm::vec2 p;
		while (tries < 100) {
			if (square) {
				p = MathUtil::randomPointInSquare(CITY_SIZE);
			} else {
				p = MathUtil::randomPointInCircle(CITY_SIZE / 2.0f);
			}

			float distToClosestCity = std::numeric_limits<float>::max();
			glm::vec2 closestCity;
			for (int i = 0; i < cities.size(); i++) {
				distToClosestCity = std::min(distToClosestCity, glm::distance(p, cities[i]));
			}
			float d = distToClosestCity;
			float b = MathUtil::blend(d, BLEND_DISTANCE, 0.0f, MathUtil::cubic);
			if (d > BLEND_DISTANCE) {
				b = 0.0f;	// lol comment this
			}

			sx = MathUtil::randFloat() * 10.0f + b * 20.0f + 5.0f;
			sy = MathUtil::randFloat() * 10.0f + b * 100.0f + 5.0f;
			sz = MathUtil::randFloat() * 10.0f + b * 20.0f + 5.0f;

			AABB box(glm::vec3(p.x - sx / 2.0, 0.0f, p.y - sz / 2.0), glm::vec3(p.x + sx / 2.0, sy, p.y + sz / 2.0));
			if (!collidesWithAny(box, boxes)) {
				boxes.push_back(box);
				break;
			}

			tries++;
		}

		glm::vec3 pos = glm::vec3(p.x, sy / 2.0f, p.y);
		glm::vec3 scale = glm::vec3(sx, sy, sz);
		model = glm::translate(model, pos);
		model = glm::scale(model, scale);

		modelMatrices[i] = model;

		colors[i] = rand() % 1000 == 1 ? glm::vec3(.5f, 0, 1.0f) : rand() % 100 == 1 ? glm::vec3(1.0f, 0, 0) : rand() % 2 == 0 ? glm::vec3(0, 1.0f, rnd()*.5f + .5f) : glm::vec3(0, rnd()*.5f + .5f, 1.0f);//MathUtil::generateRandomColor();
	}
}

// set up instancing uniforms
// basically uploads all the model matrices to mesh
void uploadModelMatrices(Mesh& mesh) {
	GLuint VAO = mesh.getVAO();
	GLuint modelBuffer;
	GLuint colorBuffer;
	glBindVertexArray(VAO);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, NUMBER_OF_MESHES * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glVertexAttribDivisor(2, 1);

	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, NUMBER_OF_MESHES * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
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

GLuint buildColorBuffer() {
	GLuint cbo;
	glGenTextures(1, &cbo);
	glBindTexture(GL_TEXTURE_2D, cbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return cbo;
}

GLuint buildFrameBuffer(GLuint colorBuffer, bool withDepth) {
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	if (withDepth) {
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::MAIN::FRAMEBUFFER_NOT_COMPLETE" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
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
		glUniform1f(radLoc, i);
		glUniform1f(resLoc, horizontal ? WIDTH : HEIGHT);
		GLfloat hz = horizontal ? 1.0f : 0.0f;
		glUniform2f(dirLoc, hz, 1.0f - hz);
		glBindTexture(GL_TEXTURE_2D, first ? colorBuffer : blurColorBuffers[!horizontal]);
		glBindFramebuffer(GL_FRAMEBUFFER, blurFrameBuffers[horizontal]);

		renderQuad();

		horizontal = !horizontal;
		first = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	//back to default framebuffer
	return blurColorBuffers[!horizontal];
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
		blurColorBuffers[i] = buildColorBuffer();
		blurFrameBuffers[i] = buildFrameBuffer(blurColorBuffers[i], false);
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

	GLuint tex = loadTexture();
	Mesh mesh = buildMesh(tex);

	// generate a random city
	generateModelMatrices(buildingShader, true);
	uploadModelMatrices(mesh);

	// build main frame buffer
	// holds color and depth data
	GLuint sceneColorBuffer = buildColorBuffer();
	GLuint sceneFrameBuffer = buildFrameBuffer(sceneColorBuffer, true);

	// save matrix locations from shader
	GLint projLoc = glGetUniformLocation(buildingShader.program, "proj");
	GLint viewLoc = glGetUniformLocation(buildingShader.program, "view");

	// init camera and projection
	Camera cam(0.0f, 200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 270.0f, 0.0f);
	glm::mat4 proj = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 2000.0f);

	// set up some more stuff
	sf::Clock frameTime;
	sf::Mouse::setPosition(center, *window);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // set black clear color

	bool running = true;
	while (running) {
		GLfloat deltaTime = frameTime.restart().asSeconds();

		checkForQuit(*window, running);

		// generate new square city
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
			generateModelMatrices(buildingShader, true);
			uploadModelMatrices(mesh);
		}
		// generate new circular city
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
			generateModelMatrices(buildingShader, false);
			uploadModelMatrices(mesh);
		}

		sf::Vector2i mouseMove = getMouseMovement(*window);

		// update camera
		cam.update(getMovementDir(), mouseMove.x, mouseMove.y, deltaTime);

		// RENDER SCENE TO FRAMEBUFFER
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFrameBuffer);
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
		GLuint blurred = blurColorBuffer(sceneColorBuffer, 10, blurShader);

		// FINAL PASS (combines blur buffer with original scene buffer)
		glClear(GL_COLOR_BUFFER_BIT);
		blendShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, blurred);
		renderQuad();

		// swap buffers
		window->display();
	}

	// clean up
	glDeleteTextures(1, &tex);
	glDeleteProgram(buildingShader.program);
	delete[] modelMatrices;
	delete[] colors;
	//delete window;

	return 0;
}