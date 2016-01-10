#pragma once
#include <GL/glew.h>
#include "glHelper.h"

// follows meyers singleton pattern
class Resources {
public:
    static Resources& get() {
        static Resources res;
        return res;
    }

    GLuint gridTex;
    GLuint terrainTex;
    GLuint solidTex;


private:
    Resources() {
        gridTex = GLHelper::loadTexture("assets/images/grid.png");
        terrainTex = GLHelper::loadTexture("assets/images/grid2.png");
        solidTex = GLHelper::loadTexture("assets/images/solid.png");

        //gridFloorTex = GLHelper::loadTex
    }

    ~Resources() {
        glDeleteTextures(1, &gridTex);
        glDeleteTextures(1, &terrainTex);
        glDeleteTextures(1, &solidTex);
    }

    Resources(Resources const&);   // hide copy ctor
    Resources& operator=(Resources const&); // hide assign op

};
