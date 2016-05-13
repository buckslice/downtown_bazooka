#include "terrain.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "physics.h"
#include "input.h"
#include "graphics.h"
#include "entityManager.h"
#include "game.h"

// public variables for both terrain and chunk
glm::vec2 seed;
bool debugColors = false;

bool Terrain::hardGenerating = true;

void Terrain::setSeed(glm::vec2 sd) {
    seed = sd;
}
bool Terrain::toggleDebugColors() {
    debugColors = !debugColors;
    return debugColors;
}

CVertex Chunk::genPoint(float x, float y) {
    if (Game::isInFinalBattle()) {
        return CVertex{ glm::vec3(x, 0.0f, y), glm::vec3(0.0f, 0.0f, 0.5f) };
    }

    const int max = 2;
    double f[max];
    uint32_t id[max];
    Noise::worley(x + seed.x, y + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.003f);
    //float h = Noise::fractal_worley_3D(x, y, 0.0f, Noise::MANHATTAN, 3, 0.0075f);

    float h = static_cast<float>(f[1] - f[0]);
    h = Mth::clamp(h, 0.0f, 1.0f);

    glm::vec3 color = glm::vec3(0.0f, 0.1f, 0.3f);

    // reduce noise height near center
    float sqrdist = x*x + y*y;
    float centerLimit = 100.0f;
    float blendRange = 100.0f;
    if (sqrdist < centerLimit * centerLimit) {  // near center
        h = 0.0f;
    } else {
        Noise::worley(x + seed.x, y + seed.y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.001f);
        float b = static_cast<float>(1.0f - f[0]);
        h += Mth::cblend(b, 0.5f, 1.0f, Mth::cubic);
        if (sqrdist < (centerLimit + blendRange)*(centerLimit + blendRange)) { // at edge of center
            float d = glm::distance(glm::vec2(0.0f), glm::vec2(x, y));
            h *= Mth::blend(d, centerLimit, centerLimit + blendRange, Mth::cubic);
        } else {
            int idc = 1;    // 0 is whole cell, 1 is sides of cell, 2 is subcells within sides?
            color = glm::vec3((id[idc] % 50) / 255.0f, (id[idc] % 50) / 255.0f, (id[idc] % 127) / 255.0f);
            color = glm::clamp(color, glm::vec3(0.1f, 0.1f, 0.2f), glm::vec3(0.2f, 0.2f, 0.5f));
        }
    }

    return CVertex{ glm::vec3(x, h*100.0f, y), color };
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
void Chunk::generateStructures(RNG& rng) {
    float x, z, sx, sy, sz, x0, z0, x1, z1;

    // return variables for worley noise
    const int max = 3;
    double f[max];
    uint32_t id[max];

    bool generateTrees = false;
    bool breakOuter = false;
    for (GLuint i = 0; i < 20 && !breakOuter; ++i) {    // try to build this many buildings
        for (int j = 0; j < 10; ++j) {  // try this many times to find a spot for current building
            x = rng.x();    // random x position in chunk
            z = rng.z();    // random y position in chunk

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
            sx = rng.rand() * 10.0f + b * 20.0f + 5.0f;
            sy = rng.rand() * 10.0f + b * 100.0f + 5.0f;
            sz = rng.rand() * 10.0f + b * 20.0f + 5.0f;

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

                // calculate color
                Tag tag = Tag::NONE;
                glm::vec3 c;
                if (sy < LOW_HEIGHT) {
                    if (rng.rand() <= 0.05f) {
                        c = glm::vec3(0.0f, 1.0f, 0.25f);    // green
                        tag = Tag::HEALER;
                    } else {
                        float n = Noise::ridged_2D(x, z, 3, .001f);
                        if (n < 0.25f) {
                            c = glm::vec3(0.0f, rng.rand() * 0.5f, 1.0f);	// blueish
                        } else {
                            c = glm::vec3(0.0f, 1.0f, 1.0f);    // teal
                        }
                    }
                } else if (sy < HIGH_HEIGHT) {
                    float t = Mth::blend(sy, LOW_HEIGHT, HIGH_HEIGHT);
                    c = Mth::lerp(glm::vec3(0.3f, 0.0f, 1.0f), glm::vec3(1.0f, 0.4f, 0.0f), t);

                    //c = glm::vec3(Mth::rand01() * .25f + .5f, 0.0, 1.0f);	// purp
                } else {
                    c = glm::vec3(1.0f, rng.rand() * .2f + .3f, 0.0f);	// orange
                }

                staticIndices.push_back(Physics::addStatic(box, tag));

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
        x = rng.x();    // random x position in chunk
        z = rng.z();    // random y position in chunk

        float n = Noise::fractal_2D(x + seed.x, z + seed.y, 3, 0.001f);
        if (n > 0.0f) {
            continue;
        }
        float h = getHeight(x, z);
        sx = rng.rand() * 8.0f + 8.0f;
        sy = rng.rand() * 10.0f + 20.0f;

        AABB trunk(glm::vec3(x - 1.0f, h - 1.0f, z - 1.0f), glm::vec3(x + 1.0f, h + sy / 2.2f, z + 1.0f));
        if (Physics::checkStatic(trunk)) {  // if trunk isnt blocked then build whole tree
            continue;
        }
        staticIndices.push_back(Physics::addStatic(trunk));
        buildingModels.push_back(trunk.getModelMatrix());
        HSBColor trunkColor(0.0833f, 0.4f * rng.rand() + 0.6f, 0.4f);
        buildingColors.push_back(trunkColor.toRGB());

        glm::vec3 euler = glm::vec3(0.0f, rng.rand()*360.0f, 0.0f);
        glm::quat q = glm::quat(euler*DEGREESTORADS);
        glm::mat4 model = Mth::getModelMatrix(glm::vec3(x, h + sy * 0.4f, z), glm::vec3(sx, sy, sx), q);
        treeModels.push_back(model);

        HSBColor leafColor(0.2f * rng.rand() + 0.25f, 1.0f, 0.5f * rng.rand() + 0.3f);
        treeColors.push_back(leafColor.toRGB());
    }
}

void Chunk::spawnEntities(RNG& rng) {
    //int numEnemies = (int)(rds.zeroToOne(rng) * 1.0 + 1.0f);
    int numEnemies = 2;
    int numItems = 2;

    for (int i = 0; i < numEnemies; ++i) {
        float x = rng.x();    // random x position in chunk
        float z = rng.z();    // random y position in chunk
        float h = getHeight(x, z) + 10.0f;

        EnemyType et = rng.rand() < 0.1f ? EnemyType::ELITE : EnemyType::BASIC;

        EntityManagerInstance->SpawnEnemy(glm::vec3(x, h, z), et);
    }
    for (int i = 0; i < numItems; ++i) {
        float x = rng.x();    // random x position in chunk
        float z = rng.z();    // random y position in chunk
        float h = getHeight(x, z) + 10.0f;

        ItemType it = (ItemType)Mth::randRange(0, (int)ItemType::COUNT);

        EntityManagerInstance->SpawnItem(glm::vec3(x, h, z), it);
    }

}

void Chunk::generateFinalBattle(RNG& rng) {
    float cx = pos.first * CHUNK_SIZE;
    float cz = pos.second * CHUNK_SIZE;
    float hc = CHUNK_SIZE / 2.0f;
    float platformSize = 400.0f;
    glm::vec3 ori = Game::getFinalOrigin() + glm::vec3(500.0f, 0.0f, 0.0f);
    float sqrDistx = (cx - ori.x)*(cx - ori.x);
    float sqrDistz = (cz - ori.z)*(cz - ori.z);
    float maxDist = 700.0f*700.0f;
    if (sqrDistx > maxDist || sqrDistz > maxDist) {
        return;
    }

    // check if main boss platform should be spawned in this chunk
    if (ori.x > cx - hc && ori.x < cx + hc &&
        ori.z > cz - hc && ori.z < cz + hc) {
        glm::vec3 minOri = glm::vec3(ori.x - platformSize / 2.0f, 0.0f, ori.z - platformSize / 2.0f);
        int slices = 10;
        float sliceSize = platformSize / slices;
        for (int i = 0; i < slices*slices; ++i) {
            int x = i % slices;
            int z = i / slices;
            glm::vec3 min = minOri + glm::vec3(sliceSize * x, 0.0f, sliceSize * z);
            int ps = 0;
            const int tiers = 3;
            for (int j = 0; j < tiers; j++) {
                if (x > j && x < slices - 1 - j &&
                    z > j && z < slices - 1 - j) {
                    ps++;
                }
            }

            float h = rng.rand() * 5.0f + 90.0f + ps * 25.0f;
            glm::vec3 max = min + glm::vec3(sliceSize, h, sliceSize);

            AABB box = AABB(min, max);
            staticIndices.push_back(Physics::addStatic(box));
            buildingModels.push_back(box.getModelMatrix());
            buildingColors.push_back(glm::vec3(0.2f * rng.rand() + 0.3f, 0.0f, (ps / (float)tiers)*0.75f));

        }
        return;
    }

    float x, z, sx, sy, sz, x0, z0, x1, z1;
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 10; ++j) {
            x = rng.x();    // random x position in chunk
            z = rng.z();    // random y position in chunk

            float d = std::max(abs(x - ori.x), abs(z - ori.z));
            float b = Mth::cblend(d, 500.0f, platformSize / 2.0f, Mth::cubic);

            // calculate scale of building
            sx = rng.rand() * 3.0f + 2.0f + b * 5.0f;
            sy = rng.rand() * 1.0f + 2.0f + b * 75.0f;
            sz = rng.rand() * 3.0f + 2.0f + b * 5.0f;

            // calculate building corners
            x0 = x - sx / 2.0f;
            z0 = z - sz / 2.0f;
            x1 = x + sx / 2.0f;
            z1 = z + sz / 2.0f;

            AABB box = AABB(glm::vec3(x0, 0.0f, z0), glm::vec3(x1, sy, z1));
            if (!Physics::checkStatic(box)) {
                staticIndices.push_back(Physics::addStatic(box));
                buildingModels.push_back(box.getModelMatrix());
                buildingColors.push_back(glm::vec3(0.1f * rng.rand() + 0.2f, 0.0f, 0.0f));
                break;
            }
        }
    }

}

Chunk::Chunk(point p) {
    this->pos = p;

    float cx = pos.first * CHUNK_SIZE;
    float cz = pos.second * CHUNK_SIZE;
    float hc = CHUNK_SIZE / 2.0f;
    RNG rng(pos, cx, cz, hc);

    generateTerrain();

    if (Game::isInFinalBattle()) {
        generateFinalBattle(rng);
    } else {
        // spawn buildings and entities if not in center
        float sqrdist = cx*cx + cz*cz;
        float centerLimit = 200.0f;
        if (sqrdist > centerLimit * centerLimit) {
            generateStructures(rng);
            spawnEntities(rng);
        }
    }

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

float Chunk::getHeight(float x, float z) const {
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

point Terrain::worldToChunk(float x, float z) const {
    int px = (int)((x - CHUNK_SIZE / 2.0f * (x > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);
    int pz = (int)((z - CHUNK_SIZE / 2.0f * (z > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);

    return point{ px, pz };
}

int frames = 0;
void Terrain::update(float delta) {
    glm::vec3 pp = Game::getPlayerPos();
    // get players chunk coordinates
    point pcc = worldToChunk(pp.x, pp.z);
    //std::cout << pcc.first << " " << pcc.second << " " << pl.x << " " << pl.z << std::endl;

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
            float sqrdist = (pp.x - cx)*(pp.x - cx) + (pp.z - cz)*(pp.z - cz);
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
    hardGenerating = false;
    for (size_t i = 0; i < newPoints.size(); ++i) {
        hardGenerating = true;
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
        float sqrdist = (pp.x - cx)*(pp.x - cx) + (pp.z - cz)*(pp.z - cz);
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

    glUniform1f(glGetUniformLocation(r.terrainShader.program, "time"), Game::getLavaTime());
    glUniform1f(glGetUniformLocation(r.terrainShader.program, "clerp"), Game::getLavaFlowRate());
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
    hardGenerating = true; // it will be generating
}

Terrain::Terrain() {
    setSeed(glm::vec2(Mth::randRange(-10000.0f, 10000.0f), Mth::randRange(-10000.0f, 10000.0f)));
}


Terrain::~Terrain() {
    deleteChunks();
}