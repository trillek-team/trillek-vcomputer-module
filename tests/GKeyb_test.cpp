/**
 * Unit tests of Generic keyboard device
 */
#include "devices/GKeyb.hpp"

#include "gtest/gtest.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

// Check if E register returns buffer size
TEST(GKeyboard, E_reg) {
  using namespace vm::dev::gkeyboard;

  GKeyboardDev gk;
  gk.Reset();

  ASSERT_EQ(0, gk.E());

  gk.SendKeyEvent ( SCANCODES::SCAN_MINUS, '-', 0 );
  ASSERT_EQ(1, gk.E());

  gk.SendKeyEvent ( SCANCODES::SCAN_SPACE, ' ', 0 );
  ASSERT_EQ(2, gk.E());

}

// Check if sending and pulling an event, works
TEST(GKeyboard, SEND_PULL_event) {
  using namespace vm::dev::gkeyboard;

  GKeyboardDev gk;
  gk.Reset();

  for (unsigned c=0; c < 255; c++) {
    for (unsigned status=0; status <= 7; status++) {
      // We use a fake scan code but allow to test more rich value set
      gk.SendKeyEvent ( 0xFFFF ^ c, c, status );
      ASSERT_EQ(1, gk.E());

      gk.SendCMD(0x0001);
      ASSERT_EQ(0, gk.E()) << "PULL-KEY failed to removed a event from buffer";
      ASSERT_EQ(c, gk.A()) << "Key Code mismatch";
      ASSERT_EQ(0xFFFF ^c, gk.B()) << "ScanCode mismatch";
      ASSERT_EQ(status, gk.C()) << "Status bits mismatch";
    }
  }
}

// Check if sending and pulling an event, works
TEST(GKeyboard, Fill_buffer) {
  using namespace vm::dev::gkeyboard;

  GKeyboardDev gk;
  gk.Reset();

  for (unsigned i=0; i < BSIZE; i++) {
    gk.SendKeyEvent ( SCANCODES::SCAN_MINUS, '-', 0 );
    ASSERT_EQ(i+1, gk.E());
  }

  gk.SendKeyEvent ( SCANCODES::SCAN_SPACE, ' ', 0 );
  ASSERT_EQ(BSIZE, gk.E()) << "Buffer bigger that BSIZE (= 64)";

}

