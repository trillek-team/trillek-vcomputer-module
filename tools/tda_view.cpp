/**
 * Trillek Virtual Computer - tda_view.cpp
 * Tool that visualizes a image of a TDA screen using a stored TDA state 
 */
#include "OS.hpp"
#include "devices/TDA.hpp"

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

unsigned winWidth;
unsigned winHeight;

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
float zoom = 10.0;

void initGL(OS& os);

int main (int argc, char* argv[]) {
  using namespace vm;
  using namespace vm::dev::tda;

  // TODO load screen from a file

	OS glfwos;
	if (!glfwos.InitializeWindow(1024, 768, "Trillek Virtual Computer demo emulator")) {
		std::clog << "Failed creating the window or context.";
		return -1;
	}
  
  initGL(glfwos);
  std::printf("Initiated OpenGL\n");
  float frame_count = 0; // Count frames
  double deltat = glfwos.GetDeltaTime();
  double t_acu = 0; // Time acumulator

  // TODO Here insert a callback fro events	

  // Test screen
  TDAState screen = {0}; // Clear it
  screen.buffer_ptr = 1;
  screen.font_ptr = 0;
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

  for (unsigned i= 40; i < WIDTH_CHARS*HEIGHT_CHARS; i++ ) {
    byte_t fg = i % 16;
    byte_t bg = (15 - i) % 16;
    screen.txt_buffer[i] = (bg << 12) | (fg << 8) | ((i-40) % 256); 
  }

  bool loop = true;
  while ( loop) {
		if (glfwos.Closing()) {
			loop = false;
			continue;
		}

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

      // Stream the texture *************************************************
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex_pbo);

      // Copy the PBO to the texture
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 240, GL_RGBA, GL_UNSIGNED_BYTE, 0);

      // Updates the PBO with the new texture
      auto tdata = (dword_t*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
      if (tdata != nullptr) {
        TDAtoRGBATexture(screen, tdata); // Write the texture to the PBO buffer

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

    deltat = glfwos.GetDeltaTime(); // Gets new Delta time
    t_acu += deltat;
  }

  return 0;
}

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



