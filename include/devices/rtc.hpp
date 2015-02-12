/**
 * \brief       Virtual Computer Real Time Clock
 * \file        rtc.hpp
 * \copyright   LGPL v3
 *
 * Implementation of embedded RTC device
 */
#ifndef __RTC_HPP_
#define __RTC_HPP_ 1

#include "../types.hpp"
#include "../addr_listener.hpp"

#include <ctime>

namespace trillek {
namespace computer {

class RTC : public AddrListener {
public:

    virtual Byte ReadB (DWord addr);
    virtual Word ReadW (DWord addr);
    virtual DWord ReadDW (DWord addr);

    virtual void WriteB (DWord addr, Byte val);
    virtual void WriteW (DWord addr, Word val);
    virtual void WriteDW (DWord addr, DWord val);

private:

    static const int EPOCH_YEAR_OFFSET = 1900 + 0; // TODO Change this depending
                                                   // of the game
                                                   // history/background
};

} // End of namespace computer
} // End of namespace trillek

#endif // __RTC_HPP_
