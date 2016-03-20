#include "cityGenerator.h"
#include <stdlib.h>

#include <time.h>
#include "mathutil.h"
#include "noise.h"
#include "hsbColor.h"

bool tooClose(float minDist, glm::vec2 city, std::vector<glm::vec2> cities) {
    for (size_t i = 0, len = cities.size(); i < len; ++i) {
        if (glm::distance(city, cities[i]) < minDist) {
            return true;
        }
    }
    return false;
}

CityGenerator::CityGenerator() {
    buildingMesh = Graphics::registerMesh();
}

// generate a random city
// uploads data to graphics and physics systems
// should remove need for physics reference and make it static
void CityGenerator::generate(bool square, bool colorByAngle, GLuint count, Physics& phys) {
    models.clear();
    colors.clear();
    std::vector<glm::vec2> cities;

    // generate some random city centers
    const int numCitys = (int)(Mth::rand01() * 9.0f + 3.0f);
    for (int i = 0; i < numCitys; ++i) {
        glm::vec2 city;
        int tries = 0;
        do {
            if (square) {
                city = Mth::randomPointInSquare(CITY_SIZE);
            } else {
                city = Mth::randomPointInCircle(CITY_SIZE / 2.0f);
            }
            tries++;
        } while (tooClose(MIN_DIST, city, cities) && tries < MAX_TRIES);
        
        cities.push_back(city);
    }
    float seedX = Mth::rand0X(1000.0f);
    float seedY = Mth::rand0X(1000.0f);
    // generate randomly located buildings
    // sized based on their distance to nearest city center
    for (GLuint i = 0; i < count; ++i) {
        glm::mat4 model;

        float sx, sy, sz;
        int tries = 0;
        glm::vec2 p;
        while (tries < MAX_TRIES) {
            if (square) {
                p = Mth::randomPointInSquare(CITY_SIZE);
            } else {
                p = Mth::randomPointInCircle(CITY_SIZE / 2.0f);
            }

            float distToClosestCity = std::numeric_limits<float>::max();
            glm::vec2 closestCity;
            for (size_t i = 0, len = cities.size(); i < len; ++i){
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
            if (!phys.checkStatic(box)) {
                phys.addStatic(box);
                break;
            }

            tries++;
        }

        glm::vec3 pos = glm::vec3(p.x, sy / 2.0f, p.y);
        glm::vec3 scale = glm::vec3(sx, sy, sz);
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);

        models.push_back(model);

        // set up color for building
        glm::vec3 c;
        if (!colorByAngle) {
            if (sy < LOW_HEIGHT) {
                if (Mth::rand01() <= 0.05f) {
                    c = glm::vec3(0.0f, 1.0f, 0.25f);    // green
                } else {
                    float n = Noise::ridged_2D(pos.x + seedX, pos.z + seedY, 3, .001f);
                    if (n < 0.25f) {
                        c = glm::vec3(0.0f, Mth::rand01()* 0.5f, 1.0f);	// blueish
                    } else {
                        c = glm::vec3(0.0f, 1.0f, 1.0f);    // teal
                    }
                }
            } else if (sy < HIGH_HEIGHT) {
                float t = Mth::blend(sy, LOW_HEIGHT, HIGH_HEIGHT);
                c = Mth::lerp(glm::vec3(0.3f, 0.0f, 1.0f), glm::vec3(1.0f, 0.4f, 0.0f), t);

                //c = glm::vec3(Mth::rand01() * .25f + .5f, 0.0, 1.0f);	// purp
            } else {
                c = glm::vec3(1.0f, Mth::rand01() * .2f + .3f, 0.0f);	// orange
            }
        } else {
            // make colors based on angle
            glm::vec2 r = glm::vec2(1.0f, 0.0f);
            float angle = atan2(p.y, p.x) - atan2(r.y, r.x);
            angle *= RADSTODEGREES;
            if (angle < 0.0f) {
                angle += 360.0;
            }

            c = HSBColor(angle / 360.0f, 1.0f, 1.0f).toRGB();
        }

        colors.push_back(c);
    }

    //Graphics::setColors(buildingMesh, colors);
    //Graphics::setModels(buildingMesh, models);

}