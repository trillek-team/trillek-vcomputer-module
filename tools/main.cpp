/**
 * Trillek Virtual Computer - main.cpp
 * Test/Toy executable that uses the Virtual Computer lib to run a emulation
 */
#include "OS.hpp"

#include "VC.hpp"
#include "DisTR3200.hpp"

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

#ifdef GLFW3_ENABLE

unsigned winWidth;
unsigned winHeight;

//bool capture_keyboard = false;

GLuint screenTex; // ID of screen Texture
GLuint tex_pbo;   // ID of the screen texture PBO

// Handler of shader program
GLuint shaderProgram;

// Ptrs to source doe of shaders
GLchar *vertexSource = nullptr;
GLchar *fragmentSource = nullptr;

// Handlers of the shader programs
GLuint vertexShader, fragmentShader;

// Handles for uniform inputs to the shader
GLuint modelId;
GLuint viewId;
GLuint projId;
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
float zoom = 6.0;

void initGL(OS& os);

//void updatePBO (vm::cda::CDA*);

#endif

//vm::cda::CDA* cda_ptr = nullptr;

void print_regs(const vm::cpu::TR3200State& state);
void print_pc(const vm::cpu::TR3200State& state, const vm::VComputer& vc);
//void print_stack(const vm::cpu::TR3200& cpu, const vm::ram::Mem& ram);

int main(int argc, char* argv[]) {
  using namespace vm;
  using namespace vm::cpu;

  byte_t* rom = nullptr;
  size_t rom_size = 0;

  if (argc < 2) {
    std::printf("Usage: %s binary_file\n", argv[0]);
    return -1;

  } else {
    rom = new byte_t[32*1024];

    std::printf("Opening file %s\n", argv[1]);
    
    int size = vm::aux::LoadROM(argv[1], rom);
    if (size < 0) {
      std::fprintf(stderr, "An error occurred while reading file %s\n", argv[1]);
      return -1;
    }
    
    std::printf("Read %d bytes and stored in ROM\n", size);
    rom_size = size;
  }

  // Create the Virtual Machine
  VComputer vc;
	std::unique_ptr<vm::cpu::TR3200> cpu(new TR3200());
	cpu->Clock();
	vc.SetCPU(std::move(cpu));
	
  vc.SetROM(rom, rom_size);

  // Add devices to tue Virtual Machine
  auto gcard = std::make_shared<vm::dev::tda::TDADev>();
  vm::dev::tda::TDAScreen gcard_screen = {0};
  //cda::CDA gcard(0, 10);
  //keyboard::GKeyboard keyb;
  vc.AddDevice(0, gcard);
  //vm.AddDevice(5, keyb);

  vc.Reset();

  std::cout << "Run program (r) or Step Mode (s) ?\n";
  char mode;
  std::cin >> mode;
  std::getchar();


  bool debug = false;
  if (mode == 's' || mode == 'S') {
    debug = true;
  }



#ifdef GLFW3_ENABLE
  OS glfwos;
  if (!glfwos.InitializeWindow(1024, 768, "Trillek Virtual Computer demo emulator")) {
    std::clog << "Failed creating the window or context.";
    return -1;
  }
  
  initGL(glfwos);
  std::printf("Initiated OpenGL\n");
  float frame_count = 0; // Count frames
	
#endif

  std::cout << "Running!\n";
  unsigned ticks = 16050;
  unsigned long ticks_count = 0;

  using namespace std::chrono;
  auto clock = high_resolution_clock::now();
  double delta; // Time delta in seconds
  double t_acu = 0; // Acumulated time

  int c = ' ';
  bool loop = true;
	vm::cpu::TR3200State cpu_state;

  while ( loop) {
    // Calcs delta time 

    auto oldClock = clock;
    clock = high_resolution_clock::now();
    delta = duration_cast<milliseconds>(clock - oldClock).count();

    t_acu += delta/1000.0;


#ifdef GLFW3_ENABLE
		if (glfwos.Closing()) {
			loop = false;
			continue;
		}
		/*
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
          auto k = vm::aux::SDL2KeyToTR3200(e.key.keysym.scancode);
          keyb.PushKeyEvent( true, k);
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
          auto k = vm::aux::SDL2KeyToTR3200(e.key.keysym.scancode);
          keyb.PushKeyEvent( false, k);
        }

      }
    }
		*/
#endif

    if (debug) {
			vc.GetState((void*) &cpu_state, sizeof(cpu_state));
      print_pc(cpu_state, vc);
      //if (vm.CPU().Skiping())
      //  std::printf("Skiping!\n");
      //if (vm.CPU().Sleeping())
      //  std::printf("ZZZZzzzz...\n");
    }

    if (!debug) {
      ticks_count += ticks;
      vc.Tick(ticks, delta / 1000.0 );
      //ticks = (vc.Clock() * delta * 0.000001) + 0.5f; // Rounding bug in VS
      //if (ticks <= 3)
      //  ticks = 3;

    } else {
      ticks = vc.Step(delta / 1000.0 ); 
      //ticks = 1; vc.Tick(1, delta * 0.001f );
    }


    // Speed info
    if (!debug && ticks_count > 200000) {
      std::printf("Running %u cycles in %f ms ", ticks, delta);
      double ttick = delta / ticks;
      double tclk = 1000.0 / 1000000.0; // Base clock 1Mhz
      std::printf("Ttick %f ms ", ttick);
      std::printf("Tclk %f ms ", tclk);
      std::printf("Speed of %f %% \n", 100.0f * (tclk / ttick) );
      ticks_count -= 200000;
    }


    if (debug) {
      vc.GetState((void*) &cpu_state, sizeof(cpu_state));
      std::printf("Takes %u cycles\n", cpu_state.wait_cycles);
      print_regs(cpu_state);
      //print_stack(vm.CPU(), vm.RAM());
      c = std::getchar();
      if (c == 'q' || c == 'Q')
        loop = false;

    }

#ifdef GLFW3_ENABLE
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

    if (t_acu >= 0.04) { // Updates screen texture at a rate of ~25 Hz
      t_acu -= 0.04;
      // Dump a copy of the CDA card state
      gcard->DumpScreen (gcard_screen);
      gcard->DoVSync();

      // Stream the texture *************************************************
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex_pbo);

      // Copy the PBO to the texture
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, 0);

      // Updates the PBO with the new texture
      auto tdata = (dword_t*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
      if (tdata != nullptr) {
        //std::fill_n(tdata, 320*240, 0xFF800000);
        TDAtoRGBATexture(gcard_screen, tdata); // Write the texture to the PBO buffer

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
		glfwos.SwapBuffers();
		glfwos.OSMessageLoop();
#endif
  }

#ifdef GLFW3_ENABLE
	glfwos.Terminate();
#endif

  return 0;
}

// Alias to special registers
#define REG_Y			(11)
#define BP				(12)
#define SP				(13)
#define REG_IA		(14)
#define REG_FLAGS (15)

// Operation in Flags bits
#define GET_CF(x)          ((x) & 0x1)
#define SET_ON_CF(x)       (x |= 0x1)
#define SET_OFF_CF(x)      (x &= 0xFFFFFFFE)

#define GET_OF(x)          (((x) & 0x2) >> 1)
#define SET_ON_OF(x)       (x |= 0x2)
#define SET_OFF_OF(x)      (x &= 0xFFFFFFFD)

#define GET_DE(x)          (((x) & 0x4) >> 2)
#define SET_ON_DE(x)       (x |= 0x4)
#define SET_OFF_DE(x)      (x &= 0xFFFFFFFB)

#define GET_IF(x)          (((x) & 0x8) >> 3)
#define SET_ON_IF(x)       (x |= 0x8)
#define SET_OFF_IF(x)      (x &= 0xFFFFFFF7)

// Enable bits that change what does the CPU
#define GET_EI(x)          (((x) & 0x100) >> 8)
#define SET_ON_EI(x)       (x |= 0x100)
#define SET_OFF_EI(x)      (x &= 0xFFFFFEFF)

#define GET_ESS(x)         (((x) & 0x200) >> 9)
#define SET_ON_ESS(x)      (x |= 0x200)
#define SET_OFF_ESS(x)     (x &= 0xFFFFFDFF)

// Internal alias to Y Flags and IA registers
#define RY      r[REG_Y]
#define IA      r[REG_IA]
#define FLAGS   r[REG_FLAGS]

void print_regs(const vm::cpu::TR3200State& state) {
  // Print registers

  for (int i=0; i < 11; i++) {
    std::printf("%%r%2d= 0x%08X ", i, state.r[i]);
    if (i == 3 || i == 7 || i == 11 || i == 15 || i == 19 || i == 23 || i == 27 || i == 31)
      std::printf("\n");
  }
  std::printf("%%y= 0x%08X\n", state.r[REG_Y]);

  std::printf("%%ia= 0x%08X ", state.r[REG_IA]);
  auto flags = state.r[REG_FLAGS];
  std::printf("%%flags= 0x%08X ", flags);
  std::printf("%%bp= 0x%08X ",  state.r[BP]);
  std::printf("%%sp= 0x%08X\n", state.r[SP]);

  std::printf("%%pc= 0x%08X \n", state.pc);
  std::printf("ESS: %d EI: %d \t IF: %d DE %d OF: %d CF: %d\n",
      GET_ESS(flags), GET_EI(flags), GET_IF(flags) , GET_DE(flags) , GET_OF(flags) , GET_CF(flags));
  std::printf("\n");
  
}

void print_pc(const vm::cpu::TR3200State& state, const vm::VComputer& vc) {
  vm::dword_t val = vc.ReadDW(state.pc);

  std::printf("\tPC : 0x%08X > 0x%08X ", state.pc, val); 
  std::cout << vm::cpu::Disassembly(vc,  state.pc) << std::endl;  
}

/*
void print_stack(const vm::cpu::TR3200& cpu, const vm::ram::Mem& ram) {
  std::printf("STACK:\n");

  for (size_t i =0; i <5*4; i +=4) {
    auto val = ram.RD(cpu.Reg(SP)+ i);

    std::printf("0x%08X\n", val);
    if (((size_t)(cpu.Reg(SP)) + i) >= 0xFFFFFFFF)
      break;
  }
}
*/

#ifdef GLFW3_ENABLE


// Init OpenGL ************************************************************
void initGL(OS& os) {
  int OpenGLVersion[2];

  // Use the GL3 way to get the version number
  glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
  glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
  std::cout << "OpenGL " << OpenGLVersion[0] << "." << OpenGLVersion[1] << "\n";

  // Sanity check to make sure we are at least in a good major version number.
  assert((OpenGLVersion[0] > 1) && (OpenGLVersion[0] < 5));

  winWidth = os.GetWindowWidth(); winHeight = os.GetWindowHeight();

  // Determine the aspect ratio and sanity check it to a safe ratio
  GLfloat aspectRatio = static_cast<float>(winWidth) / static_cast<float>(winHeight);
  if (aspectRatio < 1.0f) {
	  aspectRatio = 4.0f / 3.0f;
  }
  // Projection matrix : 45° Field of View
  proj = glm::perspective(
      45.0f,			// FOV
      aspectRatio, 
      0.1f,				// Near cliping plane
      10000.0f);	// Far cliping plane

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


  // Camera matrix
  view = glm::lookAt(
      glm::vec3(3,3,7), // Camera is at (3,3,7), in World Space
      glm::vec3(0,0,0), // and looks at the origin
      glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
      );

}

/*
void updatePBO (vm::cda::CDA* cda) {
  using namespace vm;

}
*/

#endif


