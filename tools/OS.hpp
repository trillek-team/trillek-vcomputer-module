#pragma once
/**
 * Stuff to create GLFW window and gestionate it
 */

#ifndef __OS_HPP_
#define __OS_HPP_ 1

#include "config_main.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef GLFW3_ENABLE

	class OS {
	private:
		GLFWwindow* window;
		int width, height; // Current window's client width and height.
		double oldMouseX, oldMouseY;
		double lastTime; // The time at the last call to GetDeltaTime().
		bool mouseLock; // If mouse lock is enabled causing the cursor to snap to mid-window each movement event.

	public:
		OS() : mouseLock(false) {
      this->lastTime = glfwGetTime(); // Better that garbage...
    }
		
		~OS() { }
		
		bool InitializeWindow(const int width, const int height, const std::string title, const unsigned int glMajor = 3, const unsigned int glMinor = 2) {
			// Initialize the library.
			if (glfwInit() != GL_TRUE) {
				return false;
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);

#ifdef __APPLE__
			// Must use the Core Profile on OS X to get GL 3.2.
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

			// Create a windowed mode window and its OpenGL context.
			this->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
				
			if (!this->window) {
				glfwTerminate();
				return false;
			}

			this->width = width;
			this->height = height;

#ifdef __APPLE__
			// Force retina displays to create a 1x framebuffer so we don't choke our fillrate.
			id cocoaWindow = glfwGetCocoaWindow(this->window);
			id cocoaGLView = ((id (*)(id, SEL)) objc_msgSend)(cocoaWindow, sel_getUid("contentView"));
			((void (*)(id, SEL, bool)) objc_msgSend)(cocoaGLView, sel_getUid("setWantsBestResolutionOpenGLSurface:"), false);
#endif

			// Make the window's context current.
			glfwMakeContextCurrent(this->window);
			
#ifndef __APPLE__
			// setting glewExperimental fixes a glfw context problem
			// (tested on Ubuntu 13.04)
			glewExperimental = GL_TRUE;
			
			// Init GLEW.
			GLuint error = glewInit();
			if (error != GLEW_OK) {
				return false;
			}
#endif

			// Associate a pointer for this instance with this window.
			glfwSetWindowUserPointer(this->window, this);

			// Set up some callbacks.
			//glfwSetWindowSizeCallback(this->window, &OS::windowResized);
			//glfwSetKeyCallback(this->window, &keyboardEvent);
			//glfwSetCursorPosCallback(this->window, &OSmouseMoveEvent);
			//glfwSetCharCallback(this->window, &characterEvent);
			//glfwSetMouseButtonCallback(this->window, &mouseButtonEvent);
			//glfwSetWindowFocusCallback(this->window, &windowFocusChange);

			glfwGetCursorPos(this->window, &this->oldMouseX, &this->oldMouseY);

			return true;
		}

		void Terminate() {
			glfwTerminate();
		}

		bool Closing() {
			return glfwWindowShouldClose(this->window) > 0;
		}

		void SwapBuffers() {
			glfwSwapBuffers(this->window);
		}

		void OSMessageLoop() {
			glfwPollEvents();
		}

		int GetWindowWidth() {
			return this->width;
		}

		int GetWindowHeight() {
			return this->height;
		}

		GLFWwindow* GetWindow() {
			return this->window;
		}

    /**
     * Return delta time in SECONDS
     */
		double GetDeltaTime() {
			double time = glfwGetTime();
			double delta = time - this->lastTime;
			this->lastTime = time;
			return delta;
		}

		void windowResized(GLFWwindow* window, int width, int height) {
			// Get the user pointer and cast it.
			OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

			if (os) {
				os->UpdateWindowSize(width, height);
			}
		}

		void UpdateWindowSize(const int width, const int height) {
			this->width = width;
			this->height = height;
		}


	};
#endif

#endif // __OS_HPP_

