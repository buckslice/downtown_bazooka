#pragma once
#include <GL/glew.h>
#include <SFML/Graphics.hpp>


struct FBO {
    GLuint frame;
    GLuint color;
    GLuint depth;

    // not in destructor since structs pass by value
    // when buildFBO returns the fbo struct it made will
    // have its destructor called (after it is copied)
    void destroy() {
        glDeleteBuffers(1, &frame);
        glDeleteTextures(1, &color);
    }
};

class GLHelper {
public:
    static GLuint loadTexture(std::string path, bool mipped);

    static GLuint loadCubeMap(std::vector<std::string> faces);

    static FBO buildFBO(GLuint width, GLuint height, bool withDepth);
};