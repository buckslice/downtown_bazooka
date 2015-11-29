#include "cityGenerator.h"
#include "mathutil.h"
#include <stdlib.h>
#include <algorithm>
#include <time.h>

// checks if box collides with any of the boxes on the list
bool collidesWithAny(AABB box, std::vector<AABB> boxes) {
	for (GLuint i = 0; i < boxes.size(); i++) {
		if (AABB::check(box, boxes[i])) {
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

CityGenerator::CityGenerator() {
}

void CityGenerator::generate(bool square, GLuint count, Graphics& g) {
	srand(time(NULL));
	srand(1); //my testing city for physics bugs lol
    models.clear();
    colors.clear();
	boxes.clear();
	std::vector<glm::vec2> cities;

	// generate some random city centers
	const int numCitys = 8;
	for (GLuint i = 0; i < numCitys; i++) {
		glm::vec2 city;
		int tries = 0;
		do {
			if (square) {
				city = Mth::randomPointInSquare(CITY_SIZE);
			} else {
				city = Mth::randomPointInCircle(CITY_SIZE / 2.0f);
			}
			tries++;
		} while (tooClose(400.0f, city, cities) && tries < 100);

		cities.push_back(city);
	}

	// generate randomly located buildings
	// sized based on their distance to nearest city center
	for (GLuint i = 0; i < count; i++) {
		glm::mat4 model;

		float sx, sy, sz;
		int tries = 0;
		glm::vec2 p;
		while (tries < 100) {
			if (square) {
				p = Mth::randomPointInSquare(CITY_SIZE);
			} else {
				p = Mth::randomPointInCircle(CITY_SIZE / 2.0f);
			}

			float distToClosestCity = std::numeric_limits<float>::max();
			glm::vec2 closestCity;
			for (int i = 0; i < cities.size(); i++) {
				distToClosestCity = std::min(distToClosestCity, glm::distance(p, cities[i]));
			}
			float d = distToClosestCity;
			float b = Mth::blend(d, BLEND_DISTANCE, 0.0f, Mth::cubic);
			if (d > BLEND_DISTANCE) {
				b = 0.0f;	// lol comment this
			}

			sx = Mth::rand01() * 10.0f + b * 20.0f + 5.0f;
			sy = Mth::rand01() * 10.0f + b * 100.0f + 5.0f;
			sz = Mth::rand01() * 10.0f + b * 20.0f + 5.0f;

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

        models.push_back(model);

		glm::vec3 c;
		if (rand() % 50 == 1) {
			c = glm::vec3(1.0f, 0, 0);		// red
		} else if (sy < 15.0f) {
			c = glm::vec3(0.0f, Mth::rand01() * 0.5f, 1.0f);	// blue
		} else if (sy < 20.0f) {
			c = glm::vec3(0.0f, 1.0f, 1.0f);	// blue
		} else if (sy < 50.0f) {
			c = glm::vec3(Mth::rand01() * .25f + .5f, 0.0, 1.0f);	// purp
		} else {
			c = glm::vec3(1.0f, Mth::rand01() * .2f + .3f, 0.0f);	// orange
		}

        colors.push_back(c);
	}
    
    if (!first) {
        glDeleteBuffers(1, &colorBuffer);
        glDeleteBuffers(1, &modelBuffer);
    }
    g.genColorBuffer(*g.cube, colors);
    g.genModelBuffer(*g.cube, models);
    first = false;
}