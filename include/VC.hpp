/**
 * \brief       Virtual Computer Main header
 * \file        VC.hpp
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

#include "Types.hpp"
#include "VComputer.hpp"

// VM CPUs
#include "TR3200/TR3200.hpp"
#include "DCPU16N.hpp"

// Devices
#include "devices/TDA.hpp"
#include "devices/GKeyb.hpp"
#include "devices/M5FDD.hpp"
#include "devices/DebugSerialConsole.hpp"

// Embed devices
#include "devices/Timer.hpp"
#include "devices/Beeper.hpp"
#include "devices/RNG.hpp"
#include "devices/RTC.hpp"

// Misc
#include "Auxiliar.hpp"

#endif // __VC_HPP_
