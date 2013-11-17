#include <vm.hpp>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp> 

#include <SDL2/SDL.h>

#include <iostream>
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

vm::byte_t vram_t0[0x1190] = {0};

vm::byte_t vram_0[0x1B31] = {0};

vm::byte_t vram_1[0x2431] = {0};

vm::byte_t vram_2[0x2580] = {0};

unsigned sdl_width = 800;
unsigned sdl_height = 600;
int sdl_other_flags = SDL_WINDOW_SHOWN;
SDL_Window* pwin = nullptr;
SDL_Renderer* prend = nullptr;
SDL_GLContext ogl_context;


GLuint screenVTex[3]; // ID of screen Texture in Video mode X
GLuint screenTTex[1]; // ID of screen Texture in Text mode 0

// Handler of shader program
GLuint shaderProgram;
 
// Ptrs to source doe of shaders
GLchar *vertexSource = nullptr;
GLchar *fragmentSource = nullptr;

// Handlers of the shader programs
GLuint vertexShader, fragmentShader;

GLuint modelId, viewId, projId; // Handle for uniform inputs to the shader

const unsigned int sh_in_Position = 0;
const unsigned int sh_in_Color = 1;
const unsigned int sh_in_UV = 3;
 
static const GLfloat N_VERTICES=4;
GLuint vertexbuffer;
static const float vdata[] = {
            3.2,  2.4, 0.0, // Top Right
           -3.2,  2.4, 0.0, // Top Left
            3.2, -2.4, 0.0, // Botton Right
           -3.2, -2.4, 0.0, // Bottom Left
};

GLuint colorbuffer;
static const float color_data[] = {
            1.0,  1.0, 1.0, // Top Right
            1.0,  1.0, 1.0, // Top Left
            1.0,  1.0, 1.0, // Botton Right
            1.0,  1.0, 1.0, // Bottom Left
};

GLuint uvbuffer;
static const float uv_data[] = {
            1.0,  0.0,      // Top Right
            0.0,  0.0,      // Top Left
            1.0,  1.0,      // Botton Right
            0.0,  1.0,      // Bottom Left
};

glm::mat4 proj, view, model; // MVP Matrixes

float yaw, pith, zoom; // Camara orientation

void initSDL();
void initGL();

int main(int argc, char* argv[]) {
    using namespace vm;
    using namespace vm::cda;

    unsigned vmode = 2;
    bool text_mode = false;

    // Fill vram buffers ******************************************************
    // Text Mode 0
    for(auto i=0; i< 40*30; i++) {
      vram_t0[i*2] = i % 255;
      vram_t0[i*2+1] = 0x0F | ((i % 16) << 4); // White ink, rainbow background
    }

    // Video Mode 0 and 1
    for(auto i=0; i< 32*193; i++) {
      if (i % 4 == 0) {
        vram_0[i] = 0;
        vram_1[i] = 0;
      } else {
        vram_0[i] = 0xC3;
        vram_1[i] = 0xC3;
      } 
    }
    for (auto i=0; i < 32; i++) {
        vram_0[32*30 +i] = 0xFF; // Fills a line in y = 30
        vram_0[32*191 +i] = 0xFF; // Fills a line in y = 191

        vram_1[32*30 +i] = 0xFF; // Fills a line in y = 30
        vram_1[32*191 +i] = 0xFF; // Fills a line in y = 191
    }

    // Border color
    vram_0[0x1B00] = 12; // Pure Red
    vram_1[0x2400] = 6; // Brown
    
    // Attrbutes of Video mode 0
    for(auto i=0; i< 32*4; i++) {
      vram_0[0x1800 + i] = i % 16;
    }
    for(auto i=32*4; i< 32*8; i++) {
      vram_0[0x1800 + i] = ((i % 16) << 4) | 7;
    }
    for(auto i=32*8; i< 32*24; i++) {
      vram_0[0x1800 + i] = i% 256;
    }
    
    // Attrbutes of Video mode 1
    for(auto i=0; i< 64*4; i++) {
      vram_1[0x1800 + i] = i % 16;
    }
    for(auto i=64*4; i< 64*8; i++) {
      vram_1[0x1800 + i] = ((i % 16) << 4) | 7;
    }
    for(auto i=64*8; i< 64*48; i++) {
      vram_1[0x1800 + i] = i% 256;
    }


    // Video mode 2
    for(auto i=0; i< 40*240; i++) {
      if (i % 4 == 0) {
        vram_2[i] = 0;
      } else {
        vram_2[i] = 0xC3;
      } 
    }

    initSDL();

    initGL();

    std::printf("Initiated OpenGL\n");
    std::printf("Initial VideoMode %u\n", vmode);

    bool loop = true;
    while ( loop) {
            
        SDL_Event e;
        while (SDL_PollEvent(&e)){
            //If user closes he window
            if (e.type == SDL_QUIT)
                loop = false;
            else if (e.type == SDL_KEYDOWN) {
              if (e.key.keysym.sym == SDLK_n) { // Next videomode
                if (text_mode) {
                  text_mode = false;
                  vmode = 0;
                    std::printf("Set Videomode %u\n", vmode);
                } else {
                  vmode++;
                  if (vmode >= 3) {
                    text_mode = true;
                    vmode = 0;
                    std::printf("Set Textmode %u\n", vmode);

                  } else
                    std::printf("Set Videomode %u\n", vmode);
                }

              } else if (e.key.keysym.sym == SDLK_a) { // Turn to left
                yaw -= 0.1; 
              } else if (e.key.keysym.sym == SDLK_d) { // Turn to right
                yaw += 0.1; 
              } else if (e.key.keysym.sym == SDLK_w) { // Turn to up
                pith += 0.1; 
              } else if (e.key.keysym.sym == SDLK_s) { // Turn to down
                pith -= 0.1; 

              } else if (e.key.keysym.sym == SDLK_r) { // Zoom In
                zoom = (zoom > 2.0)? zoom - 0.1 : zoom; 
              } else if (e.key.keysym.sym == SDLK_f) { // Zoom out
                zoom += 0.1; 
              }
            }
        }

        // Clear The Screen And The Depth Buffer
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
        if (text_mode) {
          glBindTexture(GL_TEXTURE_2D, screenTTex[0]);
        } else {
          glBindTexture(GL_TEXTURE_2D, screenVTex[vmode]);
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

        glDrawArrays(GL_TRIANGLE_STRIP, sh_in_Position, N_VERTICES);

        glDisableVertexAttribArray(sh_in_Position);
        glDisableVertexAttribArray(sh_in_Color);
        glDisableVertexAttribArray(sh_in_UV);

        // Update host window
        SDL_RenderPresent(prend);
        SDL_GL_SwapWindow(pwin);
    }


    SDL_GL_DeleteContext(ogl_context);
    SDL_DestroyWindow(pwin);
    SDL_Quit();

    return 0;
}



void initSDL() {
    // Init SDL2 / OpenGL stuff
    if (SDL_Init(SDL_INIT_VIDEO) == -1){
        std::cout << SDL_GetError() << std::endl;
        exit(-1);
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    pwin = SDL_CreateWindow("RC3200 VM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
              sdl_width, sdl_height, SDL_WINDOW_OPENGL | sdl_other_flags);

    ogl_context = SDL_GL_CreateContext(pwin);

    GLenum status = glewInit();
    if (status != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
        exit(1);
    }
    
    // sync buffer swap with monitor's vertical refresh rate
    SDL_GL_SetSwapInterval(1);
}

void initGL() {
    // Init OpenGL ************************************************************
    
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
   
    // Initialize Texture *****************************************************
    vm::dword_t tdata[320*240] = {0};

    glGenTextures(3, screenVTex);
    glGenTextures(1, screenTTex);
    
    // Textmode 0
    vm::cda::RGBATexture(vram_t0, 0, false, false, true, tdata);
    glBindTexture(GL_TEXTURE_2D, screenTTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // Videomode 0
    vm::cda::RGBATexture(vram_0, 0, false, false, false, tdata);
    glBindTexture(GL_TEXTURE_2D, screenVTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Videomode 1
    vm::cda::RGBATexture(vram_1, 1, false, false, false, tdata);
    glBindTexture(GL_TEXTURE_2D, screenVTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Videomode 2
    vm::cda::RGBATexture(vram_2, 2, false, false, false, tdata);
    glBindTexture(GL_TEXTURE_2D, screenVTex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



    // Loading shaders ********************************************************
    auto f_vs = std::fopen("./assets/shaders/mvp_template.vert", "r");
    if (f_vs != nullptr) {
      fseek(f_vs, 0L, SEEK_END);
      size_t bufsize = ftell(f_vs);
      vertexSource = new GLchar[bufsize + 1]();

      fseek(f_vs, 0L, SEEK_SET);
      fread(vertexSource, sizeof(GLchar), bufsize, f_vs);
      
      fclose(f_vs);
      vertexSource[bufsize] = 0; // Enforce null char
    }

    auto f_fs = std::fopen("./assets/shaders/basic_texture.frag", "r");
    if (f_fs != nullptr) {
      fseek(f_fs, 0L, SEEK_END);
      size_t bufsize = ftell(f_fs);
      fragmentSource = new GLchar[bufsize +1 ]();

      fseek(f_fs, 0L, SEEK_SET);
      fread(fragmentSource, sizeof(GLchar), bufsize, f_fs);
      
      fclose(f_fs);
      fragmentSource[bufsize] = 0; // Enforce null char
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

    if (vertexSource != nullptr)
      delete[] vertexSource;

    if (fragmentSource != nullptr)
      delete[] fragmentSource;
   
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Vertex Shader compiling error messages
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength); 
    glGetShaderInfoLog(vertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    std::printf("%s\n", &VertexShaderErrorMessage[0]);
    
    // Fragment Shader compiling error messages
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength); 
    glGetShaderInfoLog(fragmentShader, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    std::printf("%s\n", &FragmentShaderErrorMessage[0]);
    

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

     
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Projection matrix : 45° Field of View
    proj = glm::perspective(45.0f,  (GLfloat)(sdl_width) / sdl_height, 0.1f, 100.0f);
    
    // Camera matrix
    pith = yaw = 0;
    zoom = 10.f;
    view = glm::lookAt(
        glm::vec3(zoom*sin(yaw), zoom*sin(pith), zoom*cos(yaw)), // Were is the camera
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
      );

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
}


