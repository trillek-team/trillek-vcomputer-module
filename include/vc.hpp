/**
 * \brief       Virtual Computer Main header
 * \file        vc.hpp
 * \copyright   The MIT License (MIT)
 *
 * A header that includes the needed headers to create a virtual computer and
 * run it.
 *
 * One Header to rule them all, One Header to find them,
 * One Header to bring them all and in the darkness bind them
 * In the Land of Mordor where the Shadows lie.
 */
#ifndef __VC_HPP_
#define __VC_HPP_ 1

#include "types.hpp"
#include "vcomputer.hpp"

// VM CPUs
#include "tr3200/tr3200.hpp"

// Devices
#include "devices/tda.hpp"
#include "devices/gkeyb.hpp"
#include "devices/m5fdd.hpp"
#include "devices/debug_serial_console.hpp"

// Misc
#include "auxiliar.hpp"

#endif // __VC_HPP_
