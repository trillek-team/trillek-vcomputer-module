#pragma once
/*!
 * \brief       Virtual Computer Device API
 * \file        ICPU.hpp
 * \copyright   The MIT License (MIT)
 *
 * Defines a base interface for all Devices in the Virtual Computer
 */

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {

    class VComputer;

    /*!
     * \class IDevice
     * Interface that must be implemente by any Device that will be used by the
     * Virtual Computer.
     * Derived class constructors should set vcomp == nullptr.
     */
    class IDevice {
        public:

        virtual ~IDevice() {
        }

        /*!
         * Sets the VComputer pointer
         * This method must be only called by VComputer itself
         * \param[in] vcomp VComputer pointer or nullptr
         * \sa VComputer
         */
        virtual void SetVComputer (VComputer* vcomp) {
            this->vcomp = vcomp;
        }

        /*!
         * Resets device internal state
         * Called by VComputer
         */
        virtual void Reset () = 0;

        /*!
         * Sends (writes to CMD register) a command to the device
         * \param cmd Command value to send
         */
        virtual void SendCMD (word_t cmd) = 0;

        virtual void A (word_t cmd) { }     //! Set A register value

        virtual void B (word_t cmd) { }     //! Set B register value

        virtual void C (word_t cmd) { }     //! Set C register value

        virtual void D (word_t cmd) { }     //! Set D register value

        virtual void E (word_t cmd) { }     //! Set E register value

        virtual word_t A () { return 0; }   //! Return A register value

        virtual word_t B () { return 0; }   //! Return B register value

        virtual word_t C () { return 0; }   //! Return C register value

        virtual word_t D () { return 0; }   //! Return D register value

        virtual word_t E () { return 0; }   //! Return E register value

        /*!
         * Device Type
         */
        virtual byte_t DevType() const = 0;

        /*!
         * Device SubType
         */
        virtual byte_t DevSubType() const = 0;

        /*!
         * Device ID
         */
        virtual byte_t DevID() const = 0;

        /*!
         * Device Vendor ID
         */
        virtual dword_t DevVendorID() const = 0;

        /*!
         * Return if the device does something each Device Clock cycle.
         * Few devices really need to do this, so IDevice implementation
         * returns false.
         */
        virtual bool IsSyncDev() const {
            return false;
        }

        /*!
         * Executes N Device clock cycles.
         *
         * Here resides the code that is executed every Device Clock tick.
         * IDevice implementation does nothing.
         * \param n Number of clock cycles to be executed
         * \param delta Number milliseconds since the last call
         */
        virtual void Tick (unsigned n=1, const double delta = 0) {
        }

        /*!
         * Checks if the device is trying to generate an interrupt
         *
         * IDevice implementation does nothing.
         * \param[out] msg The interrupt message will be writen here
         * \return True if is generating a new interrupt
         */
        virtual bool DoesInterrupt(word_t& msg) {
            return false;
        }

        /*!
         * Informs to the device that his generated interrupt was accepted by the CPU
         *
         * IDevice implementation does nothing.
         */
        virtual void IACK () {
        }

        /*!
         * Writes a copy of Device state in a chunk of memory pointer by ptr.
         * \param[out] ptr Pointer were to write
         * \param[in,out] size Size of the chunk of memory were can write. If is
         * sucesfull, it will be set to the size of the write data.
         */
        virtual void GetState (void* ptr, std::size_t& size) const = 0;

        /*!
         * Sets the Device state.
         * \param ptr[in] Pointer were read the state information
         * \param size Size of the chunk of memory were will read.
         * \return True if can read the State data from the pointer.
         */
        virtual bool SetState (const void* ptr, std::size_t size) = 0;

        protected:

        VComputer* vcomp;   //! Ptr to the Virtual Computer
    };

} // End of namespace vm

