#include "config_main.hpp"

#include <vm.hpp>

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

#ifdef SDL2_ENABLE

unsigned sdl_width = 1024;
unsigned sdl_height = 768;
int sdl_other_flags = SDL_WINDOW_SHOWN;
SDL_Window* pwin = nullptr;
SDL_Renderer* prend = nullptr;
SDL_GLContext ogl_context;

bool capture_keyboard = false;

GLuint screenTex; // ID of screen Texture
GLuint tex_pbo;     // ID of the screen texture PBO

// Handler of shader program
GLuint shaderProgram;
 
// Ptrs to source doe of shaders
GLchar *vertexSource = nullptr;
GLchar *fragmentSource = nullptr;

// Handlers of the shader programs
GLuint vertexShader, fragmentShader;

GLuint modelId, viewId, projId; // Handle for uniform inputs to the shader
GLuint timeId;

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
float yaw  = 0.0;
float pith = 0.0;
float zoom = 10.0;

void initSDL();
void initGL();

void updatePBO (vm::cda::CDA*);

#endif

vm::cda::CDA* cda_ptr = nullptr;

void print_regs(const vm::cpu::CpuState& state);
void print_pc(const vm::cpu::CpuState& state, const vm::ram::Mem& ram);
void print_stack(const vm::cpu::CpuState& state, const vm::ram::Mem& ram);

int main(int argc, char* argv[]) {
    using namespace vm;
    using namespace vm::cpu;

    byte_t* rom = NULL;
    size_t rom_size = 0;

    if (argc < 2) {
        std::printf("Usage: %s binary_file\n", argv[0]);
        return -1;

    } else {
        rom = new byte_t[64*1024];

        std::printf("Opening file %s\n", argv[1]);
        std::fstream f(argv[1], std::ios::in | std::ios::binary);
        unsigned count = 0;
#if (BYTE_ORDER != LITTLE_ENDIAN)
        while (f.good() && count < 64*1024) {
                f.read(reinterpret_cast<char*>(rom + count++), 1); // Read byte at byte, so the file must be in Little Endian
        }
#else
        size_t size;
        auto begin = f.tellg();
        f.seekg (0, std::ios::end);
        auto end = f.tellg();
        f.seekg (0, std::ios::beg);

        size = end - begin;
        size = size > (64*1024) ? (64*1024) : size;
        
        f.read(reinterpret_cast<char*>(rom), size);
        count = size;
#endif
        std::printf("Read %u bytes and stored in ROM\n", count);
        rom_size = count;
    }

    // Create the Virtual Machine
    VirtualComputer vm;
    vm.WriteROM(rom, rom_size);
    delete[] rom;

    // Add devices to tue Virtual Machine
    cda::CDA gcard(0, 10);
    keyboard::GKeyboard keyb;
    vm.AddDevice(0, gcard);
    vm.AddDevice(5, keyb);

    vm.Reset();
    
    std::printf("Size of CPU state : %zu bytes \n", sizeof(vm.CPUState()) );
    
std::cout << "Run program (r) or Step Mode (s) ?\n";
    char mode;
    std::cin >> mode;
    std::getchar();


    bool debug = false;
    if (mode == 's' || mode == 'S') {
        debug = true;
    }
 


#ifdef SDL2_ENABLE
    initSDL();

    initGL();
    std::printf("Initiated OpenGL\n");
    float frame_count = 0; // Count frames
#endif

    std::cout << "Running!\n";
    unsigned ticks = 1675;
    unsigned long ticks_count = 0;
    
    using namespace std::chrono;
    auto clock = high_resolution_clock::now();
    double delta, updt_scr_acu; // Time Deltas
    updt_scr_acu = 0;

    int c = ' ';
    bool loop = true;
    while ( loop) {
      // Calcs delta time 

      auto oldClock = clock;
      clock = high_resolution_clock::now();
      delta = duration_cast<microseconds>(clock - oldClock).count();

      updt_scr_acu += delta;


#ifdef SDL2_ENABLE
      SDL_Event e;
      while (SDL_PollEvent(&e)){
        //If user closes he window
        if (e.type == SDL_QUIT) {
          loop = false;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {

        } else if (e.type == SDL_MOUSEWHEEL) {
          zoom += e.wheel.y / 10.0f;
          if (zoom < 2.0f)
            zoom = 2.0f;
        } else if (e.type == SDL_KEYDOWN) {
          if (e.key.keysym.sym == SDLK_F3 ) { // F3 togles capture keyboard
            capture_keyboard = ! capture_keyboard;

          } else if (capture_keyboard && e.key.repeat == 0) {
            auto k = e.key.keysym.sym;
            //std::printf("\tkey: '%c' %u\n", k, k);
            // Shit SDL! TODO Do a function that maps key to ascii
            if (k >= 'a' && k <= 'z') {
              if (e.key.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
                k = k -'a' + 'A';

              keyb.PushKeyEvent( true, k);
            } else if (k >= '0' && k <= '9') {
              if (e.key.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
                k = k -'0' + '!';
              
              keyb.PushKeyEvent( true, k);
            } else if (k == ' ' || k == 8 || k == 13) {
              keyb.PushKeyEvent( true, k);
            }

          } else {
            if (e.key.keysym.sym == SDLK_q ) {
              loop = false;
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

        } else if (e.type == SDL_KEYUP) {
          if (capture_keyboard) {
            auto k = e.key.keysym.sym;
            if (k >= 'a' && k <= 'z') {
              if (e.key.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
                k = k -'a' + 'A';
              
              keyb.PushKeyEvent( false, k);
            } else if (k >= '0' && k <= '9') {
              if (e.key.keysym.mod & (KMOD_SHIFT | KMOD_CAPS))
                k = k -'0' + '!';
              
              keyb.PushKeyEvent( false, k);
            } else if (k == ' ' || k == 8 || k == 13) {
              keyb.PushKeyEvent( false, k);
            }

          }

        }
      }
#endif

        if (debug) {
          print_pc(vm.CPUState(), vm.RAM());
          if (vm.CPUState().skiping)
            std::printf("Skiping!\n");
          if (vm.CPUState().sleeping)
            std::printf("ZZZZzzzz...\n");
        }

        if (!debug) {
          ticks_count += ticks;
          vm.Tick(ticks, delta * 0.001f );
          //ticks = 26700000.0f / delta + 0.5f; // Rounding bug correction
        } else
          ticks = vm.Step(delta * 0.001f); 


        // Speed info
        if (!debug && ticks_count > 100000) {
            std::cout << "Running " << ticks << " cycles in " << delta << " nS"
                                << " Speed of " 
                                << 100.0f * (((ticks * 1000000.0) / vm.Clock()) / delta)
                                << "% \n";
            std::cout << std::endl;
            ticks_count -= 100000;
        }


        if (debug) {
            std::printf("Takes %u cycles\n", vm.CPUState().wait_cycles);
            print_regs(vm.CPUState());
            print_stack(vm.CPUState(), vm.RAM());
            c = std::getchar();
            if (c == 'q' || c == 'Q')
                loop = false;

        }

#ifdef SDL2_ENABLE
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

        if (updt_scr_acu >= 50000) { // Updates screen texture at a rate of 20 Hz
          updt_scr_acu -= 50000;
          // Stream the texture *************************************************
          glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex_pbo);

          // Copy the PBO to the texture
          glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, 0);

          // Updates the PBO with the new texture
          auto tdata = (dword_t*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
          if (tdata != nullptr) {
            gcard.ToRGBATexture(tdata);
            
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
          }

          glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // Release the PBO
          gcard.VSync();
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
        SDL_RenderPresent(prend);
        SDL_GL_SwapWindow(pwin);
#endif
    }

#ifdef SDL2_ENABLE

    SDL_GL_DeleteContext(ogl_context);
    SDL_DestroyWindow(pwin);
    SDL_Quit();
#endif

    return 0;
}


void print_regs(const vm::cpu::CpuState& state) {
    // Print registers
    for (int i=0; i < 27; i++) {
        std::printf("%%r%2d= 0x%08X ", i, state.r[i]);
        if (i == 3 || i == 7 || i == 11 || i == 15 || i == 19 || i == 23 || i == 27 || i == 31)
        std::printf("\n");
    }
    std::printf("%%y= 0x%08X\n", RY);
    
    std::printf("%%ia= 0x%08X ", IA);
    std::printf("%%flags= 0x%08X ", FLAGS);
    std::printf("%%bp= 0x%08X ", state.r[BP]);
    std::printf("%%sp= 0x%08X\n", state.r[SP]);

    std::printf("%%pc= 0x%08X \n", state.pc);
/*
    std::printf("EDE: %d EOE: %d ESS: %d EI: %d \t IF: %d DE %d OF: %d CF: %d\n",
                    GET_EDE(FLAGS), GET_EOE(FLAGS), GET_ESS(FLAGS), GET_EI(FLAGS),
                    GET_IF(FLAGS) , GET_DE(FLAGS) , GET_OF(FLAGS) , GET_CF(FLAGS));
*/
    std::printf("ESS: %d EI: %d \t IF: %d DE %d OF: %d CF: %d\n",
                    GET_ESS(FLAGS), GET_EI(FLAGS), GET_IF(FLAGS) , GET_DE(FLAGS) , GET_OF(FLAGS) , GET_CF(FLAGS));
    std::printf("\n");

}

void print_pc(const vm::cpu::CpuState& state, const vm::ram::Mem&  ram) {
    vm::dword_t val = ram.RD(state.pc);
    
    std::printf("\tPC : 0x%08X > 0x%08X ", state.pc, val); 
    std::cout << vm::cpu::Disassembly(ram,  state.pc) << std::endl;  
}

void print_stack(const vm::cpu::CpuState& state, const vm::ram::Mem& ram) {
    std::printf("STACK:\n");

    for (size_t i =0; i <5*4; i +=4) {
            auto val = ram.RD(state.r[SP]+ i);

            std::printf("0x%08X\n", val);
            if (((size_t)(state.r[SP]) + i) >= 0xFFFFFFFF)
                    break;
    }
}


#ifdef SDL2_ENABLE

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
    SDL_SetRelativeMouseMode(SDL_TRUE);
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
    auto f_vs = std::fopen("./assets/shaders/mvp_template.vert", "r");
    if (f_vs != nullptr) {
      fseek(f_vs, 0L, SEEK_END);
      size_t bufsize = ftell(f_vs);
      vertexSource = new GLchar[bufsize + 1]();

      fseek(f_vs, 0L, SEEK_SET);
      auto t = fread(vertexSource, sizeof(GLchar), bufsize, f_vs);
      if (t <= 0)
        std::cerr << "Error reading Vertex Shader\n";
      
      fclose(f_vs);
      vertexSource[bufsize] = 0; // Enforce null char
    }

    //auto f_fs = std::fopen("./assets/shaders/basic_texture.frag", "r");
    auto f_fs = std::fopen("./assets/shaders/retro_texture.frag", "r");
    if (f_fs != nullptr) {
      fseek(f_fs, 0L, SEEK_END);
      size_t bufsize = ftell(f_fs);
      fragmentSource = new GLchar[bufsize +1 ]();

      fseek(f_fs, 0L, SEEK_SET);
      auto t = fread(fragmentSource, sizeof(GLchar), bufsize, f_fs);
      if (t <= 0)
        std::cerr << "Error reading Fragment Shader\n";
      
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
    
    timeId = glGetUniformLocation(shaderProgram, "time");

     
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Projection matrix : 45° Field of View
    proj = glm::perspective(45.0f,  (GLfloat)(sdl_width) / sdl_height, 0.1f, 100.0f);
    
    // Camera matrix
    view = glm::lookAt(
        glm::vec3(3,3,7), // Camera is at (3,3,7), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
      );

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
}


void updatePBO (vm::cda::CDA* cda) {
  using namespace vm;

}


#endif

