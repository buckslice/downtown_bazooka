#include "cityGenerator.h"
#include "mathutil.h"
#include "aabb.h"
#include <stdlib.h>
#include <algorithm>
#include <time.h>

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

// checks if box collides with any of the boxes on the list
bool collidesWithAny(AABB box, std::vector<AABB> boxes) {
	for (GLuint i = 0; i < boxes.size(); i++) {
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
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

CityGenerator::CityGenerator() {
}

Mesh CityGenerator::buildMesh(GLuint& tex) {
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

void CityGenerator::generateModelMatrices(bool square) {
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

		glm::vec3 c;
		if (sy < 20.0f) {
			if (rand() % 50 == 1) {
				c = glm::vec3(1.0f, 0, 0);		// red
			} else {
				c = glm::vec3(rnd() * .25f + .5f, 0.0, 1.0f);	// purp
			}
		} else if (sy < 50.0f) {
			c = glm::vec3(1.0f, rnd() * .2f + .3f, 0.0f);	// orange
		} else {
			c = glm::vec3(0.0f, 0.5f * rnd() + .5f, 1.0f);	// blue
		}

		colors[i] = c;

		//colors[i] = rand() % 1000 == 1 ? glm::vec3(.5f, 0, 1.0f) : rand() % 100 == 1 ? glm::vec3(1.0f, 0, 0) : rand() % 2 == 0 ? glm::vec3(0, 1.0f, rnd()*.5f + .5f) : glm::vec3(0, rnd()*.5f + .5f, 1.0f);//MathUtil::generateRandomColor();
	}
}


// set up instancing uniforms
// basically uploads all the model matrices to mesh
void CityGenerator::uploadModelMatrices(Mesh& mesh) {
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

void CityGenerator::destroy() {
	delete[] modelMatrices;
	delete[] colors;
}