#pragma once
#include <GL/glew.h>
#include "glHelper.h"
#include "shader.h"

// follows meyers singleton pattern
class Resources {
public:
    static Resources& get() {
        static Resources res;
        return res;
    }

    GLuint gridTex;
    //GLuint terrainTex;
    GLuint solidTex;

    Shader instanceShader;
    Shader tiledShader;
    Shader terrainShader;
    Shader blurShader;
    Shader screenShader;
    Shader blendShader;

    void buildShaders();

private:
    Resources();

    ~Resources();

    Resources(Resources const&);   // hide copy ctor
    Resources& operator=(Resources const&); // hide assign op

    void deleteShaders();
};
