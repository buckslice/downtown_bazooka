#include "terrain.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "physics.h"
#include "input.h"
#include "graphics.h"

// public variables for both terrain and chunk
glm::vec2 seed;
bool debugColors = false;

void Terrain::setSeed(glm::vec2 sd) {
    seed = sd;
}
bool Terrain::toggleDebugColors() {
    debugColors = !debugColors;
    return debugColors;
}

CTVertex Chunk::genPoint(float x, float y) {
    const int max = 2;
    double f[max];
    uint32_t id[max];
    Noise::worley(x + seed.x, y + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.003f);
    //float h = Noise::fractal_worley_3D(x, y, 0.0f, Noise::MANHATTAN, 3, 0.0075f);

    float h = static_cast<float>(f[1] - f[0]);
    h = Mth::clamp(h, 0.0f, 1.0f);

    int idc = 1;    // 0 is whole cell, 1 is sides of cell, 2 is subcells within sides?
    glm::vec3 rand = glm::vec3((id[idc] % 50) / 255.0f, (id[idc] % 50) / 255.0f, (id[idc] % 127) / 255.0f);
    rand = glm::clamp(rand, glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.2f, 0.2f, 0.5f));
    glm::vec3 color = glm::vec3(rand);

    Noise::worley(x + seed.x, y + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.001f);
    float b = static_cast<float>(1.0f - f[0]);
    b = Mth::cblend(b, 0.5f, 1.0f, Mth::cubic);

    return CTVertex{ glm::vec3(x, (h + b)*100.0f, y), color, glm::vec2() };
}

void Chunk::generateTerrain() {
    std::vector<GLuint> tris;
    tris.reserve(NUM_TILES*NUM_TILES * 6);
    verts.reserve((NUM_TILES + 1)*(NUM_TILES + 1));

    // used for texture coordinates
    bool bot = true;

    // debug color
    glm::vec3 debugColor = HSBColor(Mth::rand01(), 1.0f, 1.0f).toRGB();

    // calculate vertex for each point
    for (int y = 0; y < NUM_TILES + 1; y++) {
        bool left = true;
        for (int x = 0; x < NUM_TILES + 1; x++) {
            float xo = x * TILE_SIZE + pos.first * CHUNK_SIZE - CHUNK_SIZE / 2.0f;
            float yo = y * TILE_SIZE + pos.second * CHUNK_SIZE - CHUNK_SIZE / 2.0f;

            CTVertex cv = genPoint(xo, yo);
            if (debugColors) {
                cv.color = debugColor;
            }

            // texcoords are mirrored for max vertex sharing,
            // limitation is that textures have to be symmetric
            cv.texcoord = glm::vec2(left ? 0.0f : 1.0f, bot ? 1.0f : 0.0f);

            verts.push_back(cv);
            left = !left;
        }
        bot = !bot;
    }

    // calculate triangles
    for (int i = 0, len = (NUM_TILES + 1)*NUM_TILES - 1; i < len; ++i) {
        if ((i + 1) % (NUM_TILES + 1) == 0) {
            continue;
        }
        tris.push_back(i);
        tris.push_back(i + NUM_TILES + 1);
        tris.push_back(i + NUM_TILES + 2);
        tris.push_back(i + NUM_TILES + 2);
        tris.push_back(i + 1);
        tris.push_back(i);
    }
    mesh = new StandardMesh(verts, tris, Resources::get().terrainTex);
}

// randomly generate trees and buildings in cities
void Chunk::generateStructures() {
    float cx = pos.first * CHUNK_SIZE;
    float cz = pos.second * CHUNK_SIZE;
    float hc = CHUNK_SIZE / 2.0f;
    float x, z, sx, sy, sz, x0, z0, x1, z1;

    // use a seeded random generator for this chunk based on its chunk coords
    // this way the building positions and shapes will be the same each time
    std::random_device rd;
    std::mt19937 rng(rd());
    int rngseed = (pos.first + pos.second)*(pos.first + pos.second + 1) / 2 + pos.second;
    rng.seed(rngseed);

    // generators for this chunk
    std::uniform_real_distribution<float> unix(cx - hc, cx + hc);
    std::uniform_real_distribution<float> uniz(cz - hc, cz + hc);
    std::uniform_real_distribution<float> zeroToOne(0.0f, 1.0f);

    // return variables for worley noise
    const int max = 3;
    double f[max];
    uint32_t id[max];

    bool generateTrees = false;
    bool breakOuter = false;    // i was gonna use a goto but this is less gross kinda?
    for (GLuint i = 0; i < 20 && !breakOuter; ++i) {    // try to build this many buildings
        for (int j = 0; j < 10; ++j) {  // try this many times to find a spot for current building
            x = unix(rng);    // random x position in chunk
            z = uniz(rng);    // random y position in chunk

            // calculate noise that represents city centers
            Noise::worley(x + seed.x, z + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.001f);
            float b = static_cast<float>(1.0f - f[0]);
            b = Mth::cblend(b, 0.5f, 1.0f, Mth::cubic);
            // if not in a city then have a lower chance to spawn building
            if (b < 0.001f) {
                generateTrees = true;
                clearStatics();
                buildingModels.clear();
                buildingColors.clear();
                breakOuter = true;
                break;
            }
            // calculate scale of building
            sx = zeroToOne(rng) * 10.0f + b * 20.0f + 5.0f;
            sy = zeroToOne(rng) * 10.0f + b * 100.0f + 5.0f;
            sz = zeroToOne(rng) * 10.0f + b * 20.0f + 5.0f;

            // calculate building corners
            x0 = x - sx / 2.0f;
            z0 = z - sz / 2.0f;
            x1 = x + sx / 2.0f;
            z1 = z + sz / 2.0f;

            // make box with incorrect height but just to see if spot is available
            // since pretty much all the blocking happens on the x and z
            AABB box(glm::vec3(x0, 0.0f, z0), glm::vec3(x1, sy + 1000.0f, z1));

            if (!Physics::checkStatic(box)) {
                // now calculate height at each corner and get lowest for accurate box
                float lowestHeight = glm::min(getHeight(x0, z0),
                    glm::min(getHeight(x1, z0), glm::min(getHeight(x1, z1), getHeight(x0, z1))));

                // add to physics matrix
                box = AABB(glm::vec3(x0, lowestHeight, z0), glm::vec3(x1, sy + lowestHeight, z1));
                staticIndices.push_back(Physics::addStatic(box));

                // calculate color
                glm::vec3 c;
                if (sy < LOW_HEIGHT) {
                    if (zeroToOne(rng) <= 0.05f) {
                        c = glm::vec3(0.0f, 1.0f, 0.25f);    // green
                    } else {
                        float n = Noise::ridged_2D(x, z, 3, .001f);
                        if (n < 0.25f) {
                            c = glm::vec3(0.0f, zeroToOne(rng)* 0.5f, 1.0f);	// blueish
                        } else {
                            c = glm::vec3(0.0f, 1.0f, 1.0f);    // teal
                        }
                    }
                } else if (sy < HIGH_HEIGHT) {
                    float t = Mth::blend(sy, LOW_HEIGHT, HIGH_HEIGHT);
                    c = Mth::lerp(glm::vec3(0.3f, 0.0f, 1.0f), glm::vec3(1.0f, 0.4f, 0.0f), t);

                    //c = glm::vec3(Mth::rand01() * .25f + .5f, 0.0, 1.0f);	// purp
                } else {
                    c = glm::vec3(1.0f, zeroToOne(rng) * .2f + .3f, 0.0f);	// orange
                }
                buildingModels.push_back(box.getModelMatrix());
                buildingColors.push_back(c);

                break;
            }
        }
    }

    if (!generateTrees) {
        return;
    }
    for (GLuint i = 0; i < 20; ++i) {
        x = unix(rng);    // random x position in chunk
        z = uniz(rng);    // random y position in chunk

        float n = Noise::fractal_2D(x + seed.x, z + seed.y, 3, 0.001f);
        if (n > 0.0f) {
            continue;
        }
        float h = getHeight(x, z);
        sx = zeroToOne(rng) * 8.0f + 8.0f;
        sy = zeroToOne(rng) * 10.0f + 20.0f;

        AABB trunk(glm::vec3(x - 1.0f, h - 1.0f, z - 1.0f), glm::vec3(x + 1.0f, h + sy / 2.2f, z + 1.0f));
        if (Physics::checkStatic(trunk)) {  // if trunk isnt blocked then build whole tree
            continue;
        }
        staticIndices.push_back(Physics::addStatic(trunk));
        buildingModels.push_back(trunk.getModelMatrix());
        HSBColor trunkColor(0.0833f, 0.4f * zeroToOne(rng) + 0.6f, 0.4f);
        buildingColors.push_back(trunkColor.toRGB());

        glm::vec3 euler = glm::vec3(0.0f, zeroToOne(rng)*360.0f, 0.0f);
        glm::quat q = glm::quat(euler*DEGREESTORADS);
        glm::mat4 model = Mth::getModelMatrix(glm::vec3(x, h + sy * 0.4f, z), glm::vec3(sx, sy, sx), q);
        treeModels.push_back(model);

        HSBColor leafColor(0.2f * zeroToOne(rng) + 0.25f, 1.0f, 0.5f * zeroToOne(rng) + 0.3f);
        treeColors.push_back(leafColor.toRGB());
    }
}

Chunk::Chunk(point pos) {
    this->pos = pos;
    generateTerrain();
    generateStructures();
}

Chunk::~Chunk() {
    delete mesh;
    clearStatics();
}

void Chunk::clearStatics() {
    for (size_t i = 0, len = staticIndices.size(); i < len; ++i) {
        Physics::removeStatic(staticIndices[i]);
    }
    staticIndices.clear();
}

void Chunk::render() {
    mesh->render();
    if (!Graphics::DEBUG) {
        Graphics::addToStream(Shape::CUBE_GRID, buildingModels, buildingColors);
        Graphics::addToStream(Shape::PYRAMID, treeModels, treeColors);
    } else {
        for (size_t i = 0, len = treeModels.size(); i < len; ++i) {
            Graphics::addToStream(Shape::PYRAMID, treeModels[i], glm::vec3(1.0f, 0.0f, 1.0f));
        }
    }
}


float Chunk::getHeight(float x, float z) {
    glm::vec3 o = verts[0].position;
    // get tile coordinate in mesh
    int xi = Mth::clamp((int)((x - o.x) / TILE_SIZE), 0, NUM_TILES - 1);
    int zi = Mth::clamp((int)((z - o.z) / TILE_SIZE), 0, NUM_TILES - 1);

    int a = xi + zi * (NUM_TILES + 1);
    int b = xi + (zi + 1) * (NUM_TILES + 1) + 1;
    glm::vec3 c;
    float i, j;

    // shouldnt ever happen but not sure if it still does lol
    // should try this #def NDEBUG shit working would be cool
    //assert(a + 1 < vert.size() && b - 1 < verts.size());

    // check which triangle of tile youre in
    if (x - o.x - xi*TILE_SIZE > z - o.z - zi*TILE_SIZE) {
        c = verts[a + 1].position;
        i = c.x - x;
        j = z - c.z;
    } else {
        c = verts[b - 1].position;
        i = c.z - z;
        j = x - c.x;
    }

    return (((verts[a].position - c)*i + (verts[b].position - c)*j) / TILE_SIZE + c).y;
}

float Terrain::queryHeight(float x, float z) {
    point p = worldToChunk(x, z);
    if (!coordsByIndices.count(p)) {
        return -10000.0f;   // basically -infinity right? lol
    }

    return chunks[coordsByIndices[p]]->getHeight(x, z);
}

point Terrain::worldToChunk(float x, float z) {
    int px = (int)((x - CHUNK_SIZE / 2.0f * (x > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);
    int pz = (int)((z - CHUNK_SIZE / 2.0f * (z > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);

    return point{ px, pz };
}

int frames = 0;
void Terrain::update(glm::vec3 pl) {
    // get players chunk coordinates
    point pcc = worldToChunk(pl.x, pl.z);
    //std::cout << pcc.first << " " << pcc.second << " " << pl.x << " " << pl.z << std::endl;

    std::vector<point> newPoints;
    std::vector<float> distances;

    size_t newChunksPerFrame = 10;

    // find closest chunks to build this frame
    for (int y = -CHUNK_RAD; y <= CHUNK_RAD; y++) {
        for (int x = -CHUNK_RAD; x <= CHUNK_RAD; x++) {
            point ccc = point{ pcc.first + x, pcc.second + y };
            // continue if current chunk coord is already in table
            if (coordsByIndices.count(ccc)) {
                continue;
            }
            // chunk center in worldspace
            float cx = ccc.first*CHUNK_SIZE;
            float cz = ccc.second*CHUNK_SIZE;
            // calculate square distance
            float sqrdist = (pl.x - cx)*(pl.x - cx) + (pl.z - cz)*(pl.z - cz);
            if (sqrdist > DIST*DIST) {
                continue;
            }

            // insertion sort
            size_t len = distances.size();
            if (len < newChunksPerFrame) {
                distances.push_back(sqrdist);
                newPoints.push_back(ccc);
            } else {
                for (size_t i = 0; i < len; ++i) {
                    if (sqrdist < distances[i]) {
                        distances.insert(distances.begin() + i, sqrdist);
                        newPoints.insert(newPoints.begin() + i, ccc);
                        distances.pop_back();
                        newPoints.pop_back();
                        break;
                    }
                }
            }
        }
    }

    // build new chunks
    for (size_t i = 0; i < newPoints.size(); ++i) {
        coordsByIndices[newPoints[i]] = chunks.size();
        chunks.push_back(new Chunk(newPoints[i]));
    }

    // delete chunks no longer nearby player
    // dont need to check every frame
    if (++frames < 10) {
        return;
    }
    frames = 0;
    for (size_t i = 0; i < chunks.size(); ++i) {
        Chunk* c = chunks[i];
        float cx = c->pos.first * CHUNK_SIZE;
        float cz = c->pos.second * CHUNK_SIZE;
        float sqrdist = (pl.x - cx)*(pl.x - cx) + (pl.z - cz)*(pl.z - cz);
        if (sqrdist > DIST * DIST + 1.0f) { // if too far away
            // reassign back guy to this index, 
            coordsByIndices[chunks.back()->pos] = i;

            // then swap and pop
            std::swap(chunks[i], chunks.back());
            chunks.pop_back();

            // delete this chunks stuff
            coordsByIndices.erase(c->pos);
            delete c;

            // decrement to offset next increment since we lost an element
            i--;
        }
    }

}

// render all terrain chunks
void Terrain::render(glm::mat4 view, glm::mat4 proj) {
    Resources& r = Resources::get();
    r.defaultShader.use();
    glUniformMatrix4fv(glGetUniformLocation(r.defaultShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(r.defaultShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    //std::cout << chunks.size() << std::endl;
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        chunks[i]->render();
    }

}

void Terrain::deleteChunks() {
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        delete chunks[i];
    }
    chunks.erase(chunks.begin(), chunks.end());
    coordsByIndices.clear();
}

Terrain::Terrain() {
    setSeed(glm::vec2(Mth::randRange(-10000.0f, 10000.0f), Mth::randRange(-10000.0f, 10000.0f)));
}


Terrain::~Terrain() {
    deleteChunks();
}