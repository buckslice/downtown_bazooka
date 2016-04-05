#pragma once

// openGL functions
#include <GL/glew.h>

// matrix math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "shader.h"

extern std::vector<GLuint> cubeElements;

struct Vertex {
    Vertex(glm::vec3 position) :
        position(position) {
    }
    glm::vec3 position;
};

struct TVertex : Vertex {
    TVertex(glm::vec3 position, glm::vec2 texcoord) :
        Vertex(position), texcoord(texcoord) {
    }
    glm::vec2 texcoord;
};

//struct CVertex : Vertex {
//    glm::vec3 color;
//};

struct CTVertex : Vertex {
    CTVertex(glm::vec3 position, glm::vec3 color, glm::vec2 texcoord) :
        Vertex(position), color(color), texcoord(texcoord) {
    }
    glm::vec3 color;
    glm::vec2 texcoord;
};

template<class Vertex>
class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    const GLuint TRIS;
    bool visible = false;

    virtual void render() = 0;

    Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) : TRIS(indices.size()) {
        this->vertices = vertices;
        this->indices = indices;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind array to remember attributes
        glBindVertexArray(VAO);

        // load vertex buffer onto GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // load element buffer onto GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

        // set up attribute pointers
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

        glBindVertexArray(0);
    }

    ~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

protected:
    // render handles
    GLuint VAO, VBO, EBO;
};

// used for standard models (none in game yet) and terrain
class StandardMesh : Mesh<CTVertex> {
    // bool manual mesh binding?

public:

    GLuint texture;
    //bool manualBinding = true;

    StandardMesh(std::vector<CTVertex>& vertices, std::vector<GLuint> indices, GLuint texture) : Mesh(vertices, indices) {
        this->texture = texture;
        // set up attribute pointers
        glBindVertexArray(VAO);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CTVertex), (GLvoid*)offsetof(CTVertex, color));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CTVertex), (GLvoid*)offsetof(CTVertex, texcoord));

        glBindVertexArray(0);
    }

    void render() override {
        // check visibility (or do in base?)
        // set bindings
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        // draw
        glDrawElements(GL_TRIANGLES, TRIS, GL_UNSIGNED_INT, 0);

        // unbind stuff
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


};

template <class Vertex>
class IMesh : public Mesh<Vertex> {
public:
    GLuint instanceCount, colorBuffer, modelBuffer;
    bool built = false;

    IMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) : Mesh<Vertex>(vertices, indices) {
    }

    ~IMesh() {
        if (built) {
            glDeleteBuffers(1, &colorBuffer);
            glDeleteBuffers(1, &modelBuffer);
        }
    }

    // sets up instancing buffers starting at attribute 'start'
    // could probably just make constructor do this but couldn't figure out how
    // to handle adding additional attributes correctly in child classes
    void setInstanceBuffers(GLuint attribStart) {
        if (built) {
            std::cout << "ERROR::ALREADY_BUILT_INSTANCE_BUFFER" << std::endl;
            return;
        }

        glBindVertexArray(VAO);
        // build color buffer
        glGenBuffers(1, &colorBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glEnableVertexAttribArray(attribStart);
        glVertexAttribPointer(attribStart, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
        glVertexAttribDivisor(attribStart, 1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // build model buffer
        glGenBuffers(1, &modelBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(++attribStart);
            glVertexAttribPointer(attribStart, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(i * sizeof(glm::vec4)));
            glVertexAttribDivisor(attribStart, 1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        built = true;
    }

    // stream should be set if this function is getting called each frame
    void setColors(std::vector<glm::vec3>& colors, bool stream, int count = 0) {
        if (colors.size() == 0 || !built) {
            return;
        }
        if (count == 0) {
            count = colors.size();
        }
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), &colors[0], stream ? GL_STREAM_DRAW : GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void setModels(std::vector<glm::mat4>& models, bool stream, int count = 0) {
        if (models.size() == 0 || !built) {
            visible = false;
            return;
        }
        if (count == 0) {
            count = models.size();
        }
        visible = true;
        instanceCount = count;
        glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::mat4), &models[0], stream ? GL_STREAM_DRAW : GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

};

class PIMesh : public IMesh <Vertex> {
public:
    PIMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices) : IMesh<Vertex>(vertices, indices) {
        setInstanceBuffers(1);
    }

    void render() override {
        if (instanceCount <= 0) {
            return;
        }

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, TRIS, GL_UNSIGNED_INT, 0, instanceCount);
        glBindVertexArray(0);
    }

    static std::vector<Vertex> cubeVertices;
};


class TIMesh : public IMesh<TVertex> {
public:
    GLuint texture;

    TIMesh(std::vector<TVertex>& vertices, std::vector<GLuint>& indices, GLuint texture) : IMesh<TVertex>(vertices, indices) {
        this->texture = texture;
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TVertex), (GLvoid*)offsetof(TVertex, texcoord));
        glBindVertexArray(0);
        setInstanceBuffers(2);
    }

    void render() override {
        if (instanceCount <= 0) {
            return;
        }

        // set bindings
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        glDrawElementsInstanced(GL_TRIANGLES, TRIS, GL_UNSIGNED_INT, 0, instanceCount);

        // unbind stuff
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static std::vector<TVertex> cubeVertices, offsetCubeVertices;

};