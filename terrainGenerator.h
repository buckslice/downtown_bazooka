#pragma once
#include "mesh.h"
#include <unordered_map>
#include "hsbColor.h"
#include "mathutil.h"
#include "resources.h"
#include "noise.h"

const int NUM_TILES = 20;
const float TILE_SIZE = 5.0f;
const float CHUNK_SIZE = NUM_TILES*TILE_SIZE;
const int CHUNK_RAD = 10;
const float DIST = CHUNK_RAD * CHUNK_SIZE + 10.0f;

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

    std::pair<int, int> pos;
    ColorMesh* mesh;

private:
    std::vector<CVertex> verts; // save these for collision detection

    void generate();
};

class TerrainGenerator {
public:

    TerrainGenerator() {
    }
    ~TerrainGenerator();

    void update(glm::vec3 pl);

    void render();

    float queryHeight(float x, float z);

private:
    std::vector<Chunk*> chunks; // list of actual chunk objects
    //std::unordered_map<point, int> chunkCoords;   // maps chunk coordinates to the chunk vector index
    std::unordered_map<point, size_t> coordsByIndices;
    point worldToChunk(float x, float z);
};