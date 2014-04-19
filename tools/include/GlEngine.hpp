#pragma once
/*!
 * \brief       OpenGL Stuff of the test/toy emulator
 * \file        GlEngine.hpp
 * \copyright   The MIT License (MIT)
 *
 * OpenGL Stuff of the test/toy emulator.
 */

#include "OS.hpp"

#include <iostream>
#include <functional>

#include <cassert>

#ifdef GLFW3_ENABLE

class GlEngine {
public:
    GlEngine () {
        winWidth  = 0;
        winHeight = 0;

        vertexSource = nullptr;
        fragmentSource = nullptr;

        yaw  = 0;
        pith = 0;
        zoom = 6.0;

        frame_count = 0;
        t_acu = 0;
    }

    ~GlEngine () {
        // RAII
        if (vertexSource != nullptr) {
            delete[] vertexSource;
        }

        if (fragmentSource != nullptr) {
            delete[] fragmentSource;
        }
    }

    //! Init OpenGL
    int initGL(OS::OS& os);

    //! Sets the Painter function of screen texture
    void SetTextureCB (std::function<void(void*)> painter);

    /*!
     * Updates the whole screen
     * \param os
     * \param delta delta in seconds
     */
    void UpdScreen (OS::OS& os, const double delta);

private:
    unsigned winWidth;
    unsigned winHeight;

    //bool capture_keyboard = false;

    GLuint screenTex; // ID of screen Texture
    GLuint tex_pbo;   // ID of the screen texture PBO

    // Handler of shader program
    GLuint shaderProgram;

    // Ptrs to source doe of shaders
    GLchar* vertexSource;
    GLchar* fragmentSource;

    // Handlers of the shader programs
    GLuint vertexShader;
    GLuint fragmentShader;

    // Handles for uniform inputs to the shader
    GLuint modelId;
    GLuint viewId;
    GLuint projId;
    GLuint timeId;

    static const unsigned int sh_in_Position;
    static const unsigned int sh_in_Color;
    static const unsigned int sh_in_UV;

    static const GLfloat N_VERTICES;

    GLuint vertexbuffer;
    static const float vdata[];

    GLuint colorbuffer;
    static const float color_data[];

    GLuint uvbuffer;
    static const float uv_data[];

    glm::mat4 proj, view, model; //! MVP Matrixes

    float yaw;
    float pith;
    float zoom;

    float frame_count;  //! Count frames
    double t_acu;       //! Acumulated time

    std::function<void(void*)> painter; //! Function that paints screen texture
};

#endif

