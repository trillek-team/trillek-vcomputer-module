/**
 * \brief       Virtual Computer Real Time Clock
 * \file        RTC.cpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of embedded RTC device
 */

#include "RTC.hpp"
#include "VSFix.hpp"

namespace vm {

byte_t RTC::ReadB(dword_t addr) {

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

word_t RTC::ReadW(dword_t addr) {

    std::time_t t    = std::time(NULL);
    struct tm* clock = std::gmtime(&t);

    switch (addr)
    {
    case 0x11E030:
        return (clock->tm_sec << 8) + clock->tm_min;

    case 0x11E032:
        return (clock->tm_hour << 8) + clock->tm_mday;

    case 0x11E034:
        return (clock->tm_mon << 8) + (byte_t)(clock->tm_year + EPOCH_YEAR_OFFSET);

    case 0x11E036:
        return (byte_t)( (clock->tm_year + EPOCH_YEAR_OFFSET) >> 8 );

    default:
        return this->ReadB(addr) | (this->ReadB(addr + 1) << 8);
    } // switch
}     // ReadW

dword_t RTC::ReadDW(dword_t addr) {

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

void RTC::WriteB(dword_t addr, byte_t val) {
}

void RTC::WriteW(dword_t addr, word_t val) {
}

void RTC::WriteDW(dword_t addr, dword_t val) {
}
}
