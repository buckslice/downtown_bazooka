
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <stdlib.h>	//rand

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"


const GLuint WIDTH = 1024, HEIGHT = 768;

GLfloat vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

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

// TODO: MAKE A STATIC MATH CLASS OR SOMETHING FOR THESE NEXT COUPLE FUNCTIONS
// generates random float from 0.0 to 1.0 inclusive
float randFloat() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float cubicSCurve(float value) {
	return value * value * (3.0f - 2.0f * value);
}

float cubic(float value) {
	return value * value * value;
}

float quadric(float value) {
	return value * value * value * value;
}

float blend(float d, float low, float high) {
	return cubic((d - low) / (high - low));
}
////////////////////////////////////////////////////////////////////////////

int main() {
	// window settings
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 2;

	sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "DOWNTOWN BAZOOKA", sf::Style::Close, settings);
	window.setFramerateLimit(60);

	// init glew (must be after window creation)
	glewExperimental = GL_TRUE;
	glewInit();

	// setup OpenGL options
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

	// build and compile shader
	//Shader shader("assets/shaders/default.vert", "assets/shaders/default.frag");
	Shader shader("assets/shaders/instanced.vert", "assets/shaders/default.frag");
	shader.use();

	// load texture
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	sf::Image image;
	image.loadFromFile("assets/images/grid.png");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// build mesh from data
	std::vector<Vertex> verts;
	for (int i = 0; i < 24; i++) {
		Vertex v;
		int j = i * 5;
		v.position = glm::vec3(vertices[j], vertices[j + 1], vertices[j + 2]);
		v.texcoord = glm::vec2(vertices[j + 3], vertices[j + 4]);

		verts.push_back(v);
	}
	std::vector<GLuint> tris(elements, elements + sizeof(elements) / sizeof(GLuint));
	Mesh mesh(verts, tris, tex);

	// set up projection matrix
	GLint projLoc = glGetUniformLocation(shader.program, "proj");
	glm::mat4 proj = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 2000.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// save view matrix location in shader
	// generated and set each frame from camera
	GLint viewLoc = glGetUniformLocation(shader.program, "view");

	// set up model matrices and colors for instanced buildings
	GLuint numberOfMeshes = 10000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[numberOfMeshes];
	glm::vec3* colors;
	colors = new glm::vec3[numberOfMeshes];

	srand(time(NULL));
	for (GLuint i = 0; i < numberOfMeshes; i++) {
		glm::mat4 model;

		float size = 2000.0f;
		float x = randFloat() * size - size / 2.0f;
		float z = randFloat() * size - size / 2.0f;

		float d = glm::distance(glm::vec2(x, z), glm::vec2(0.0f, 0.0f));
		float blnd = blend(d, 500.0f, 0.0f);
		if (d > 500.0f) {
			blnd = 0.0f;	// lol comment this
		}

		float sx = randFloat() * 10.0f + blnd * 20.0f + 5.0f;
		float sy = randFloat() * 10.0f + blnd * 100.0f + 5.0f;
		float sz = randFloat() * 10.0f + blnd * 20.0f + 5.0f;

		model = glm::translate(model, glm::vec3(x, sy / 2.0f, z));
		model = glm::scale(model, glm::vec3(sx, sy, sz));

		modelMatrices[i] = model;

		GLfloat r = rand() % 101;
		GLfloat g = rand() % 101;
		GLfloat b = rand() % 101;
		colors[i] = glm::vec3(r / 100.0f, g / 100.0f, b / 100.0f);
	}

	// set up instancing uniforms
	// basically uploads all the model matrices to mesh
	GLuint VAO = mesh.getVAO();
	GLuint modelBuffer;
	GLuint colorBuffer;
	glBindVertexArray(VAO);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, numberOfMeshes * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glVertexAttribDivisor(2, 1);

	glGenBuffers(1, &modelBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
	glBufferData(GL_ARRAY_BUFFER, numberOfMeshes * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
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

	// set up some more stuff
	//sf::Clock animTime;
	sf::Clock frameTime;
	Camera cam(0.0f, 200.0f, 0.0f, 0.0f, 1.0f, 0.0f, 270.0f, 0.0f);
	sf::Vector2i center(WIDTH / 2, HEIGHT / 2);
	sf::Mouse::setPosition(center, window);

	bool running = true;
	while (running) {
		GLfloat deltaTime = frameTime.restart().asSeconds();

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

		// update camera
		cam.update(getMovementDir(), mouseMove.x, mouseMove.y, deltaTime);

		// get view matrix from camera and send it to shader
		glm::mat4 view = cam.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		// clear screen to color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw instanced mesh a bunch of times
		mesh.draw(shader, numberOfMeshes);

		// swap buffers
		window.display();
	}

	// clean up
	glDeleteTextures(1, &tex);
	glDeleteProgram(shader.program);
	delete[] modelMatrices;
	delete[] colors;

	return 0;
}












// used to go inside game loop before window.display
// just renders a bunch of meshes multiple times 
// sends in diff model mesh as uniform for each one

//glBindVertexArray(VAO);
//glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0, amount);
//glBindVertexArray(0);

//int n = 25;
//GLuint count = n*(n + 1) / 2;
//for (GLuint i = 0; i < count; i++) {
//	int r = 1;
//	int c = i;
//	while (c - r >= 0) {
//		c -= r;
//		r++;
//	}
//	r -= 1;

//	GLfloat spacing = 2.0f;
//	GLfloat halfrw = r / 2.0f * spacing;
//	glm::vec3 pos(c * spacing - halfrw, -1.0f, -spacing*0.866f * r);	// generate for each guy starting at 0.0.0

//	glm::mat4 model;
//	model = glm::translate(model, pos);
//	GLfloat angle = animTime.getElapsedTime().asSeconds() * glm::radians(180.0f) * 0.25f;
//	model = glm::rotate(model, angle, glm::vec3((GLfloat)r / 10.0f, 1.0f, -(GLfloat)r / 10.0f));

//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

//	mesh.draw(shader);
//}

//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
