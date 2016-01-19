#include "terrainGenerator.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "input.h"

float seedX = Mth::rand0X(1000.0f);
float seedY = Mth::rand0X(1000.0f);

void Chunk::generate() {
    std::vector<GLuint> tris;
    tris.reserve(NUM_TILES*NUM_TILES * 6);
    verts.reserve((NUM_TILES + 1)*(NUM_TILES + 1));

    bool bot = true;
    glm::vec3 rand = HSBColor(Mth::rand01(), 1.0f, 1.0f).toRGB() * Mth::randUnit() * 0.1f;
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
            h *= scale;
            // to make it look blocky
            h = floorf(h / 4.0f) * 4;

            glm::vec3 p = glm::vec3(xo, h, yo);
            p -= glm::vec3(CHUNK_SIZE / 2.0f, 0.0f, CHUNK_SIZE / 2.0f);   // offset by half to center
            float t = h / scale;
            glm::vec3 color;
            if (t < .01f) {
                color = glm::vec3(0.02f, 0.02f, 0.05f);
                color += glm::vec3(Noise::ridged_2D(xo + seedX, yo + seedY, 2, 0.05f)*1.0f);
            } else if (t < .25f) {
                color = glm::vec3(0.05f, 0.05f, 0.1f);
                color += glm::vec3(Noise::ridged_2D(xo + seedX, yo + seedY, 2, 0.03f)*.75f);
            } else if (t < .45f) {
                color = glm::vec3(0.1f, 0.2f, 0.3f);
                color += glm::vec3(Noise::ridged_2D(xo + seedX, yo + seedY, 2, 0.015f)*0.75f);
            } else {
                color = glm::vec3(0.2f, 0.4f, 0.6f);
                color += glm::vec3(Noise::ridged_2D(xo + seedX, yo + seedY, 2, 0.0075f)*0.5f);
            }
            //color.b += Mth::randUnit() * 0.1f + 0.1f;
            color.b += 0.2f;
            color += rand;
            color.b = std::max(color.b, 0.1f);
            //color.r -= 0.2f;
            verts.push_back(CVertex{ p, color, glm::vec2(left ? 0.0f : 1.0f, bot ? 1.0f : 0.0f) });
            left = !left;
        }
        bot = !bot;
    }
    //std::cout << verts.size() << std::endl;

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
    //std::cout << tris.size() << std::endl;
    mesh = new ColorMesh(verts, tris, Resources::get().gridTex);
}

Chunk::Chunk(point pos) {
    this->pos = pos;
    generate();
}

Chunk::~Chunk() {
    delete mesh;
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

float TerrainGenerator::queryHeight(float x, float z) {
    point p = worldToChunk(x, z);
    if (!coordsByIndices.count(p)) {
        return -10000.0f;   // basically -infinity
    }

    return chunks[coordsByIndices[p]]->getHeight(x, z);
}

point TerrainGenerator::worldToChunk(float x, float z) {
    int px = (int)((x - CHUNK_SIZE / 2.0f * (x > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);
    int pz = (int)((z - CHUNK_SIZE / 2.0f * (z > 0.0f ? -1.0f : 1.0f)) / CHUNK_SIZE);

    return point{ px, pz };
}

int frames = 0;
void TerrainGenerator::update(glm::vec3 pl) {
    // get players chunk coordinates
    point pcc = worldToChunk(pl.x, pl.z);
    //std::cout << pcc.first << " " << pcc.second << " " << pl.x << " " << pl.z << std::endl;

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

            // if close enough then spawn new chunk
            float sqrdist = (pl.x - cx)*(pl.x - cx) + (pl.z - cz)*(pl.z - cz);
            if (sqrdist < DIST * DIST) {
                coordsByIndices[ccc] = chunks.size();
                chunks.push_back(new Chunk(ccc));
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
            c->mesh->destroy();
            delete c;

            // decrement to offset next increment since we lost an element
            i--;
        }
    }

    //std::cout << chunks.size() << " " << freeList.size() << std::endl;
}


void TerrainGenerator::render() {
    // render all terrain chunks
    Resources& r = Resources::get();

    //std::cout << chunks.size() << std::endl;
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        chunks[i]->mesh->draw();
    }

}

TerrainGenerator::~TerrainGenerator() {
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        chunks[i]->mesh->destroy();
        delete chunks[i];
    }
    chunks.erase(chunks.begin(), chunks.end());
}