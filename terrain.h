#pragma once
#include "mesh.h"
#include <unordered_map>
#include "hsbColor.h"
#include "mathutil.h"
#include "resources.h"
#include "noise.h"
#include "aabb.h"

const int NUM_TILES = 20;
const float TILE_SIZE = 5.0f;
const float CHUNK_SIZE = NUM_TILES*TILE_SIZE;
const int CHUNK_RAD = 15;
//const float DIST = CHUNK_RAD * CHUNK_SIZE + 10.0f;
const float DIST = 1450.0f;

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

    float getHeight(float x, float z);
    void render();

    point pos;

private:
    std::vector<CTVertex> verts; // save these for collision detection

    void generateTerrain();
    void generateStructures();
    void spawnEntities();

    CTVertex genPoint(float xo, float yo);

    // delete statics objects out of physics
    void clearStatics();

    StandardMesh* mesh;
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

    void update(glm::vec3 pl);

    void render(glm::mat4 view, glm::mat4 proj);

    float queryHeight(float x, float z);

    void deleteChunks();

    void setSeed(glm::vec2 seed);

    bool toggleDebugColors();

private:
    // list of chunk objects
    std::vector<Chunk*> chunks;
    // map from chunks coords to vector index
    std::unordered_map<point, size_t> coordsByIndices;

    point worldToChunk(float x, float z);
};