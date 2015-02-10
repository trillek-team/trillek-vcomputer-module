/**
 * \brief       Base class for Devices
 * \file        device.hpp
 * \copyright   LGPL v3
 *
 * Defines a base interface for all Devices in the Virtual Computer
 */
#ifndef __IDEVICE_HPP_
#define __IDEVICE_HPP_ 1

#include "types.hpp"
#include "vcomputer.hpp"
#include "vc_dll.hpp"

namespace trillek {
namespace computer {

class VComputer;

/**
 * \class Device
 * Interface that must be implemente by any Device that will be used by the
 * Virtual Computer.
 * Derived class constructors should set vcomp == nullptr.
 */
class DECLDIR Device {
public:

    virtual ~Device() {
    }

    /**
     * Sets the VComputer pointer
     * This method must be only called by VComputer itself
     * \param[in] vcomp VComputer pointer or nullptr
     * \sa VComputer
     */
    virtual void SetVComputer (VComputer* vcomp) {
        this->vcomp = vcomp;
    }

    /**
     * Resets device internal state
     * Called by VComputer
     */
    virtual void Reset () = 0;

    /**
     * Sends (writes to CMD register) a command to the device
     * \param cmd Command value to send
     */
    virtual void SendCMD (Word cmd) = 0;

    virtual void A (Word cmd) {
    } /// Set A register value

    virtual void B (Word cmd) {
    } /// Set B register value

    virtual void C (Word cmd) {
    } /// Set C register value

    virtual void D (Word cmd) {
    } /// Set D register value

    virtual void E (Word cmd) {
    } /// Set E register value

    virtual Word A () {
        return 0;
    } /// Return A register value

    virtual Word B () {
        return 0;
    } /// Return B register value

    virtual Word C () {
        return 0;
    } /// Return C register value

    virtual Word D () {
        return 0;
    } /// Return D register value

    virtual Word E () {
        return 0;
    } /// Return E register value

    /**
     * Device Type
     */
    virtual Byte DevType () const = 0;

    /**
     * Device SubType
     */
    virtual Byte DevSubType () const = 0;

    /**
     * Device ID
     */
    virtual Byte DevID () const = 0;

    /**
     * Device Vendor ID
     */
    virtual DWord DevVendorID () const = 0;

    /**
     * Return if the device does something each Device Clock cycle.
     * Few devices really need to do this, so IDevice implementation
     * returns false.
     */
    virtual bool IsSyncDev() const {
        return false;
    }

    /**
     * Executes N Device clock cycles.
     *
     * Here resides the code that is executed every Device Clock tick.
     * IDevice implementation does nothing.
     * \param n Number of clock cycles to be executed
     * \param delta Number milliseconds since the last call
     */
    virtual void Tick (unsigned n = 1, const double delta = 0) {
    }

    /**
     * Checks if the device is trying to generate an interrupt
     *
     * IDevice implementation does nothing.
     * \param[out] msg The interrupt message will be writen here
     * \return True if is generating a new interrupt
     */
    virtual bool DoesInterrupt(Word& msg) {
        return false;
    }

    /**
     * Informs to the device that his generated interrupt was accepted by the
     **CPU
     *
     * IDevice implementation does nothing.
     */
    virtual void IACK () {
    }

    /**
     * Writes a copy of Device state in a chunk of memory pointer by ptr.
     * \param[out] ptr Pointer were to write
     * \param[in,out] size Size of the chunk of memory were can write. If is
     * sucesfull, it will be set to the size of the write data.
     */
    virtual void GetState (void* ptr, std::size_t& size) const = 0;

    /**
     * Sets the Device state.
     * \param ptr[in] Pointer were read the state information
     * \param size Size of the chunk of memory were will read.
     * \return True if can read the State data from the pointer.
     */
    virtual bool SetState (const void* ptr, std::size_t size) = 0;

protected:

    VComputer* vcomp; /// Ptr to the Virtual Computer
};

} // End of namespace computer
} // End of namespace trillek

#endif // __IDEVICE_HPP_

