#pragma once
/**
* Trillek Virtual Computer - M5FDD.hpp
* Mackapar 5.25" Floppy Drive
*/

#include "Types.hpp"
#include "VComputer.hpp"

#include <string>
#include <iostream>
#include <fstream>

#define LOG std::cout
#define LOG_DEBUG std::cout
#define LOG_ERROR std::cout

namespace vm {
    namespace dev {
        namespace m5fdd {

            /**
            * M35 Floppy Drive commands
            */
            enum class COMMANDS : uint16_t {
                SET_INTERRUPT = 0x0,
                READ_SECTOR = 0x1,
                WRITE_SECTOR = 0x2,
                QUERY_MEDIA = 0x3,
            };

            /**
            * M35 Floppy Drive status codes
            */
            enum class STATE_CODES : uint16_t {
                NO_MEDIA = 0,   /// There's no floppy in the drive
                READY = 1,      /// The drive is ready to accept commands
                READY_WP = 2,   /// Same as ready, but the floppy is Write Protected
                BUSY = 3,       /// The drive is busy either reading or writing a sector
            };

            /**
            * M35 Floppy Device error codes
            */
            enum class ERROR_CODES : uint16_t {
                NONE = 0,       /// No error since the last poll
                BUSY = 1,       /// Drive is busy performning a action
                NO_MEDIA = 2,   /// Attempted to read or write without a floppy
                PROTECTED = 3,  /// Attempted to write to a protected floppy
                EJECT = 4,      /// The floppy was ejected while was reading/writing
                BAD_SECTOR = 5, /// The requested sector is broken, the data on it is lost

                BROKEN = 0xFFFF /// There's been some major software/hardware problem.
                /// Try to do a hard reset the device.
            };


            static const char FileHeader[] = { 'F', 1 };

            class M35_Floppy;

            /**
            * How many cycles need to move to a nearby track
            */
            static const unsigned SEEK_CYCLES_PER_TRACK = 240;

            /**
            * How many cycles need to read/write a whole sector
            */
            static const unsigned READ_CYCLES_PER_SECTOR = 1668;
            static const unsigned WRITE_CYCLES_PER_SECTOR = 1668;

            /**
            * How many sectors are in a track
            */
            static const unsigned SECTORS_PER_TRACK = 18;

            /**
            * How many words are stored in a sector
            */
            static const unsigned SECTOR_SIZE = 512;
            static const unsigned SECTOR_SIZE_BYTES = 1024;

            /**
            * Mackapar 3,5" floppy drive
            */
            class M35FD : public IDevice {
            public:
                M35FD();
                virtual ~M35FD();

                /*!
                * Resets device internal state
                * Called by VComputer
                */
                virtual void Reset();

                /**
                * Sends (writes to CMD register) a command to the device
                * @param cmd Command value to send
                */
                virtual void SendCMD(word_t cmd);

                virtual void A(word_t val) { a = val; }
                virtual void B(word_t val) { b = val; }
                virtual void C(word_t val) { c = val; }

                virtual word_t A() { return a; }
                virtual word_t B() { return b; }
                virtual word_t C() { return c; }
                virtual word_t D() { return static_cast<word_t>(state); }
                virtual word_t E() { return static_cast<word_t>(error); }

                /**
                * Device Type
                */
                virtual byte_t DevType() const {
                    return 0x08; // Mass Storage Device
                }

                /**
                * Device SubType
                */
                virtual byte_t DevSubType() const {
                    return 0x01; // Floppy Drive
                }

                /**
                * Device ID
                */
                virtual byte_t DevID() const {
                    return 0x01; // Mackapar 5.25" Floppy Drive
                }

                /**
                * Device Vendor ID
                */
                virtual dword_t DevVendorID() const {
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
                virtual void Tick(unsigned n = 1, const double delta = 0);

                /*!
                * Checks if the device is trying to generate an interrupt
                *
                * IDevice implementation does nothing.
                * \param[out] msg The interrupt message will be writen here
                * \return True if is generating a new interrupt
                */
                virtual bool DoesInterrupt(word_t& msg);

                /*!
                * Informs to the device that his generated interrupt was accepted by the CPU
                *
                * IDevice implementation does nothing.
                */
                virtual void IACK();

                /*!
                * Writes a copy of Device state in a chunk of memory pointer by ptr.
                * \param[out] ptr Pointer were to write
                * \param[in,out] size Size of the chunk of memory were can write. If is
                * sucesfull, it will be set to the size of the write data.
                */
                virtual void GetState(void* ptr, std::size_t& size) const {}

                /*!
                * Sets the Device state.
                * \param ptr[in] Pointer were read the state information
                * \param size Size of the chunk of memory were will read.
                * \return True if can read the State data from the pointer.
                */
                virtual bool SetState(const void* ptr, std::size_t size) { return true; }

                //----------------------------------------------------

                /**
                * @brief Inserts a floppy in the unit
                * If there is a floppy disk previusly inserte, this is ejected
                * @param floppy Floppy disk
                */
                void insertFloppy(std::shared_ptr<M35_Floppy> floppy);

                /**
                * @brief Ejects the floppy actually isnerted if is there one
                */
                void eject();


                friend class M35_Floppy;

            protected:
                std::shared_ptr<M35_Floppy> floppy;     /// Floppy inserted
                STATE_CODES state;      /// Floppy drive actual estatus
                ERROR_CODES error;      /// Floppy drive actual error state

                unsigned busy_cycles;   /// CPU Cycles that the device will be busy
                uint16_t msg;           /// Mesg to send if need to trigger a interrupt
                bool trigger;           /// Must launch a interrupt from device to CPU ?
                dword_t a, b, c, d;
            };

            /**
            * @brief M35FD floppy disk
            *
            * File structure must be like this :
            *
            * Bytes      0          1          2          3           4
            *            ----------------------------------------------
            * Head:      #   Type   | Version  #  Unused  |   Tracks  |
            *            ----------------------------------------------
            *            |                   Data                     |
            *            |                  size =                    |
            *            | 2*Tracks * SECTORS_PER_TRACK * SECTOR_SIZE |
            *            |                                            |
            *            ----------------------------------------------
            *            |             Bad Sectors BitMap             |
            *            |                   size =                   |
            *            |          Tracks * SECTORS_PER_TRACK / 8    |
            *            |                                            |
            *            ----------------------------------------------
            * Header:
            * Type = 'F' (Floppy image)
            * Version = 1
            * This kind of header will allow future upgrades and if we need diferent
            * data files (cassetes, tapes, hard disk, etc...), we will share the same
            * basic header.
            *
            * Floppy data:
            * Tracks : Number of tracks, should be 40 or 80, I don't expect any other
            * track size.
            *
            * Data: RAW data. To access a particular sector, you only need to read at
            *     (4 + sector * SECTOR_SIZE)
            *
            * BitMap:
            * The bitmap stores 8 sectors state in each byte. It uses the MSB bit for
            * the lowest sector and LSB for the bigger sector.
            * To read is a particular sector is bad, you read the byte at
            *     ( (4 + Size of Data secction) +
            *         (sector /8 ) & 128 >> (sector % 8) ) != 0
            *
            * The RAW data will be read/write directly to the file, but the bitmap will
            * be keep in RAM for quick read of it.
            */
            class M35_Floppy {
            public:


                /**
                * Creates a new floppy device
                * @param filename Filename were the floppy data is stored
                * @param tracks Number of tracks of the floppy medium
                * @param wp Write protected ?
                */
                M35_Floppy(const std::string filename, uint8_t tracks = 80,
                    bool wp = false);
                virtual ~M35_Floppy();

                /**
                * Total number of tracks of this floppy
                */
                uint8_t getTotalTracks() const {
                    return tracks;
                }

                /**
                * Total number of sectors of this floppy
                */
                uint16_t getTotalSectors() const {
                    return tracks * SECTORS_PER_TRACK;
                }

                /**
                * Gets the actual Track
                */
                uint16_t getTrack() const {
                    return last_sector / SECTORS_PER_TRACK;
                }

                /**
                * Sets Write protecction flag
                */
                void setProtected(bool val) {
                    wp_flag = val;
                }

                /**
                * Return if is write protected
                */
                bool isProtected() const {
                    return wp_flag;
                }

                /**
                * See if that sector is bad
                * Return True if is a bad sector
                */
                bool isSectorBad(uint16_t sector) const;

                /**
                * Change the bad sector flag of a particular sector
                * @param sector Desired sector
                * @param state True to damage these particular sector
                */
                void setSectorBad(uint16_t sector, bool state);

                /**
                * Try to write data at the desired sector
                * @param sector Desired sector to be writed
                * @param addr Data address (DCPU) to be written in the sector
                * @param cycles Number of cycles that takes to finish the operation
                * @return NONE, PROTECTED or BAD_SECTOR
                */
                ERROR_CODES write(uint16_t sector, uint16_t addr, unsigned& cycles);


                /**
                * Try to write data at the desired sector (Not during an emulation !)
                * @param sector Desired sector to be writed
                * @param the data to write must be SECTOR_SIZE at least !!
                * @return NONE, PROTECTED or BAD_SECTOR
                */
                ERROR_CODES writeToFile(uint16_t sector, const char* data);

                /**
                * Try to read data at the desired sector
                * @param sector Desired sector to be writed
                * @param addr Address (DCPU) were to write the data
                * @param cycles Number of cycles that takes to finish the operation
                * @return NONE or BAD_SECTOR
                */
                ERROR_CODES read(uint16_t sector, uint16_t addr, unsigned& cycles);

                /**
                * Does background task every CPU cycle
                */
                void tick();

                /**
                * Returns the filename
                */
                const std::string& getFilename() const {
                    return this->filename;
                }

                /**
                * Write all the wholes bad sector on the file associated
                */
                void writeBadSectorsToFile();

                friend class M35FD;

            protected:
                std::string filename;   /// Mainly for info&debug
                uint8_t tracks;         /// Total tracks of the floppy
                uint8_t* bad_sectors;   /// Bitmap of bad sectors

                std::fstream datafile;

                bool wp_flag;           /// Is write protected
                uint16_t last_sector;   /// Last sector write/read

                uint16_t cursor;        /// Points to where read/write in CPU ram
                uint16_t count;         /// Counts how many words has read/write
                bool reading;           /// Is reading or writting

                M35FD* drive;
                /**
                * Moves the head to the desired track
                * @retun Number of cycles that need to seek the desired track
                */
                unsigned setTrack(uint16_t track) {
                    if (track > getTrack()) {
                        return (track - getTrack()) * SEEK_CYCLES_PER_TRACK;
                    }
                    else {
                        return (getTrack() - track) * SEEK_CYCLES_PER_TRACK;
                    }
                }

            };

        } // End of namespace m5ffd
    } // End of namespace dev
} // End of namespace vm
