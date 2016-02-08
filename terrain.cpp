#include "terrain.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "input.h"

// just public here in the class so chunk can see
glm::vec2 seed;
bool debugColors = false;

void Terrain::setSeed(glm::vec2 sd) {
    seed = sd;
}
bool Terrain::toggleDebugColors() {
    debugColors = !debugColors;
    return debugColors;
}

float minh, maxh;
CVertex Chunk::genPoint(float x, float y) {
    const int max = 2;
    double f[max];
    uint32_t id[max];
    Noise::worley(x, y, 0.0f, max, f, id, Noise::MANHATTAN, 0.005f);

    float h = static_cast<float>(f[1] - f[0]);

    //minh = std::min(minh, h);
    //maxh = std::max(maxh, h);

    h = Noise::fractal_worley_3D(x + seed.x, y + seed.y, 0.0f, Noise::MANHATTAN, 2, 0.005f);

    h -= .025f;
    h = Mth::clamp(h, 0.0f, 1.0f);

    // reduce noise height near center
    float sqrdist = x*x + y*y;
    float cityLimit = 1000.0f;;
    float range = 100.0f;
    if (sqrdist < cityLimit * cityLimit) {  // inside city
        h = 0.0f;
    } else {
        // add general mountain noise (should later just increase worley octaves
        //h += Noise::ridged_2D(x + seed.x, y + seed.y, 2, 0.008f) * 0.3f * h;
        if (sqrdist < (cityLimit + range)*(cityLimit + range)) { // between city and mountains
            float d = glm::distance(glm::vec2(0.0f), glm::vec2(x, y));
            float b = Mth::blend(d, cityLimit, cityLimit + range, Mth::cubic);
            h *= b;
        }
    }
    h = Mth::clamp(h, 0.0f, 1.0f);
    // to make it look blocky
    //h = floorf(h / 4.0f) * 4;

    glm::vec3 color;
    if (!debugColors) {
        int idc = 1;    // 0 is whole cell, 1 is sides of cell, 2 is subcells within sides?
        glm::vec3 rand = glm::vec3((id[idc] % 255) / 255.0f, (id[idc] % 155) / 255.0f, (id[idc] & 100) / 255.0f)*0.2f;
        if (h < 0.01f) {
            color = glm::vec3(Noise::ridged_2D(x + seed.x, y + seed.y, 2, 0.05f));
            color += glm::vec3(0.02f, 0.02f, 0.25f);
            color.r = Mth::clamp(color.r, 0.0f, 0.5f);
            color.g = Mth::clamp(color.g, 0.0f, 0.5f);
            color.b = Mth::clamp(color.b, 0.15f, 1.0f);
        } else {
            float r = Mth::cblend(h, 0.4f, 0.6f)*0.9f;
            float g = Mth::cblend(h, 0.2f, 0.5f);
            float b = Mth::cblend(h, 0.0f, 0.3f) * .85f + 0.15f;
            color = glm::vec3(r, g, b);
        }
        color += rand;
    }
    float scale = 100.0f;
    h *= scale;

    return CVertex{ glm::vec3(x, h, y), color, glm::vec2() };
}

CVertex Chunk::genTest(float x, float y) {
    const int max = 3;
    double f[max];
    uint32_t id[max];
    Noise::worley(x, y, 0.0f, max, f, id, Noise::EUCLIDIAN, 0.0075f);

    float h = static_cast<float>(f[1] - f[0]);
    //float h = Noise::fractal_worley_3D(x, y, 0.0f, Noise::MANHATTAN, 3, 0.0075f);

    int idc = 1;    // 0 is whole cell, 1 is sides of cell, 2 is subcells within sides?
    glm::vec3 rand = glm::vec3((id[idc] % 255) / 255.0f, (id[idc] % 155) / 255.0f, (id[idc] & 100) / 255.0f);
    h = Mth::clamp(h, 0.0f, 1.0f);
    glm::vec3 color = glm::vec3(rand);
    //glm::vec3 color = glm::vec3(h);

    h *= 100.0f;
    return CVertex{ glm::vec3(x, h, y), color, glm::vec2() };
}

void Chunk::generate() {
    std::vector<GLuint> tris;
    tris.reserve(NUM_TILES*NUM_TILES * 6);
    verts.reserve((NUM_TILES + 1)*(NUM_TILES + 1));

    // used for texture coordinates
    bool bot = true;

    //minh = 1000.0f;
    //maxh = -1000.0f;

    // debug color
    glm::vec3 debugColor = HSBColor(Mth::rand01(), 1.0f, 1.0f).toRGB();

    // TODO: get generation code to work from YAML files or something
    // so dont have to recompile to see changes!!!
    for (int y = 0; y < NUM_TILES + 1; y++) {
        bool left = true;
        for (int x = 0; x < NUM_TILES + 1; x++) {
            float xo = x * TILE_SIZE + pos.first * CHUNK_SIZE - CHUNK_SIZE / 2.0f;
            float yo = y * TILE_SIZE + pos.second * CHUNK_SIZE - CHUNK_SIZE / 2.0f;

            CVertex cv = genPoint(xo, yo);
            if (debugColors) {
                cv.color = debugColor;
            }

            // texcoords are mirrored to work with XTREME!!! vertex sharing,
            // limitation is that textures have to be symmetric
            cv.texcoord = glm::vec2(left ? 0.0f : 1.0f, bot ? 1.0f : 0.0f);

            verts.push_back(cv);
            left = !left;
        }
        bot = !bot;
    }
    //std::cout << verts.size() << std::endl;
    //std::cout << minh << " " << maxh << std::endl;

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
    mesh = new ColorMesh(verts, tris, Resources::get().terrainTex);
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

    int numBuiltThisFrame = 0;
    int maxBuiltPerFrame = 10;
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
                if (++numBuiltThisFrame >= maxBuiltPerFrame) {
                    goto outer;
                }
            }
        }
    }
outer:  // jump here if built max this frame

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

}

void Terrain::render() {
    // render all terrain chunks
    Resources& r = Resources::get();

    //std::cout << chunks.size() << std::endl;
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        chunks[i]->mesh->draw();
    }

}

void Terrain::deleteChunks() {
    for (size_t i = 0, len = chunks.size(); i < len; ++i) {
        chunks[i]->mesh->destroy();
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