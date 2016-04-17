#include "terrain.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "physics.h"
#include "input.h"
#include "graphics.h"
#include "entityManager.h"

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

CVertex Chunk::genPoint(float x, float y) {
    const int max = 2;
    double f[max];
    uint32_t id[max];
    Noise::worley(x + seed.x, y + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.003f);
    //float h = Noise::fractal_worley_3D(x, y, 0.0f, Noise::MANHATTAN, 3, 0.0075f);

    float h = static_cast<float>(f[1] - f[0]);
    h = Mth::clamp(h, 0.0f, 1.0f);

    int idc = 1;    // 0 is whole cell, 1 is sides of cell, 2 is subcells within sides?
    glm::vec3 color = glm::vec3((id[idc] % 50) / 255.0f, (id[idc] % 50) / 255.0f, (id[idc] % 127) / 255.0f);
    color = glm::clamp(color, glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.2f, 0.2f, 0.5f));
    //color = glm::vec3(1.0f, 0.0f, 0.0f);

    Noise::worley(x + seed.x, y + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.001f);
    float b = static_cast<float>(1.0f - f[0]);
    b = Mth::cblend(b, 0.5f, 1.0f, Mth::cubic);

    return CVertex{ glm::vec3(x, (h + b)*100.0f, y), color };
}

void Chunk::generateTerrain() {
    std::vector<GLuint> tris;
    tris.reserve(NUM_TILES*NUM_TILES * 6);
    std::vector<CVertex> genVerts;
    genVerts.reserve((NUM_TILES + 1)*(NUM_TILES + 1));
    verts.reserve((NUM_TILES + 1)*(NUM_TILES + 1));

    // debug color
    glm::vec3 debugColor = HSBColor(Mth::rand01(), 1.0f, 1.0f).toRGB();

    // calculate vertex for each point
    for (int y = 0; y < NUM_TILES + 1; y++) {
        for (int x = 0; x < NUM_TILES + 1; x++) {
            float xo = x * TILE_SIZE + pos.first * CHUNK_SIZE - CHUNK_SIZE / 2.0f;
            float yo = y * TILE_SIZE + pos.second * CHUNK_SIZE - CHUNK_SIZE / 2.0f;

            CVertex cv = genPoint(xo, yo);
            if (debugColors) {
                cv.color = debugColor;
            }

            genVerts.push_back(cv);
            verts.push_back(cv.position);
        }
    }

    std::vector<CTVertex> meshVerts;

    // split triangles and make UV based on chunk size
    GLuint tri = 0;
    for (int y = 0; y < NUM_TILES; ++y) {
        for (int x = 0; x < NUM_TILES; ++x) {
            float x0 = (float)x / NUM_TILES;
            float x1 = (float)(x + 1) / NUM_TILES;
            float y0 = (float)y / NUM_TILES;
            float y1 = (float)(y + 1) / NUM_TILES;

            meshVerts.push_back({ genVerts[x + y*(NUM_TILES + 1)], glm::vec2(x0, y0) });
            meshVerts.push_back({ genVerts[x + (y + 1)*(NUM_TILES + 1)], glm::vec2(x0, y1) });
            meshVerts.push_back({ genVerts[x + (y + 1)*(NUM_TILES + 1) + 1], glm::vec2(x1, y1) });
            meshVerts.push_back({ genVerts[x + y*(NUM_TILES + 1) + 1], glm::vec2(x1, y0) });

            tris.push_back(tri);
            tris.push_back(tri + 1);
            tris.push_back(tri + 2);
            tris.push_back(tri + 2);
            tris.push_back(tri + 3);
            tris.push_back(tri);
            tri += 4;
        }
    }

    mesh = new StandardMesh(meshVerts, tris);
}

// randomly generate trees and buildings in cities
void Chunk::generateStructures(std::mt19937& rng, Distributions rds) {
    float x, z, sx, sy, sz, x0, z0, x1, z1;

    // return variables for worley noise
    const int max = 3;
    double f[max];
    uint32_t id[max];

    bool generateTrees = false;
    bool breakOuter = false;
    for (GLuint i = 0; i < 20 && !breakOuter; ++i) {    // try to build this many buildings
        for (int j = 0; j < 10; ++j) {  // try this many times to find a spot for current building
            x = rds.unix(rng);    // random x position in chunk
            z = rds.uniz(rng);    // random y position in chunk

            // calculate noise that represents city centers
            Noise::worley(x + seed.x, z + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.001f);
            float b = static_cast<float>(1.0f - f[0]);
            b = Mth::cblend(b, 0.5f, 1.0f, Mth::cubic);
            // if any random point not in city then delete all buildings of chunk
            // and have chance to spawn trees
            if (b < 0.001f) {
                generateTrees = true;
                clearStatics();
                buildingModels.clear();
                buildingColors.clear();
                breakOuter = true;
                break;
            }
            // calculate scale of building
            sx = rds.zeroToOne(rng) * 10.0f + b * 20.0f + 5.0f;
            sy = rds.zeroToOne(rng) * 10.0f + b * 100.0f + 5.0f;
            sz = rds.zeroToOne(rng) * 10.0f + b * 20.0f + 5.0f;

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
                    if (rds.zeroToOne(rng) <= 0.05f) {
                        c = glm::vec3(0.0f, 1.0f, 0.25f);    // green
                    } else {
                        float n = Noise::ridged_2D(x, z, 3, .001f);
                        if (n < 0.25f) {
                            c = glm::vec3(0.0f, rds.zeroToOne(rng)* 0.5f, 1.0f);	// blueish
                        } else {
                            c = glm::vec3(0.0f, 1.0f, 1.0f);    // teal
                        }
                    }
                } else if (sy < HIGH_HEIGHT) {
                    float t = Mth::blend(sy, LOW_HEIGHT, HIGH_HEIGHT);
                    c = Mth::lerp(glm::vec3(0.3f, 0.0f, 1.0f), glm::vec3(1.0f, 0.4f, 0.0f), t);

                    //c = glm::vec3(Mth::rand01() * .25f + .5f, 0.0, 1.0f);	// purp
                } else {
                    c = glm::vec3(1.0f, rds.zeroToOne(rng) * .2f + .3f, 0.0f);	// orange
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
        x = rds.unix(rng);    // random x position in chunk
        z = rds.uniz(rng);    // random y position in chunk

        float n = Noise::fractal_2D(x + seed.x, z + seed.y, 3, 0.001f);
        if (n > 0.0f) {
            continue;
        }
        float h = getHeight(x, z);
        sx = rds.zeroToOne(rng) * 8.0f + 8.0f;
        sy = rds.zeroToOne(rng) * 10.0f + 20.0f;

        AABB trunk(glm::vec3(x - 1.0f, h - 1.0f, z - 1.0f), glm::vec3(x + 1.0f, h + sy / 2.2f, z + 1.0f));
        if (Physics::checkStatic(trunk)) {  // if trunk isnt blocked then build whole tree
            continue;
        }
        staticIndices.push_back(Physics::addStatic(trunk));
        buildingModels.push_back(trunk.getModelMatrix());
        HSBColor trunkColor(0.0833f, 0.4f * rds.zeroToOne(rng) + 0.6f, 0.4f);
        buildingColors.push_back(trunkColor.toRGB());

        glm::vec3 euler = glm::vec3(0.0f, rds.zeroToOne(rng)*360.0f, 0.0f);
        glm::quat q = glm::quat(euler*DEGREESTORADS);
        glm::mat4 model = Mth::getModelMatrix(glm::vec3(x, h + sy * 0.4f, z), glm::vec3(sx, sy, sx), q);
        treeModels.push_back(model);

        HSBColor leafColor(0.2f * rds.zeroToOne(rng) + 0.25f, 1.0f, 0.5f * rds.zeroToOne(rng) + 0.3f);
        treeColors.push_back(leafColor.toRGB());
    }
}

void Chunk::spawnEntities(std::mt19937& rng, Distributions rds) {
    //int numEnemies = (int)(rds.zeroToOne(rng) * 1.0 + 1.0f);
    int numEnemies = 0;
    int numItems = 2;

    for (int i = 0; i < numEnemies; ++i) {
        float x = rds.unix(rng);    // random x position in chunk
        float z = rds.uniz(rng);    // random y position in chunk
        float h = getHeight(x, z) + 10.0f;

        EnemyType et = rds.zeroToOne(rng) < 0.1f ? EnemyType::ELITE : EnemyType::BASIC;

        EntityManagerInstance->SpawnEnemy(glm::vec3(x, h, z), et);
    }
    for (int i = 0; i < numItems; ++i) {
        float x = rds.unix(rng);    // random x position in chunk
        float z = rds.uniz(rng);    // random y position in chunk
        float h = getHeight(x, z) + 10.0f;

        ItemType it = (ItemType)Mth::randRange(0, (int)ItemType::COUNT);

        EntityManagerInstance->SpawnItem(glm::vec3(x, h, z), it);
    }

}

Chunk::Chunk(point p) {
    this->pos = p;

    float cx = pos.first * CHUNK_SIZE;
    float cz = pos.second * CHUNK_SIZE;
    float hc = CHUNK_SIZE / 2.0f;

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
    Distributions dist = Distributions{ unix, uniz, zeroToOne };

    generateTerrain();
    generateStructures(rng, dist);
    spawnEntities(rng, dist);
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
    //if (Mth::rand01() < 0.1f) {
    //    float cx = pos.first * CHUNK_SIZE;
    //    float cz = pos.second * CHUNK_SIZE;
    //    float hc = CHUNK_SIZE / 2.0f;
    //    float x = Mth::randRange(cx - hc, cx + hc);
    //    float z = Mth::randRange(cz - hc, cz + hc);
    //    float h = getHeight(x, z);

    //    glm::vec3 pos = glm::vec3(x,h,z);
    //    EntityManagerInstance->SpawnParticle(pos, ParticleType::FIRE, 0.0f, glm::vec3(Mth::randUnit(), Mth::rand01(), Mth::randUnit())*10.0f);
    //}
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
    glm::vec3 o = verts[0];
    // get tile coordinate in mesh
    int xi = Mth::clamp((int)((x - o.x) / TILE_SIZE), 0, NUM_TILES - 1);
    int zi = Mth::clamp((int)((z - o.z) / TILE_SIZE), 0, NUM_TILES - 1);

    int a = xi + zi * (NUM_TILES + 1);
    int b = xi + (zi + 1) * (NUM_TILES + 1) + 1;
    glm::vec3 c;
    float i, j;

    // check which triangle of tile youre in
    if (x - o.x - xi*TILE_SIZE > z - o.z - zi*TILE_SIZE) {
        c = verts[a + 1];
        i = c.x - x;
        j = z - c.z;
    } else {
        c = verts[b - 1];
        i = c.z - z;
        j = x - c.x;
    }

    return (((verts[a] - c)*i + (verts[b] - c)*j) / TILE_SIZE + c).y;
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
void Terrain::update(float delta, glm::vec3 pl) {
    // get players chunk coordinates
    point pcc = worldToChunk(pl.x, pl.z);
    //std::cout << pcc.first << " " << pcc.second << " " << pl.x << " " << pl.z << std::endl;

    float flowStartup = 2.0f;
    if (Input::pressed(sf::Keyboard::BackSpace)) {
        timeSinceFlow -= delta / flowStartup;
    } else {
        timeSinceFlow += delta / flowStartup;
    }
    timeSinceFlow = Mth::saturate(timeSinceFlow);
    lavaTime += delta * (1.0f - timeSinceFlow);


    std::vector<point> newPoints;
    std::vector<float> distances;

    size_t newChunksPerFrame = 10;

    // find closest chunks to build this frame
    for (int y = -CHUNK_LOAD_RADIUS; y <= CHUNK_LOAD_RADIUS; y++) {
        for (int x = -CHUNK_LOAD_RADIUS; x <= CHUNK_LOAD_RADIUS; x++) {
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
            if (sqrdist > LOAD_DIST*LOAD_DIST) {
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
        if (sqrdist > LOAD_DIST * LOAD_DIST + 1.0f) { // if too far away
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
    r.terrainShader.use();

    glUniformMatrix4fv(glGetUniformLocation(r.terrainShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(r.terrainShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform1f(glGetUniformLocation(r.terrainShader.program, "time"), lavaTime);
    glUniform1f(glGetUniformLocation(r.terrainShader.program, "clerp"), timeSinceFlow);
    glUniform1f(glGetUniformLocation(r.terrainShader.program, "tileFactor"), (float)NUM_TILES);

    chunks[0]->mesh->bindTextures(Resources::get().terrainTex, Resources::get().noiseTex);

    //std::cout << chunks.size() << std::endl;
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        chunks[i]->render();
    }

    chunks[0]->mesh->unbind();

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