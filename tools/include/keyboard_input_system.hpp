#pragma once

#include <map>
#include <vector>

#include <cstdio>

namespace OS {
  namespace event {
    enum KEY_STATE {KS_UP, KS_DOWN};

    class KeyboardInputSystem;

    // A keyboard event handler interface. Handlers can be controllers, loggers, etc.
    struct IKeyboardEventHandler {
      std::vector<unsigned int> keys;
      std::vector<unsigned int> chars;
      std::map<unsigned int, KEY_STATE> keyState; // State of the keys.

      virtual ~IKeyboardEventHandler () { }

      /**
       * \brief Called when on the keys reported during register has a state change.
       *
       * \param[in] unsigned int key The key that has had a state change
       * \param[in] KEY_STATE state The new state the key is in
       */
      virtual void KeyStateChange(const unsigned int key, const KEY_STATE state) = 0;
      virtual void KeyStateChange(const unsigned int key, const KEY_STATE state, const KEY_STATE laststate) {
        KeyStateChange(key,state);
      }
      virtual void CharDown(const unsigned int c) { }
      virtual void LostKeyboardFocus() { }
      KeyboardInputSystem* keyboardSystem;
    };

    class KeyboardInputSystem {
    public:
      KeyboardInputSystem() : focusLock(nullptr) { }
      ~KeyboardInputSystem() { }

      /**
       * \brief Called to request focus lock.
       *
       * By requesting focus lock all future keyboard events will go to that handler until it calls ReleaseFocusLock.
       * \param[in] IKeyboardEventHandler* e
       * \return bool True is focus was locked or if this handler already has focus lock otherwise false.
       */
      bool RequestFocusLock(IKeyboardEventHandler* e) {
        if ((this->focusLock == nullptr) || (this->focusLock == e)) {
          this->focusLock = e;
          LostKeyboardFocus(); // Since we are now locking focus we should inform other components about the lose of focus.
          return true;
        }
        return false;
      }

      /**
       * \brief Called when keyboard focus has been lost.
       *
       * When a focus lock is requested or the window simply loses focus lock, this will notify each even handler that focus has been lost.
       * \return void
       */
      void LostKeyboardFocus() {
        for (auto itr = this->eventHandlers.begin(); itr != this->eventHandlers.end(); ++itr) {
          for (auto citr = itr->second.begin(); citr != itr->second.end(); ++citr) {
            (*citr)->LostKeyboardFocus();
          }
        }
      }

      bool HasFocusLock() {
        return !(this->focusLock == nullptr);
      }

      /**
       * \brief Releases focus lock if the handler currently has the lock.
       *
       * \param[in] IKeyboardEventHandler* e
       * \return void No return indication if the lock was successfully released.
       */
      void ReleaseFocusLock(IKeyboardEventHandler* e) {
        if (this->focusLock == e) {
          this->focusLock = nullptr;
        }
      }

      /**
       * \brief Register a KeyboardEventHandler.
       *
       * \param[in] IKeyboardEventHandler* e The keyboard event handler to register.
       * \return void
       */
      void Register(IKeyboardEventHandler* e) {
        e->keyboardSystem = this;
        for (auto itr = e->keys.begin(); itr != e->keys.end(); ++itr) {
          this->eventHandlers[*itr].push_back(e);
        }
        for (auto itr = e->chars.begin(); itr != e->chars.end(); ++itr) {
          this->charHandlers[*itr].push_back(e);
        }

         // Free up the memory now. We don't need to check which keys it is listening to any more.
        e->keys.clear();
        e->chars.clear();
      }

      /**
       * \brief Key Up event.
       *
       * Loops through each event handler that is registered to the supplied key and calls its KeyStateChange method.
       * \param[in] const unsigned int key The key the is now up.
       * \return void
       */
      void KeyUp(const unsigned int key) {
        if (this->eventHandlers.find(key) != this->eventHandlers.end()) {
          for (auto itr = this->eventHandlers[key].begin(); itr != this->eventHandlers[key].end(); ++ itr) {
            if (focusLock != nullptr) {
              if ((*itr) == focusLock) {
                (*itr)->KeyStateChange(key, KS_UP, KS_DOWN);
              }
            }
            else {
              (*itr)->KeyStateChange(key, KS_UP, KS_DOWN);
            }
          }
        }
      }

      /**
       * \brief Called when a character event occurs.
       *
       * These are human readable characters already modified such as shift-a is A or numpad_9 is 9.
       * \param[in] const unsigned int c The character code.
       * \return void
       */
      void CharDown(const unsigned int c) {
        if (this->charHandlers.find(c) != this->charHandlers.end()) {
          for (auto itr = this->charHandlers[c].begin(); itr != this->charHandlers[c].end(); ++ itr) {
            if (focusLock != nullptr) {
              if ((*itr) == focusLock) {
                (*itr)->CharDown(c);
              }
            }
            else {
              (*itr)->CharDown(c);
            }
          }
        }
      }

      /**
       * \brief Key Down event.
       *
       * Loops through each event handler that is registered to the supplied key and calls its KeyStateChange method.
       * \param[in] const unsigned int key The key the is now down.
       */
      void KeyDown(const unsigned int key) {
        if (this->eventHandlers.find(key) != this->eventHandlers.end()) {
          for (auto itr = this->eventHandlers[key].begin(); itr != this->eventHandlers[key].end(); ++ itr) {
            if (focusLock != nullptr) {
              if ((*itr) == focusLock) {
                (*itr)->KeyStateChange(key, KS_DOWN, KS_UP);
              }
            }
            else {
              (*itr)->KeyStateChange(key, KS_DOWN, KS_UP);
            }
          }
        }
      }

      /**
       * \brief Key Repeat event.
       *
       * Loops through each event handler that is registered to the supplied key and calls its KeyStateChange method.
       * \param[in] const unsigned int key The key the is now down.
       */
      void KeyRepeat(const unsigned int key) {
        if (this->eventHandlers.find(key) != this->eventHandlers.end()) {
          for (auto itr = this->eventHandlers[key].begin(); itr != this->eventHandlers[key].end(); ++ itr) {
            if (focusLock != nullptr) {
              if ((*itr) == focusLock) {
                (*itr)->KeyStateChange(key, KS_DOWN, KS_DOWN);
              }
            }
            else {
              (*itr)->KeyStateChange(key, KS_DOWN, KS_DOWN);
            }
          }
        }
      }

    private:
      std::map<unsigned int, std::vector<IKeyboardEventHandler*> > eventHandlers;
      std::map<unsigned int, std::vector<IKeyboardEventHandler*> > charHandlers;
      IKeyboardEventHandler* focusLock;
    };
  }
}
