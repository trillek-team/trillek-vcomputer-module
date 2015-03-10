/*!
 * \brief       Stuff to create GLFW window and gestionate it
 * \file        os.hpp
 * \copyright   LGPL v3
 *
 * Stuff to create GLFW window and gestionate it
 */

#ifndef __OS_HPP_
#define __OS_HPP_ 1

#include "config_main.hpp"

#ifdef _WIN32
#include <windows.h>
#include "unwindows.hpp"
#endif

#ifdef GLFW3_ENABLE
#include "keyboard_input_system.hpp"

#include <cstdio>
#include <iostream>

namespace OS {
    // Error helper function used by GLFW for error messaging.
    // Currently outputs to std::cout.
    static void ErrorCallback(int error, const char* description) {
        std::cerr << error << " -> " << description << std::endl;
    }

    class OS {
        public:
            OS() : mouseLock(false) {
                this->lastTime = glfwGetTime(); // Better that garbage...
            }

            ~OS() { }

			bool InitializeWindow(const int width, const int height, const std::string title) {
				glfwSetErrorCallback(ErrorCallback);

				// Initialize the library.
				if (glfwInit() != GL_TRUE) {
					std::cerr << "Can¡t start GLFW\n";
					return false;
				}
#ifdef __APPLE__
				glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
				// Create a windowed mode window and its OpenGL context.
				this->window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
				if (!this->window) {
					glfwTerminate();
					std::cerr << "Can't open a window with GLFW\n";
					return false;
				}
				
				// Make the window's context current.
				glfwMakeContextCurrent(this->window);



                this->width = width;
                this->height = height;

#ifdef __APPLE__
                // Force retina displays to create a 1x framebuffer so we don't choke our fillrate.
                id cocoaWindow = glfwGetCocoaWindow(this->window);
                id cocoaGLView = ((id (*)(id, SEL)) objc_msgSend)(cocoaWindow, sel_getUid("contentView"));
                ((void (*)(id, SEL, bool)) objc_msgSend)(cocoaGLView, sel_getUid("setWantsBestResolutionOpenGLSurface:"), false);

#endif
                // setting glewExperimental fixes a glfw context problem
                // (tested on Ubuntu 13.04)
                glewExperimental = GL_TRUE;
                // Init GLEW.
                GLuint error = glewInit();
                if (error != GLEW_OK) {
					std::cerr << "Can't initialize glew\n";
					glfwTerminate();
					return false;
                }

				std::string gl_version, gl_renderer;
				gl_version = (char*)glGetString(GL_VERSION);
				gl_renderer = (char*)glGetString(GL_RENDERER);

				std::cout << "Renderer: " << gl_renderer << "\n";
				std::cout << "OpenGL Version: " << gl_version << "\n";

                // Associate a pointer for this instance with this window.
                glfwSetWindowUserPointer(this->window, this);

                // Set up some callbacks.
                glfwSetWindowSizeCallback(this->window, &OS::windowResized);

                glfwSetKeyCallback(this->window, &keyboardEvent);
                //glfwSetCursorPosCallback(this->window, &OSmouseMoveEvent);
                glfwSetCharCallback(this->window, &characterEvent);
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


            void UpdateWindowSize(const int width, const int height) {
                this->width = width;
                this->height = height;
            }

            /**
             * \brief Callback for when the window is resized.
             *
             * \param[in] GLFWwindow * window
             * \param[in] int width, height The new client width and height of the window.
             * \return void
             */
            static void windowResized(GLFWwindow* window, int width, int height) {
                // Enforces desired size
                // Get the user pointer and cast it.
                OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));
                if (os) {
                    if (width != os->width || height != os->height) {
                        glfwSetWindowSize(window, os->width, os->height);
                    }
                }
            }

            /**
             * \brief Callback for keyboard events.
             *
             * \param[in] GLFWwindow * window
             * \param[in] int key ASCII key number.
             * \param[in] int scancode The converted key value
             * \param[in] int action The event type.
             * \param[in] int mods Modifier keys.
             * \return void
             */
            static void keyboardEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
                // Get the user pointer and cast it.
                OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

                if (os) {
                    os->DispatchKeyboardEvent(key, scancode, action, mods);
                }

            }
            /**
             * \brief Callback for unicode character event.
             *
             * This is different from just a normal keyboard event as it has been translated and modified by
             * the OS and is just like typing into a text editor.
             * \param[in] GLFWwindow * window
             * \param[in] unsigned int uchar The unicode character key code.
             * \return void
             */
            static void characterEvent(GLFWwindow* window, unsigned int uchar) {
                // Get the user pointer and cast it.
                OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

                if (os) {
                    os->DispatchCharacterEvent(uchar);
                }
            }

            /**
             * \brief Callback for window focus change events.
             *
             * \param[in/out] GLFWwindow * window
             * \param[in/out] int focused GL_TRUE if focused, GL_FALSE if unfocused.
             * \return void
             */
            static void windowFocusChange(GLFWwindow* window, int focused) {
                if (focused == GL_FALSE) {
                    // Get the user pointer and cast it.
                    OS* os = static_cast<OS*>(glfwGetWindowUserPointer(window));

                    if (os) {
                        os->KeyboardEventSystem.LostKeyboardFocus();
                    }
                }
            }

            void RegisterKeyboardEventHandler(event::IKeyboardEventHandler* handler) {
                this->KeyboardEventSystem.Register(handler);
            }

            bool HasKeyboardFocusLock() {
                return this->KeyboardEventSystem.HasFocusLock();
            }

            void ToggleMouseLock() {
                this->mouseLock = !this->mouseLock;
                if (this->mouseLock) {
                    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            void SetMousePosition(double x, double y) {
                glfwSetCursorPos(this->window, x, y);
            }

            bool CheckKeyState(event::KEY_STATE state, const int key) {
                if (state == event::KS_DOWN) {
                    if (glfwGetKey(this->window, key) == GLFW_PRESS) {
                        return true;
                    }
                } else {
                    if (glfwGetKey(this->window, key) == GLFW_RELEASE) {
                        return true;
                    }
                }

                return false;
            }

        private:

            /**
             * \brief Dispatches keyboard events from the callback.
             *
             * \param[in] const int key ASCII key number.
             * \param[in] const int scancode The converted key value
             * \param[in] const int action The event type.
             * \param[in] const int mods Modifier keys.
             * \return void
             */
            void DispatchKeyboardEvent(const int key, const int scancode, const int action, const int mods) {
                if (action == GLFW_PRESS) {
                    KeyboardEventSystem.KeyDown(key);
                }else if (action == GLFW_REPEAT) {
                    KeyboardEventSystem.KeyRepeat(key);
                } else if (action == GLFW_RELEASE) {
                    KeyboardEventSystem.KeyUp(key);
                }
            }

            /**
             * \brief Dispatches a character event.
             *
             * \param[in] const unsigned int uchar The unicode character key code.
             * \return void
             */
            void DispatchCharacterEvent(const unsigned int uchar) {
                this->KeyboardEventSystem.CharDown(uchar);
            }

            GLFWwindow* window;
            int width, height; // Current window's client width and height.
            double oldMouseX, oldMouseY;
            double lastTime; // The time at the last call to GetDeltaTime().
            bool mouseLock; // If mouse lock is enabled causing the cursor to snap to mid-window each movement event.

            event::KeyboardInputSystem KeyboardEventSystem;
    };

} // End of namespace OS

#endif

#endif // __OS_HPP_

