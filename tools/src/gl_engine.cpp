/*!
 * \brief       OpenGL Stuff of the test/toy emulator
 * \file        gl_engine.cpp
 * \copyright   LGPL v3
 *
 * OpenGL Stuff of the test/toy emulator.
 */

#include "gl_engine.hpp"

#if WIN32
#include <shlobj.h>
#include <winerror.h>
#include <atlbase.h>
#include <comutil.h>
#pragma comment(lib, "comsuppw")
#endif

#ifdef GLFW3_ENABLE

const unsigned int GlEngine::sh_in_Position = 0;
const unsigned int GlEngine::sh_in_Color = 1;
const unsigned int GlEngine::sh_in_UV = 3;

const GLfloat GlEngine::N_VERTICES = 4;

const float GlEngine::vdata[] = {
    3.2,  2.4, 0.0, // Top Right
    -3.2,  2.4, 0.0, // Top Left
    3.2, -2.4, 0.0, // Botton Right
    -3.2, -2.4, 0.0, // Bottom Left
};

const float GlEngine::color_data[] = {
    1.0,  1.0, 1.0, // Top Right
    1.0,  1.0, 1.0, // Top Left
    1.0,  1.0, 1.0, // Botton Right
    1.0,  1.0, 1.0, // Bottom Left
};

const float GlEngine::uv_data[] = {
    1.0,  0.0,      // Top Right
    0.0,  0.0,      // Top Left
    1.0,  1.0,      // Botton Right
    0.0,  1.0,      // Bottom Left
};


void GlEngine::SetTextureCB (std::function<void(void*)> painter) {
    this->painter = painter;
}


int GlEngine::initGL(OS::OS& os) {
    int OpenGLVersion[2];

    // Use the GL3 way to get the version number
    glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
    std::cout << "OpenGL " << OpenGLVersion[0] << "." << OpenGLVersion[1] << "\n";

    // Sanity check to make sure we are at least in a good major version number.
    assert((OpenGLVersion[0] > 1) && (OpenGLVersion[0] < 5));
    if (OpenGLVersion[0] < 3 || (OpenGLVersion[0] == 3 && OpenGLVersion[1] < 2) ) {
        std::cerr << "Error!\nWe need OpenGL 3.2 at least!";
        return -1;
    }

    winWidth = os.GetWindowWidth();
    winHeight = os.GetWindowHeight();

    // Determine the aspect ratio and sanity check it to a safe ratio
    GLfloat aspectRatio = static_cast<float>(winWidth) / static_cast<float>(winHeight);
    if (aspectRatio < 1.0f) {
        aspectRatio = 4.0f / 3.0f;
    }

    // Projection matrix : 45° Field of View
    proj = glm::perspective(
            45.0f,      // FOV
            aspectRatio,
            0.1f,       // Near cliping plane
            10000.0f);  // Far cliping plane

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
#if __APPLE__
    // GL_MULTISAMPLE are Core.
    glEnable(GL_MULTISAMPLE);
#else
    if (GLEW_ARB_multisample) {
        glEnable(GL_MULTISAMPLE_ARB);
    }
#endif
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Initialize VBOs ********************************************************
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Upload data to VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_STATIC_DRAW);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    // Upload data to VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_data), color_data, GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    // Upload data to VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_data), uv_data, GL_STATIC_DRAW);

    // Initialize PBO *********************************************************
    glGenBuffers(1, &tex_pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex_pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 320*240*4, nullptr, GL_STREAM_DRAW);

    // Initialize Texture *****************************************************
    glGenTextures(1, &screenTex);
    glBindTexture(GL_TEXTURE_2D, screenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Loading shaders ********************************************************
    FILE* f_vs = nullptr;
    FILE* f_fs = nullptr;
#if WIN32
    const std::string vertShaderFilename = "assets\\shaders\\mvp_template.vert";
    const std::string fragShaderFilename = "assets\\shaders\\retro_texture.frag";
    std::string programFilesPath = "";
    LPWSTR wszPath = nullptr;
    HRESULT hr;

    hr = SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &wszPath);
    if (SUCCEEDED(hr) ) {
        _bstr_t bstrPath(wszPath);
        programFilesPath = (char*)bstrPath;
        std::string path = programFilesPath + "\\trillek-tools\\"+ vertShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_vs = std::fopen(path.c_str(), "r");

        path = programFilesPath + "\\trillek-tools\\" + fragShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_fs = std::fopen(path.c_str(), "r");

        CoTaskMemFree(wszPath);
    }
#else
    const std::string vertShaderFilename = "assets/shaders/mvp_template.vert";
    const std::string fragShaderFilename = "assets/shaders/retro_texture.frag";
    std::string path = "./"+ vertShaderFilename;
    std::clog << "Trying " << path << std::endl;
    f_vs = std::fopen(path.c_str(), "r");
    if (f_vs == nullptr) {
        path = "/usr/share/trillek-tools/"+ vertShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_vs = std::fopen(path.c_str(), "r");
    }
    if (f_vs == nullptr) {
        path = "/usr/local/share/trillek-tools/"+ vertShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_vs = std::fopen(path.c_str(), "r");
    }
    if (f_vs == nullptr) {
        path = "/opt/trillek-tools/"+ vertShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_vs = std::fopen(path.c_str(), "r");
    }

    path = "./"+ fragShaderFilename;
    std::clog << "Trying " << path << std::endl;
    f_fs = std::fopen(path.c_str(), "r");
    if (f_fs == nullptr) {
        path = "/usr/share/trillek-tools/"+ fragShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_fs = std::fopen(path.c_str(), "r");
    }
    if (f_fs == nullptr) {
        path = "/usr/local/share/trillek-tools/"+ fragShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_fs = std::fopen(path.c_str(), "r");
    }
    if (f_fs == nullptr) {
        path = "/opt/trillek-tools/"+ fragShaderFilename;
        std::clog << "Trying " << path << std::endl;
        f_fs = std::fopen(path.c_str(), "r");
    }

#endif

    if (f_vs != nullptr) {
        fseek(f_vs, 0L, SEEK_END);
        size_t bufsize = ftell(f_vs);
        vertexSource = new GLchar[bufsize + 1]();

        fseek(f_vs, 0L, SEEK_SET);
        auto t = fread(vertexSource, sizeof(GLchar), bufsize, f_vs);
        if (t <= 0) {
            std::cerr << "Error reading Vertex Shader\n";
            return -1;
        }

        fclose(f_vs);
        vertexSource[bufsize] = 0; // Enforce null char
    } else {
        std::cerr << "Can't load Vertex Shader\n";
        return -1;
    }

    if (f_fs != nullptr) {
        fseek(f_fs, 0L, SEEK_END);
        size_t bufsize = ftell(f_fs);
        fragmentSource = new GLchar[bufsize +1 ]();

        fseek(f_fs, 0L, SEEK_SET);
        auto t = fread(fragmentSource, sizeof(GLchar), bufsize, f_fs);
        if (t <= 0) {
            std::cerr << "Error reading Fragment Shader\n";
            return -1;
        }

        fclose(f_fs);
        fragmentSource[bufsize] = 0; // Enforce null char
    } else {
        std::cerr << "Can't load Fragment Shader\n";
        return -1;
    }

    // Assign our handles a "name" to new shader objects
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Associate the source code buffers with each handle
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

    // Compile our shader objects
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    if (vertexSource != nullptr) {
        delete[] vertexSource;
        vertexSource = nullptr;
    }

    if (fragmentSource != nullptr) {
        delete[] fragmentSource;
        fragmentSource = nullptr;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Vertex Shader compiling error messages
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    GLchar* vertexshader_error_msg = new GLchar[InfoLogLength +1];
    glGetShaderInfoLog(vertexShader, InfoLogLength, NULL, vertexshader_error_msg);
    if (vertexshader_error_msg != nullptr) {
        if (vertexshader_error_msg[0] != '\0') { // Error compiling vertex shader
            std::fprintf(stderr, "> %s\n", vertexshader_error_msg);
            delete[] vertexshader_error_msg;
            return -1;
        }
        delete[] vertexshader_error_msg;
    }

    // Fragment Shader compiling error messages
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    GLchar* fragmentshader_error_msg = new GLchar[InfoLogLength +1];
    glGetShaderInfoLog(fragmentShader, InfoLogLength, NULL, fragmentshader_error_msg);
    if (fragmentshader_error_msg != nullptr) {
        if (fragmentshader_error_msg[0] != '\0') { // Error compiling fragment shader
            std::fprintf(stderr, "> %s\n", fragmentshader_error_msg);
            delete[] fragmentshader_error_msg;
            return -1;
        }
        delete[] fragmentshader_error_msg;
    }

    // Assign our program handle a "name"
    shaderProgram = glCreateProgram();

    // Attach our shaders to our program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Link shader program
    glLinkProgram(shaderProgram);

    // Bind attributes indexes
    glBindAttribLocation(shaderProgram, sh_in_Position, "in_Position");
    glBindAttribLocation(shaderProgram, sh_in_Color, "in_Color");
    glBindAttribLocation(shaderProgram, sh_in_UV, "in_UV");

    modelId = glGetUniformLocation(shaderProgram, "in_Model");
    viewId  = glGetUniformLocation(shaderProgram, "in_View");
    projId  = glGetUniformLocation(shaderProgram, "in_Proj");

    timeId = glGetUniformLocation(shaderProgram, "time");


    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // Camera matrix
    view = glm::lookAt(
            glm::vec3(3,3,7), // Camera is at (3,3,7), in World Space
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
            );

    return 0;
}

void GlEngine::UpdScreen (OS::OS& os, const double delta) {
    t_acu += delta;

    // Clear The Screen And The Depth Buffer
    frame_count += 1.0;

    glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Model matrix <- Identity
    model = glm::mat4(1.0f);

    // Camera Matrix
    view = glm::lookAt(
            glm::vec3(
                (float)(zoom * sin(yaw)*cos(pith)),
                (float)(zoom * sin(pith)),
                (float)(zoom * cos(yaw))*cos(pith)), // Were is the camera
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
            );

    // Drawing ...
    glUseProgram(shaderProgram);
    // Set sampler to user Texture Unit 0
    glUniform1i(glGetUniformLocation( shaderProgram, "texture0" ), 0);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTex);

    if (t_acu >= 0.04 || delta == 0) { // Updates screen texture at a rate of ~25 Hz
        t_acu -= 0.04;

        // Stream the texture *************************************************
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex_pbo);

        // Copy the PBO to the texture
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        // Updates the PBO with the new texture
        auto tdata = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (tdata != nullptr) {
            //std::fill_n(tdata, 320*240, 0xFF800000);
            if (painter) {
                painter(tdata);
            }

            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // Release the PBO
    }

    // Enable attribute in_Position as being used
    glEnableVertexAttribArray(sh_in_Position);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    // Vertex data to attribute index 0 (shadderAttribute) and is 3 floats
    glVertexAttribPointer(
            sh_in_Position,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            0 );

    // Enable attribute in_Color as being used
    glEnableVertexAttribArray(sh_in_Color);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    // Vertex data to attribute index 0 (shadderAttribute) and is 3 floats
    glVertexAttribPointer(
            sh_in_Color,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            0 );

    // Enable attribute in_UV as being used
    glEnableVertexAttribArray(sh_in_UV);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    // Vertex data to attribute index 0 (shadderAttribute) and is 3 floats
    glVertexAttribPointer(
            sh_in_UV,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            0 );

    // Send M, V, P matrixes to the uniform inputs,
    glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projId, 1, GL_FALSE, &proj[0][0]);

    glUniform1f(timeId, (float)((int)frame_count +1));

    glDrawArrays(GL_TRIANGLE_STRIP, sh_in_Position, N_VERTICES);

    glDisableVertexAttribArray(sh_in_Position);
    glDisableVertexAttribArray(sh_in_Color);
    glDisableVertexAttribArray(sh_in_UV);

    // Update host window
    os.SwapBuffers();
    os.OSMessageLoop();
}

#endif

