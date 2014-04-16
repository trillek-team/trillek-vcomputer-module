#pragma once
/*!
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

#include "Types.hpp"
#include "VComputer.hpp"

// VM CPUs
#include "TR3200/TR3200.hpp"
#include "DCPU16N/DCPU16N.hpp"

// Devices
#include "devices/TDA.hpp"
#include "devices/GKeyb.hpp"

// Misc
#include "Auxiliar.hpp"

