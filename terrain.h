#pragma once
#include "mesh.h"
#include <unordered_map>
#include "hsbColor.h"
#include "mathutil.h"
#include "noise.h"
#include "aabb.h"

const int NUM_TILES = 20;
const float TILE_SIZE = 5.0f;
const float CHUNK_SIZE = NUM_TILES*TILE_SIZE;
const int CHUNK_LOAD_RADIUS = 15;
const float LOAD_DIST = CHUNK_LOAD_RADIUS * CHUNK_SIZE - 50.0f;

const float CITY_SIZE = 2000.0f;    // distance from one side to another
const float LOW_HEIGHT = 20.0f;     // for building generation
const float HIGH_HEIGHT = 60.0f;

// hash function for pairs
// should figure out how to get it working for sf::Vector2i
// or use typedefs so i dont have to type std::pair<int,int> so much
template <class T>
inline void hash_combine(std::size_t & seed, const T & v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
    template<typename S, typename T> struct hash<pair<S, T>> {
        inline size_t operator()(const pair<S, T> & v) const {
            size_t seed = 0;
            ::hash_combine(seed, v.first);
            ::hash_combine(seed, v.second);
            return seed;
        }
    };
}

typedef std::pair<int, int> point;

class Chunk {
public:
    Chunk(point pos);
    ~Chunk();

    void render();

    float getHeight(float x, float z) const;

    point pos;
    StandardMesh* mesh;
private:
    class RNG {
    public:
        RNG(point pos, float cx, float cz, float hc):
            unix(cx - hc, cx + hc),
            uniz(cz - hc, cz + hc),
            zeroToOne(0.0f, 1.0f){
            // use a seeded random generator for this chunk based on its chunk coords
            // this way the building positions and shapes will be the same each time
            int rngseed = (pos.first + pos.second)*(pos.first + pos.second + 1) / 2 + pos.second;
            rng.seed(rngseed);
        }
        // returns random x value within chunk boundaries
        float x() { 
            return unix(rng);
        }
        // returns random z value within chunk boundaries
        float z() { 
            return uniz(rng);
        }
        // returns random value from 0-1
        float rand() { 
            return zeroToOne(rng);
        }

    private:
        std::mt19937 rng;
        std::uniform_real_distribution<float> unix;
        std::uniform_real_distribution<float> uniz;
        std::uniform_real_distribution<float> zeroToOne;
    };

    std::vector<glm::vec3> verts; // save these for collision detection

    void generateTerrain();
    void generateStructures(RNG& rng);
    void generateFinalBattle(RNG& rng);
    void spawnEntities(RNG& rng);

    CVertex genPoint(float xo, float yo);

    // delete statics objects out of physics
    void clearStatics();

    std::vector<glm::mat4> buildingModels;
    std::vector<glm::vec3> buildingColors;
    std::vector<glm::mat4> treeModels;
    std::vector<glm::vec3> treeColors;

    std::vector<int> staticIndices;
};

class Terrain {
public:

    Terrain();

    ~Terrain();

    void update(float delta);

    void render(glm::mat4 view, glm::mat4 proj);

    float queryHeight(float x, float z);

    void deleteChunks();

    void setSeed(glm::vec2 seed);

    bool toggleDebugColors();

    // whether terrain generated last frame or is about to
    static bool hardGenerating;

private:
    // list of chunk objects
    std::vector<Chunk*> chunks;
    // map from chunks coords to vector index
    std::unordered_map<point, size_t> coordsByIndices;

    point worldToChunk(float x, float z) const;

    std::uniform_real_distribution<float> un;

};