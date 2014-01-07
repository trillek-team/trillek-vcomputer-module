/**
 * Auxiliar functions
 * \file aux.cpp
 */

#include "Aux.hpp"
#include "VSFix.hpp"
#include "GKeyboard.hpp"


namespace vm {
  namespace aux {

  byte_t GLFWKeyToTR3200 (int key) {
    switch (key) {
      case -1:
        return vm::keyboard::KEY_UNKNOW;

      case 256:
        return vm::keyboard::KEY_ESC;
      case 257:
        return vm::keyboard::KEY_RETURN;
      case 258:
        return vm::keyboard::KEY_TAB;
      case 259:
        return vm::keyboard::KEY_BACKSPACE;
      case 260:
        return vm::keyboard::KEY_INSERT;
      case 261:
        return vm::keyboard::KEY_DELETE;

      case 262:
        return vm::keyboard::KEY_ARROW_RIGHT;
      case 263:
        return vm::keyboard::KEY_ARROW_LEFT;
      case 264:
        return vm::keyboard::KEY_ARROW_DOWN;
      case 265:
        return vm::keyboard::KEY_ARROW_UP;

      case 340:
      case 344:
        return vm::keyboard::KEY_SHIFT;
      case 341:
      case 345:
        return vm::keyboard::KEY_CONTROL;
      case 346:
        return vm::keyboard::KEY_ALT_GR;

      default:
        if (key <= 255) // Maps 1:1 GLFW keys
          return key;
        return vm::keyboard::KEY_UNKNOW;

    }
    return vm::keyboard::KEY_UNKNOW;
  }

  byte_t SDL2KeyToTR3200 (int key) {
    switch (key) {
      case 0:
        return vm::keyboard::KEY_UNKNOW;

      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
        return (key - 4) + vm::keyboard::KEY_A;

      case 30:
      case 31:
      case 32:
      case 33:
      case 34:
      case 35:
      case 36:
      case 37:
      case 38:
        return (key - 30) + vm::keyboard::KEY_1;
      case 39:
        return vm::keyboard::KEY_0;

      case 40:
        return vm::keyboard::KEY_RETURN;
      case 41:
        return vm::keyboard::KEY_ESC;
      case 42:
        return vm::keyboard::KEY_BACKSPACE;
      case 43:
        return vm::keyboard::KEY_TAB;
      case 44:
        return vm::keyboard::KEY_SPACEBAR;
      case 45:
        return vm::keyboard::KEY_MINUS;
      case 46:
        return vm::keyboard::KEY_EQUAL;
      case 47:
        return vm::keyboard::KEY_LEFT_BRACKET;
      case 48:
        return vm::keyboard::KEY_RIGHT_BRACKET;
      case 49:
        return vm::keyboard::KEY_BACKSLASH;
      case 51:
        return vm::keyboard::KEY_SEMICOLON;
      case 52:
        return vm::keyboard::KEY_APOSTROPHE;
      case 53:
        return vm::keyboard::KEY_GRAVE_ACCENT;
      case 54:
        return vm::keyboard::KEY_COMA;
      case 55:
        return vm::keyboard::KEY_PERIOD;
      case 56:
        return vm::keyboard::KEY_SLASH;
      case 73:
        return vm::keyboard::KEY_INSERT;
      case 76:
        return vm::keyboard::KEY_DELETE;

      case 79:
        return vm::keyboard::KEY_ARROW_RIGHT;
      case 80:
        return vm::keyboard::KEY_ARROW_LEFT;
      case 81:
        return vm::keyboard::KEY_ARROW_DOWN;
      case 82:
        return vm::keyboard::KEY_ARROW_UP;

      case 224:
      case 228:
        return vm::keyboard::KEY_CONTROL;
      
      case 225:
      case 229:
        return vm::keyboard::KEY_SHIFT;
      
      case 230:
        return vm::keyboard::KEY_ALT_GR;

      default:
        return vm::keyboard::KEY_UNKNOW;
      
    }
    return vm::keyboard::KEY_UNKNOW;
  }


  } // end of namespace aux
} // end of namespace vm

