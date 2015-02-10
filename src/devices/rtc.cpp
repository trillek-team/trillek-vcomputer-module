/**
 * \brief       Virtual Computer Real Time Clock
 * \file        rtc.cpp
 * \copyright   LGPL v3
 *
 * Implementation of embedded RTC device
 */

#include "devices/rtc.hpp"
#include "vs_fix.hpp"

namespace trillek {
namespace computer {

Byte RTC::ReadB(DWord addr) {

    std::time_t t    = std::time(NULL);
    struct tm* clock = std::gmtime(&t);

    switch (addr)
    {
    case 0x11E030:
        return clock->tm_sec;

    case 0x11E031:
        return clock->tm_min;

    case 0x11E032:
        return clock->tm_hour;

    case 0x11E033:
        return clock->tm_mday;

    case 0x11E034:
        return clock->tm_mon;

    case 0x11E035:
        return clock->tm_year + EPOCH_YEAR_OFFSET;

    case 0x11E036:
        return (clock->tm_year + EPOCH_YEAR_OFFSET) >> 8;

    default:
        return 0;
    } // switch
}     // ReadB

Word RTC::ReadW(DWord addr) {

    std::time_t t    = std::time(NULL);
    struct tm* clock = std::gmtime(&t);

    switch (addr)
    {
    case 0x11E030:
        return (clock->tm_sec << 8) + clock->tm_min;

    case 0x11E032:
        return (clock->tm_hour << 8) + clock->tm_mday;

    case 0x11E034:
        return (clock->tm_mon << 8) + (Byte)(clock->tm_year + EPOCH_YEAR_OFFSET);

    case 0x11E036:
        return (Byte)( (clock->tm_year + EPOCH_YEAR_OFFSET) >> 8 );

    default:
        return this->ReadB(addr) | (this->ReadB(addr + 1) << 8);
    } // switch
}     // ReadW

DWord RTC::ReadDW(DWord addr) {

    std::time_t t    = std::time(NULL);
    struct tm* clock = std::gmtime(&t);

    switch (addr)
    {
    case 0x11E030:
        return (clock->tm_sec << 24) + (clock->tm_min << 16) + (clock->tm_hour << 8) + clock->tm_mday;

    case 0x11E034:
        return (clock->tm_mon << 24) + ( (clock->tm_year + EPOCH_YEAR_OFFSET) << 8 );

    default:
        return this->ReadW(addr) | (this->ReadW(addr + 2) << 16);
    }
} // ReadDW

void RTC::WriteB(DWord addr, Byte val) {
}

void RTC::WriteW(DWord addr, Word val) {
}

void RTC::WriteDW(DWord addr, DWord val) {
}

} // namespace computer
} // namespace trillek
