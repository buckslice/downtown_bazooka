#include "mesh.h"

// helps make square texture look better on buildings
GLfloat _vn = 1.0f / 32.0f;

// each uv starts in bottom left (when looking at face) and progresses clockwise around
std::vector<TVertex> TIMesh::cubeVertices = {
    //front
    TVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    //back
    TVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    //left
    TVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) },
    //right
    TVertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    //bottom
    TVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },
    //top
    TVertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) }

};

std::vector<TVertex> TIMesh::offsetCubeVertices = {
    TVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    TVertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },
    TVertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    TVertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    TVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    TVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, _vn) },
    TVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    TVertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    TVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, _vn) },
    TVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, _vn) },
    TVertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    TVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    TVertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, _vn) },
    TVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, _vn) },
    TVertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f - _vn) },
    TVertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f - _vn) },

    TVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f) },

    TVertex{ glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 0.0f) },
    TVertex{ glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f) },
    TVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f) }

};

//GLfloat ed = 0.577350f;
GLfloat ed = 0.288675f;
glm::vec3 l = glm::vec3(-0.5f, -ed, -ed);
glm::vec3 u = glm::vec3(0.0f, ed * 2.0f, 0.0f);
glm::vec3 r = glm::vec3(0.5f, -ed, -ed);
glm::vec3 f = glm::vec3(0.0f, -ed, ed * 2.0f);

std::vector<TVertex> TIMesh::pyramidVertices = {
    TVertex{ l, glm::vec2(0.0f, 0.0f)},
    TVertex{ u, glm::vec2(0.5f, 1.0f)},
    TVertex{ r, glm::vec2(1.0f, 0.0f)},

    TVertex{ f, glm::vec2(0.0f, 0.0f) },
    TVertex{ u, glm::vec2(0.5f, 1.0f) },
    TVertex{ l, glm::vec2(1.0f, 0.0f) },

    TVertex{ r, glm::vec2(0.0f, 0.0f) },
    TVertex{ u, glm::vec2(0.5f, 1.0f) },
    TVertex{ f, glm::vec2(1.0f, 0.0f) },

    TVertex{ l, glm::vec2(0.0f, 0.0f) },
    TVertex{ r, glm::vec2(0.5f, 1.0f) },
    TVertex{ f, glm::vec2(1.0f, 0.0f) },
};

std::vector<TVertex> quadVerts{
    TVertex { glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
    TVertex { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
    TVertex { glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
    TVertex { glm::vec3(1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }
};

std::vector<GLuint> pyramidElements = {
    0,1,2,3,4,5,6,7,8,9,10,11
};

std::vector<GLuint> cubeElements = {
    0,1,2,
    2,3,0,

    4,5,6,
    6,7,4,

    8,9,10,
    10,11,8,

    12,13,14,
    14,15,12,

    16,17,18,
    18,19,16,

    20,21,22,
    22,23,20
};

std::vector<Vertex> PIMesh::cubeVertices = {
    //front
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f) },
    //back
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f) },
    //left
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f) },
    //right
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f) },
    //bottom
    Vertex{ glm::vec3(-0.5f, -0.5f, -0.5f) },
    Vertex{ glm::vec3(0.5f, -0.5f, -0.5f) },
    Vertex{ glm::vec3(0.5f, -0.5f, 0.5f) },
    Vertex{ glm::vec3(-0.5f, -0.5f, 0.5f) },
    //top
    Vertex{ glm::vec3(-0.5f, 0.5f, 0.5f) },
    Vertex{ glm::vec3(0.5f, 0.5f, 0.5f) },
    Vertex{ glm::vec3(0.5f, 0.5f, -0.5f) },
    Vertex{ glm::vec3(-0.5f, 0.5f, -0.5f) }

};