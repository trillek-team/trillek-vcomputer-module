#pragma once
/**
 * RC3200 VM - CDA.hpp
 * Base class of all devices
 *
 */

#ifndef __IDEVICE_HPP__
#define __IDEVICE_HPP__ 1

#include "types.hpp"

#include <memory>

#include <cassert>


namespace vm {

/** 
 * Base class of all devices
 */
class IDevice {
public:

IDevice() : jmp1(0), jmp2(0)
{ }

virtual ~IDevice()
{ }

/**
 * Device Class
 */
virtual byte_t dev_class() const = 0;

/**
 * Device Builder/Vendor
 */
virtual word_t builder() const = 0;

/**
 * Device ID
 */
virtual word_t dev_id() const = 0;

/**
 * Device Version
 */
virtual word_t dev_ver() const = 0;

/**
 * Return Jumper 1 value
 */
word_t getJmp1() const
{
    return jmp1;
}

/**
 * Sets Jumper 1
 */
void setJmp1(byte_t val)
{
    jmp1 = val;
}

/**
 * Return Jumper 2 value
 */
word_t getJmp2() const
{
    return jmp2;
}

/**
 * Sets Jumper 2
 */
void setJmp2(byte_t val)
{
    jmp2 = val;
}

protected:
unsigned jmp1;
unsigned jmp2;

};

} // End of namespace vm

#endif
