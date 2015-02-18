#pragma once
/*!
 * \brief       OpenGL Stuff of the test/toy emulator
 * \file        gl_engine.hpp
 * \copyright   LGPL v3
 *
 * OpenGL Stuff of the test/toy emulator.
 */

#include "os.hpp"

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
        zoom = 4.7;

        frame_count = 0;
        t_acu = 0;

        vertShaderFile = "basic_vs.vert";
        fragShaderFile = "retro_texture.frag";
        this->pbo = 0;
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

    void setVertexShaderFile (const std::string& file) {
        this->vertShaderFile = file;
    }
    void setFragmentShaderFile (const std::string& file) {
        this->fragShaderFile = file;
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
    GLuint tex_pbo[2];// IDs of the screen texture PBO
    size_t pbo;

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

	static const GLsizei N_VERTICES;

    GLuint vao, vbo[3]; // vbo = {vdata, color, uv}
    static const float vdata[];
    static const float color_data[];
    static const float uv_data[];

    glm::mat4 proj, view, model; //! MVP Matrixes

    // Camera position
    float yaw;
    float pith;
    float zoom;

    float frame_count;  //! Count frames
    double t_acu;       //! Acumulated time

    std::function<void(void*)> painter; //! Function that paints screen texture

    std::string vertShaderFile;
    std::string fragShaderFile;
};

#endif

