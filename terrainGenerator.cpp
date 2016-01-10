#include "terrainGenerator.h"

float seedX = Mth::rand0X(1000.0f);
float seedY = Mth::rand0X(1000.0f);

void Chunk::generate(point chunkPos) {
    std::vector<CVertex> verts;
    std::vector<GLuint> tris;

    bool bot = true;
    //HSBColor col(Mth::rand01(), 1.0f, 1.0f);
    for (int y = 0; y < NUM_TILES + 1; y++) {
        bool left = true;
        for (int x = 0; x < NUM_TILES + 1; x++) {
            float xo = x * TILE_SIZE + pos.first * CHUNK_SIZE;
            float yo = y * TILE_SIZE + pos.second * CHUNK_SIZE;

            float scale = 100.0f;
            float h = Noise::fractal_2D(xo + seedX, yo + seedY, 4, 0.002f);
            if (h < 0.0f) {
                h = 0.0f;
            }
            h *= scale;  // scale 
            // to make it look blocky
            h = (int)(h / 4.0);
            h *= 4;

            glm::vec3 pos = glm::vec3(xo, h, yo);
            pos -= glm::vec3(CHUNK_SIZE / 2.0f, 0.0f, CHUNK_SIZE / 2.0f);   // offset by half to center
            float t = h / scale;
            glm::vec3 color;
            if (t < .01f) {
                color = glm::vec3(0.02f, 0.02f, 0.05f);
            } else if (t < .25f) {
                //color = glm::vec3(0.2f, 0.2f, 0.3f);
                color = glm::vec3(0.05f, 0.05f, 0.1f);
            } else if (t < .45f) {
                //color = glm::vec3(0.2f, 0.4f, 0.6f);
                color = glm::vec3(0.1f, 0.2f, 0.3f);
            } else {
                //color = glm::vec3(0.6f, 0.6f, 0.8f);
                color = glm::vec3(0.2f, 0.4f, 0.6f);
            }

            verts.push_back(CVertex{ pos, color, glm::vec2(left ? 0.0f : 1.0f, bot ? 1.0f : 0.0f) });
            left = !left;
        }
        bot = !bot;
    }
    //std::cout << verts.size() << std::endl;

    for (int i = 0; i < (NUM_TILES + 1)*(NUM_TILES)-1; i++) {
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
    //std::cout << tris.size() << std::endl;
    mesh = new ColorMesh(verts, tris, Resources::get().terrainTex);
}

Chunk::Chunk(point pos) {
    this->pos = pos;
    generate(pos);
}

Chunk::~Chunk() {
    delete mesh;
}

int frames = 0;
void TerrainGenerator::update(glm::vec3 pl) {
    // convert player position into chunk coordinates
    // have to do some extra shit for if its negative or not
    int px = (int)((pl.x - CHUNK_SIZE / 2.0f * (pl.x > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);
    int py = (int)((pl.z - CHUNK_SIZE / 2.0f * (pl.z > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);

    for (int y = -CHUNK_RAD; y <= CHUNK_RAD; y++) {
        for (int x = -CHUNK_RAD; x <= CHUNK_RAD; x++) {
            point tcp = point{ px + x, py + y };
            if (chunkCoords.count(tcp)) {
                continue;
            }

            float cx = (px + x)*CHUNK_SIZE;
            float cy = (py + y)*CHUNK_SIZE;

            float sqrdist = (pl.x - cx)*(pl.x - cx) + (pl.z - cy)*(pl.z - cy);
            if (sqrdist < DIST * DIST) {
                chunks.push_back(new Chunk(tcp));
                chunkCoords.insert(tcp);
            }
        }
    }

    // delete chunks no longer nearby player
    // dont need to check every frame
    if (frames < 10) {
        frames++;
        return;
    }
    frames = 0;
    for (int i = 0; i < chunks.size(); i++) {
        Chunk* c = chunks[i];
        float cx = c->pos.first * CHUNK_SIZE;
        float cy = c->pos.second * CHUNK_SIZE;
        float sqrdist = (pl.x - cx)*(pl.x - cx) + (pl.z - cy)*(pl.z - cy);
        if (sqrdist > DIST * DIST) {
            chunkCoords.erase(c->pos);
            c->mesh->destroy();
            delete c;
            std::swap(chunks[i], chunks.back());
            chunks.pop_back();
            i--;
        }
    }
}

void TerrainGenerator::render() {
    //std::cout << chunks.size() << std::endl;
    for (int i = 0; i < chunks.size(); i++) {
        chunks[i]->mesh->draw();
    }
}

TerrainGenerator::~TerrainGenerator() {
    for (int i = 0; i < chunks.size(); i++) {
        chunks[i]->mesh->destroy();
        delete chunks[i];
    }
    chunks.erase(chunks.begin(), chunks.end());
}