#pragma once

// openGL functions
#include <GL/glew.h>

// matrix math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "shader.h"

struct Vertex {
    glm::vec3 position;
    //glm::vec3 normal;
    glm::vec2 texcoord;
};

struct CVertex {
    glm::vec3 position;
    glm::vec3 color;    // maybe add in alpha later??
    glm::vec2 texcoord;
};

//struct Texture


class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    GLuint texture;

    GLuint modelBuffer;
    GLuint colorBuffer;

    bool visible = false;
    bool builtColors = false;
    bool builtModels = false;

    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, GLuint texture) {
        this->vertices = vertices;
        this->indices = indices;
        this->texture = texture;

        setupMesh();
    }

    void draw() {
        // set bindings
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        // draw
        if (amount == 0) {
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, amount);
        }

        // unbind stuff
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // not sure if need this but #safety?
    ~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    GLuint getVAO() {
        return VAO;
    }

    void setInstanceAmount(GLuint amount) {
        this->amount = amount;
    }


private:
    // render handles
    GLuint VAO, VBO, EBO;

    glm::vec3 color;

    GLuint amount = 0;  // if 0 no instancing, if > 0 then instanced

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind array to remember attributes
        glBindVertexArray(VAO);
        // load vertex buffer on GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        // load element buffer to gpu
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

        // set up attribute pointers
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        //glEnableVertexAttribArray(1);
        //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));

        glBindVertexArray(0);
    }

};


class ColorMesh {
public:
    // why the foke am i saving the data?
    std::vector<CVertex> vertices;
    std::vector<GLuint> indices;
    GLuint texture;

    GLuint modelBuffer;
    GLuint colorBuffer;

    ColorMesh() {

    }

    ColorMesh(std::vector<CVertex> vertices, std::vector<GLuint> indices, GLuint texture) {
        this->vertices = vertices;
        this->indices = indices;
        this->texture = texture;

        setupMesh();
    }

    void draw() {
        // set bindings
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        // draw
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // unbind stuff
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void destroy() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    GLuint getVAO() {
        return VAO;
    }

private:
    // render handles
    GLuint VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind array to remember attributes
        glBindVertexArray(VAO);
        // load vertex buffer on GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(CVertex), &vertices[0], GL_STATIC_DRAW);
        // load element buffer to gpu
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

        // set up attribute pointers
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CVertex), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CVertex), (GLvoid*)offsetof(CVertex, color));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CVertex), (GLvoid*)offsetof(CVertex, texcoord));

        glBindVertexArray(0);
    }

};