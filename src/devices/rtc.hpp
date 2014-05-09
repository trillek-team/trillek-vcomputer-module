/**
 * \brief       Virtual Computer Real Time Clock
 * \file        rtc.hpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of embedded RTC device
 */
#ifndef __RTC_HPP_
#define __RTC_HPP_ 1

#include "types.hpp"
#include "addr_listener.hpp"

#include <ctime>

namespace vm {

class RTC : public AddrListener {
public:

    virtual byte_t ReadB (dword_t addr);
    virtual word_t ReadW (dword_t addr);
    virtual dword_t ReadDW (dword_t addr);

    virtual void WriteB (dword_t addr, byte_t val);
    virtual void WriteW (dword_t addr, word_t val);
    virtual void WriteDW (dword_t addr, dword_t val);

private:

    static const int EPOCH_YEAR_OFFSET = 1900 + 0; // TODO Change this depending
                                                   // of the game
                                                   // history/background
};
} // End of namespace vm

#endif // __RTC_HPP_
