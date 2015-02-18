/**
 * Trillek Virtual Computer - tda_view.cpp
 * Tool that visualizes a image of a TDA screen using a stored TDA state
 *
 * \copyright   LGPL v3
 */
#include "os.hpp"
#include "devices/tda.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <ios>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <cwctype>
#include <clocale>
#include <chrono>

#if WIN32
#include <shlobj.h>
#include <winerror.h>
#include <atlbase.h>
#include <comutil.h>
#pragma comment(lib, "comsuppw")
#endif

#include "gl_engine.hpp"

int main (int argc, char* argv[]) {
    using namespace trillek;
    using namespace trillek::computer::tda;
    GlEngine gl;

    // TODO load screen from a file

    OS::OS glfwos;
    if (!glfwos.InitializeWindow(1024, 768, "TDA screen dump viewer")) {
        std::clog << "Failed creating the window or context.";
        return -1;
    }

    if ( gl.initGL(glfwos) != 0) {
        std::clog << "Error initiating OpenGL\n";
        return -1;
    }

    std::printf("Initiated OpenGL\n");

    // Test screen
    TDAScreen screen;
    screen.txt_buffer[0]  = 0x0F00 | 'H';
    screen.txt_buffer[1]  = 0x1F00 | 'e';
    screen.txt_buffer[2]  = 0x2F00 | 'l';
    screen.txt_buffer[3]  = 0x3F00 | 'l';
    screen.txt_buffer[4]  = 0x4F00 | 'o';
    screen.txt_buffer[5]  = 0x5F00 | ' ';
    screen.txt_buffer[6]  = 0x6F00 | 'w';
    screen.txt_buffer[7]  = 0x7F00 | 'o';
    screen.txt_buffer[8]  = 0x8F00 | 'r';
    screen.txt_buffer[9]  = 0x9F00 | 'l';
    screen.txt_buffer[10] = 0xAF00 | 'd';
    screen.txt_buffer[11] = 0xBF00 | '!';

    screen.cursor = true;
    screen.cur_col = 20;
    screen.cur_row = 0;
    screen.cur_start = 5;
    screen.cur_end = 7;
    screen.cur_color = 15;

    for (unsigned i= 40; i < WIDTH_CHARS*HEIGHT_CHARS; i++ ) {
        Byte fg = i % 16;
        Byte bg = (15 - i) % 16;
        screen.txt_buffer[i] = (bg << 12) | (fg << 8) | ((i-40) % 256);
    }

    // Function to update texture from TDAScreen object
    gl.SetTextureCB ([&screen] (void* tdata) {
        // Update Texture callback
        DWord* tex = (DWord*)tdata;
        TDAtoRGBATexture(screen, tex); // Write the texture to the PBO buffer
    });

    using namespace std::chrono;
    auto clock = high_resolution_clock::now();
    double delta; // Time delta in seconds

    bool loop = true;
    while ( loop) {
        // Calcs delta time

        auto oldClock = clock;
        clock = high_resolution_clock::now();
        delta = duration_cast<milliseconds>(clock - oldClock).count();

        if (glfwos.Closing()) {
            loop = false;
            continue;
        }
        // Ugly hack
        for(unsigned long i=0; i < 10000000 ; i++) {
            ;
        }

        gl.UpdScreen (glfwos, delta);
    }

    // Cleanup all the things we bound and allocated
    /*
    glUseProgram(0);
    glDisableVertexAttribArray(sh_in_Position);
    glDisableVertexAttribArray(sh_in_Color);
    glDisableVertexAttribArray(sh_in_UV);
    /*glDetachShader(shaderprogram, vertexshader);
    glDetachShader(shaderprogram, fragmentshader);
    glDeleteProgram(shaderprogram);
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);*/ /*
    glDeleteBuffers(3, vbo);
    glDeleteVertexArrays(1, &vao);
    */
    glfwos.Terminate();
    return 0;
}


