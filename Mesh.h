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

//struct Texture


class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	GLuint texture;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, GLuint texture) {
		this->vertices = vertices;
		this->indices = indices;
		this->texture = texture;
		
		setupMesh();
	}

	void draw(Shader shader, GLuint amount) {
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

private:
	// render handles
	GLuint VAO, VBO, EBO;

	glm::vec3 color;

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