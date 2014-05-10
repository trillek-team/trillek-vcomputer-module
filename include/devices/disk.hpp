/**
 * \brief       Virtual Computer Generic Media image class
 * \file        disk.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __DISK_HPP_
#define __DISK_HPP_ 1

#include "types.hpp"
#include "vcomputer.hpp"

#include <cstring>
#include <iostream>
#include <fstream>
#include <array>

namespace trillek {
namespace computer {

static const int HEADER_SIZE      = 11; /// Size on bytes of the file header
static const char HEADER_MAGIC[3] = {
    /// Magic "number" to identify the file
    'V', 'C', 'D'
};

/**
 * Media image file class
 */
enum class DiskType : Byte
{
    FLOPPY = 'F'
};

/**
 * Media image file descriptor
 */
struct DiskDescriptor
{
    DiskType TypeDisk;       /// Type of disk
    uint8_t writeProtect;    /// Disk is write protected
    uint8_t NumSides;        /// Total sides of floppy
    uint8_t TracksPerSide;   /// number of tracks per side
    uint8_t SectorsPerTrack; /// number of sectors per track
    uint16_t BytesPerSector; /// number of bytes per sector
};

enum class ERRORS : Byte
{
    NONE       = 0, // No error
    NO_MEDIA   = 2, // Disk file is not open
    PROTECTED  = 3, // Disk is write protected
    BAD_SECTOR = 5  // Sector is bad
};

/**
 * Generic class that represent a media image, and allows to save/read the disk
 * image data from a file
 */
class Disk {
public:

    /**
     * Opens a disk file
     * @param filename Filename were the floppy data is stored
     */
    Disk(const std::string filename);

    /**
     * Creates a new disk file
     */
    Disk(const std::string filename, DiskDescriptor* info);

    /**
     * closes a floppy disk file and destroys this container
     */
    virtual ~Disk();

    /**
     * Return if the disk is valid
     */
    bool isValid() const {
        return datafile.is_open() && datafile.good();
    }

    /**
     * Return info about the disk
     */
    const DiskDescriptor* getDescriptor() {
        return Info.get();
    }

    /**
     * Total number of tracks of this floppy
     */
    uint16_t getTotalTracks() const {
        return Info->NumSides * Info->TracksPerSide;
    }

    /**
     * Total number of sectors of this floppy
     */
    uint16_t getTotalSectors() const {
        return Info->NumSides * Info->TracksPerSide * Info->SectorsPerTrack;
    }

    /**
     * The exponent of the number of bytes per sector
     * 512 = 2^9, return 9
     * to reverse this: 1 << 9 = 2^9 = 512
     */
    uint8_t getBytesExponent() const;

    /**
     * Return if is write protected
     */
    bool isProtected() const {
        return Info->writeProtect;
    }

    /**
     * sets write protection of disk
     */
    void setWriteProtected(bool state) {
        Info->writeProtect = state;
    }

    /**
     * See if that sector is bad
     * Return True if is a bad sector
     */
    bool isSectorBad (uint16_t sector) const;

    /**
     * Change the bad sector flag of a particular sector
     * @param sector Desired sector
     * @param state True to damage these particular sector
     */
    ERRORS setSectorBad (uint16_t sector, bool state);

    /**
     * Try to write data at the desired sector
     * @param sector Desired sector to be written
     * @param data Sector buffer to be written to the disk
     * @param dryRun Only check for errors, the disk is untouched
     * @return NONE, NO_MEDIA, BAD_SECTOR, PROTECTED
     */
    ERRORS writeSector (uint16_t sector, std::vector<uint8_t>* data, bool dryRun = false);

    /**
     * Try to read data at the desired sector
     * @param sector Desired sector to be written
     * @param data Sector buffer to be written to the disk
     * @return NONE, NO_MEDIA, BAD_SECTOR
     */
    ERRORS readSector (uint16_t sector, std::vector<uint8_t>* data);

    /**
     * Returns the filename
     */
    const std::string getFilename() const {
        return filename;
    }

private:

    char HEADER_VERSION;

    std::string filename;  /// file name of disk file
    std::fstream datafile; /// disk file on host

    std::vector<uint8_t> badSectors;      /// Bitmap of bad sectors
    std::unique_ptr<DiskDescriptor> Info; /// disk metrics
};
    
} // End of namespace computer
} // End of namespace trillek

#endif // __DISK_HPP_
