/**
 * \brief       Mackapar 5.25" Floppy Drive
 * \file        m5fdd.hpp
 * \copyright   LGPL v3
 *
 * 5.25" Floppy Drive
 * Implement specs v0.2.3
 */
#ifndef __M5FDD_HPP_
#define __M5FDD_HPP_ 1

#include "../vcomputer.hpp"

#include "media.hpp"

#include <string>
#include <iostream>
#include <fstream>

namespace trillek {
namespace computer {
namespace m5fdd {

/**
 * M35 Floppy Drive commands
 */
enum class COMMANDS : uint16_t {
    SET_INTERRUPT = 0x0,
    READ_SECTOR   = 0x1,
    WRITE_SECTOR  = 0x2,
    QUERY_MEDIA   = 0x3,
};

/**
 * M35 Floppy Drive status codes
 */
enum class STATE_CODES : uint16_t {
    NO_MEDIA = 0, /// There's no floppy in the drive
    READY    = 1, /// The drive is ready to accept commands
    READY_WP = 2, /// Same as ready, but the floppy is Write Protected
    BUSY     = 3, /// The drive is busy either reading or writing a sector
};

/**
 * M35 Floppy Device error codes
 * This is a superset of Disk ERROR enum
 */
enum class ERROR_CODES : uint16_t {
    NONE       = 0, /// No error since the last poll
    BUSY       = 1, /// Drive is busy performing a action
    NO_MEDIA   = 2, /// Attempted to read or write without a floppy
    PROTECTED  = 3, /// Attempted to write to a protected floppy
    EJECT      = 4, /// The floppy was ejected while was reading/writing
    BAD_SECTOR = 5, /// The requested sector is broken, the data on it is lost
    BAD_CHS    = 6, /// The CHS value is not valid. Check QUERY_MEDIA

    BROKEN = 0xFFFF /// There's been some major software/hardware problem.
                    /// Try to do a hard reset the device.
};

/**
 * 5.25" floppy drive
 */
class M5FDD : public Device {
public:

	DECLDIR M5FDD();
	DECLDIR virtual ~M5FDD();

    /*!
     * Resets device internal state
     * Called by VComputer
     */
	DECLDIR virtual void Reset();

    /**
     * Sends (writes to CMD register) a command to the device
     * @param cmd Command value to send
     */
	virtual void SendCMD(Word cmd);

    virtual void A(Word val) {
        a = val;
    }

    virtual void B(Word val) {
        b = val;
    }

    virtual void C(Word val) {
        c = val;
    }

    virtual Word A() {
        return a;
    }

    virtual Word B() {
        return b;
    }

    virtual Word C() {
        return c;
    }

    virtual Word D() {
        return static_cast<Word>(state);
    }

    virtual Word E() {
        return static_cast<Word>(error);
    }

    /**
     * Device Type
     */
    virtual Byte DevType() const {
        return 0x08; // Mass Storage Device
    }

    /**
     * Device SubType
     */
    virtual Byte DevSubType() const {
        return 0x01; // Floppy Drive
    }

    /**
     * Device ID
     */
    virtual Byte DevID() const {
        return 0x01; // Mackapar 5.25" Floppy Drive
    }

    /**
     * Device Vendor ID
     */
    virtual DWord DevVendorID() const {
        return 0x1EB37E91; // Mackapar Media
    }

    /*!
     * Return if the device does something each Device Clock cycle.
     * Few devices really need to do this, so IDevice implementation
     * returns false.
     */
    virtual bool IsSyncDev() const {
        return true;
    }

    /*!
     * Executes N Device clock cycles.
     *
     * Here resides the code that is executed every Device Clock tick.
     * IDevice implementation does nothing.
     * \param n Number of clock cycles to be executed
     * \param delta Number milliseconds since the last call
     */
    virtual void Tick (unsigned n = 1, const double delta = 0);

    /*!
     * Checks if the device is trying to generate an interrupt
     *
     * IDevice implementation does nothing.
     * \param[out] msg The interrupt message will be written here
     * \return True if is generating a new interrupt
     */
    virtual bool DoesInterrupt (Word& msg);

    /*!
     * Informs to the device that his generated interrupt was accepted by the
     **CPU
     *
     * IDevice implementation does nothing.
     */
    virtual void IACK ();

    /*!
     * Writes a copy of Device state in a chunk of memory pointer by ptr.
     * \param[out] ptr Pointer were to write
     * \param[in,out] size Size of the chunk of memory were can write. If is
     * successful, it will be set to the size of the write data.
     */
	DECLDIR virtual void GetState(void* ptr, std::size_t& size) const {
    }

    /*!
     * Sets the Device state.
     * \param ptr[in] Pointer were read the state information
     * \param size Size of the chunk of memory were will read.
     * \return True if can read the State data from the pointer.
     */
	DECLDIR  virtual bool SetState(const void* ptr, std::size_t size) {
        return true;
    }

    //----------------------------------------------------

    /**
     * @brief Inserts a floppy in the unit
     * If there is a floppy disk previously inserted, this is ejected
     * @param floppy Floppy disk
     */
	DECLDIR void insertFloppy(std::shared_ptr<Media> floppy);

    /**
     * @brief Ejects the floppy actually inserted if is there one
     */
	DECLDIR void ejectFloppy();

private:

    /**
     * Moves the head to the desired position
     */
    void setSector (uint8_t track, uint8_t head, uint8_t sector);

    std::shared_ptr<Media> floppy;  /// Floppy inserted
    std::vector<Byte> sectorBuffer; // buffer of sector being accessed
    STATE_CODES state;              /// Floppy drive actual status
    ERROR_CODES error;              /// Floppy drive actual error state

    bool writing;           /// is the drive reading or writing?
    unsigned curHead;       /// current head
    unsigned curTrack;      /// current track the head is at
    unsigned curSector;     /// current sector the head is at
    unsigned curPosition;   /// current DMA position inside of the sector
    unsigned busyCycles;    /// Device Cycles that the device will be busy
    DWord dmaLocation;      /// RAM Location for the DMA transfer

    uint16_t msg;          /// Msg to send if need to trigger a interrupt
    bool pendingInterrupt; /// Must launch a interrupt from device to CPU ?
    DWord a, b, c, d;    /// Data registers
};

} // End of namespace m5ffd
} // End of namespace computer
} // End of namespace trillek

#endif // __M5FDD_HPP_
