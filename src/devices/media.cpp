/**
 * \brief       Virtual Computer Generic Media image class
 * \file        media.cpp
 * \copyright   LGPL v3
 *
 */
#include "devices/media.hpp"
#include "vs_fix.hpp"

#include <cmath>
#include <cstdio>

namespace trillek {
namespace computer {


int32_t CHStoLBA (uint8_t track, uint8_t head, uint8_t sector, const DiskDescriptor& descriptor ) {
    if ( head >= descriptor.NumSides
        || track >= descriptor.TracksPerSide
        || sector >= descriptor.SectorsPerTrack
        || sector == 0 ) {

        return -1; // Bad CHS value
    }

    // read the sector
    return (track * descriptor.NumSides + head) * descriptor.SectorsPerTrack + sector - 1;
}

Media::Media(const std::string& filename) : HEADER_VERSION(2) {

    // Check if file exists
    datafile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if ( !datafile.good() ) {
#ifndef NDEBUG
        std::cout << "[DISK] File could not be opened: " << filename.c_str() << std::endl;
#endif
        datafile.close();
        return;
    }

    if (!readHeader()) {
        datafile.close();
        return;
    }
    makeOffsets();

    readBitmap();
    if (HEADER_VERSION == 1) upgradeMedia(2);

#ifndef NDEBUG
    std::cout << "[DISK] File loaded: " << filename.c_str() << std::endl;
#endif
}

Media::Media(const std::string& filename, DiskDescriptor* info) : HEADER_VERSION(2) {
    createMedia(filename, info);
}

Media::Media(const std::string& filename, const DiskDescriptor& info) : HEADER_VERSION(2) {
    DiskDescriptor* tmpInfo = new DiskDescriptor();
    std::memmove(tmpInfo, &info, sizeof(DiskDescriptor));
    createMedia(filename, tmpInfo);
}

void Media::createMedia(const std::string& filename, DiskDescriptor* info) {
    Info.reset(info);
    // create new file
#ifndef NDEBUG
    std::cout << "[DISK] Creating file: " << filename.c_str() << std::endl;
#endif

    datafile.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    writeHeader();
    makeOffsets();

    char* sector = new char[Info->BytesPerSector];
    std::memset(sector, 0, Info->BytesPerSector);
    datafile.seekg(offset_sectors, std::fstream::beg);
    for (uint16_t i = 0; i < getTotalSectors(); i++) {
        datafile.write(sector, Info->BytesPerSector);
    }
    delete[] sector;

    writeBitmap();

    datafile.flush();
}

int Media::readHeader() {

    char temp[4];
    datafile.seekg(0, std::fstream::beg);
    datafile.read(temp, 4);
    if (std::memcmp(temp, HEADER_MAGIC, 3) != 0) {
#ifndef NDEBUG
        std::cout << "[DISK] File not a valid disk image: " << filename.c_str() << std::endl;
#endif
        return 0;
    }

    switch (temp[3]) {
    case 1:
    case 2:
        HEADER_VERSION = temp[3];
        break;
    default:
#ifndef NDEBUG
        std::cout << "[DISK] File is wrong version: " << filename.c_str() << std::endl;
#endif
        return 0;
    }

    Info.reset(new DiskDescriptor);

    /* Read meta info from disk */
    datafile.read(reinterpret_cast<char*>(&Info->TypeDisk), 1);
    datafile.read(reinterpret_cast<char*>(&Info->writeProtect), 1);
    datafile.read(reinterpret_cast<char*>(&Info->NumSides), 1);
    datafile.read(reinterpret_cast<char*>(&Info->TracksPerSide), 1);
    datafile.read(reinterpret_cast<char*>(&Info->SectorsPerTrack), 1);
    datafile.read(reinterpret_cast<char*>(&Info->BytesPerSector), 2);

    return 1;
}

void Media::writeHeader() {
    datafile.seekg(0, std::fstream::beg);
    /* write file header */
    datafile.write(HEADER_MAGIC, 3);
    datafile.write(&HEADER_VERSION, 1);
    datafile.write(reinterpret_cast<char*>(&Info->TypeDisk), 1);
    datafile.write(reinterpret_cast<char*>(&Info->writeProtect), 1);
    datafile.write(reinterpret_cast<char*>(&Info->NumSides), 1);
    datafile.write(reinterpret_cast<char*>(&Info->TracksPerSide), 1);
    datafile.write(reinterpret_cast<char*>(&Info->SectorsPerTrack), 1);
    datafile.write(reinterpret_cast<char*>(&Info->BytesPerSector), 2);
}

void Media::makeOffsets() {
    /* calculate the base offsets for data */
    switch (HEADER_VERSION) {
    case 1:
        offset_sectors = HEADER_SIZE;
        offset_bitmap = offset_sectors + getTotalSectors() * Info->BytesPerSector;
        break;
    default:
    case 2:
        offset_sectors = 0x20;
        offset_bitmap = offset_sectors + getTotalSectors() * Info->BytesPerSector;
        break;
    }

    int bitmapSize = getTotalSectors() / 8;
    bitmapSize += getTotalSectors() % 8 != 0 ? 1 : 0;
    const uint8_t fill = 0;
    if (badSectors.size() != bitmapSize) {
        badSectors = std::vector<uint8_t>(bitmapSize, fill);
    }

#ifndef NDEBUG
    std::fprintf(stderr, "[DISK] bitmap at 0x%04zX, size 0x%04zX\n", offset_bitmap, bitmapSize);
#endif

}

Media::~Media() {
    if ( datafile.is_open() ) {
        datafile.flush();
        datafile.close();
#ifndef NDEBUG
        std::cout << "[DISK] Datafile closed" << std::endl;
#endif
    }
}

/* Write the bad-sector bitmap to the file */
void Media::writeBitmap() {
    datafile.seekg(offset_bitmap, std::fstream::beg);
    datafile.write( reinterpret_cast<char*>( badSectors.data() ), badSectors.size() );
}

/* Get bad sector bitmap from the file */
void Media::readBitmap() {
    datafile.seekg(offset_bitmap, std::fstream::beg);
    datafile.read( reinterpret_cast<char*>( badSectors.data() ), badSectors.size() );
}

/* convert from one version of media to another */
void Media::upgradeMedia(char to_version) {
    size_t offset_sectors_old = offset_sectors;
    if (to_version == HEADER_VERSION) return;
#ifndef NDEBUG
    std::cout << "[DISK] Upgrade file version from " << (int)HEADER_VERSION << " to " << (int)to_version << std::endl;
#endif
    HEADER_VERSION = to_version;
    makeOffsets();
    if (offset_sectors > offset_sectors_old) {
        /* move sector data up by starting at the end and going down */
        char* sector = new char[Info->BytesPerSector];
        std::memset(sector, 0, Info->BytesPerSector);
        for (size_t i = getTotalSectors(); i-- > 0;) {
            size_t offset = i * Info->BytesPerSector;
            datafile.seekg(offset_sectors_old + offset, std::fstream::beg);
            datafile.read(sector, Info->BytesPerSector);
            datafile.seekg(offset_sectors + offset, std::fstream::beg);
            datafile.write(sector, Info->BytesPerSector);
        }
        delete[] sector;
    }
    else if (offset_sectors < offset_sectors_old) {
        /* move sector data down by starting at the beginning and going up */
        char* sector = new char[Info->BytesPerSector];
        std::memset(sector, 0, Info->BytesPerSector);
        for (size_t i = 0; i < getTotalSectors(); i++) {
            size_t offset = i * Info->BytesPerSector;
            datafile.seekg(offset_sectors_old + offset, std::fstream::beg);
            datafile.read(sector, Info->BytesPerSector);
            datafile.seekg(offset_sectors + offset, std::fstream::beg);
            datafile.write(sector, Info->BytesPerSector);
        }
        delete[] sector;
    }
    writeHeader();
    writeBitmap();

    datafile.flush();
}

uint8_t Media::getBytesExponent() const {
    int e = 0;
    uint32_t test = 1;
    for (; test < Info->BytesPerSector; test <<= 1, e++);
    return e;
}


bool Media::isSectorBad(uint16_t sector) const {
    if ( !datafile.good() || sector >= getTotalSectors() ) {
        return true;
    }

    return badSectors[sector / 8] & (0x80 >> (sector % 8));
}

ERRORS Media::setSectorBad(uint16_t sector, bool state) {
    if ( !datafile.good() ) {
        return ERRORS::NO_MEDIA;
    }
    if ( sector >= getTotalSectors() ) {
        return ERRORS::BAD_SECTOR;
    }

    if (isSectorBad(sector) == state) {
        return ERRORS::NONE;
    }

    unsigned opt_sector_8 = sector / 8;

    if (state) {
        badSectors[opt_sector_8] |= 0x80 >> (sector % 8);
    }
    else {
        badSectors[opt_sector_8] &= ~( 0x80 >> (sector % 8) );
    }

    datafile.seekg(offset_bitmap + opt_sector_8, std::ios::beg);
    datafile.write(reinterpret_cast<char*>( &(badSectors[opt_sector_8]) ), 1);

    return ERRORS::NONE;
} // setSectorBad

ERRORS Media::readSector(uint16_t sector, std::vector<uint8_t>* data) {
    if ( !datafile.good() ) {
        return ERRORS::NO_MEDIA;
    }
    if ( sector >= getTotalSectors() || isSectorBad(sector) ) {
        return ERRORS::BAD_SECTOR;
    }
    size_t which_sector = offset_sectors + sector * Info->BytesPerSector;
#ifndef NDEBUG
        std::fprintf(stderr, "[DISK] Read at 0x%04zX\n", which_sector);
#endif

    datafile.seekg(which_sector, std::ios::beg);
    datafile.read( reinterpret_cast<char*>( data->data() ), data->size() );

    return ERRORS::NONE;
} // readSector

ERRORS Media::writeSector(uint16_t sector, std::vector<uint8_t>* data, bool dryRun) {
    if ( !datafile.good() ) {
        return ERRORS::NO_MEDIA;
    }
    if ( sector >= getTotalSectors() || isSectorBad(sector) ) {
        return ERRORS::BAD_SECTOR;
    }
    if (Info->writeProtect) {
        return ERRORS::PROTECTED;
    }
    size_t which_sector = offset_sectors + sector * Info->BytesPerSector;

    if (!dryRun) {
#ifndef NDEBUG
        std::fprintf(stderr, "[DISK] Write at 0x%04zX\n", which_sector);
#endif
        datafile.seekg(which_sector, std::ios::beg);
        datafile.write( reinterpret_cast<const char*>( data->data() ), data->size() );
        datafile.flush();
    }

    return ERRORS::NONE;
} // writeSector

ERRORS Media::writeSector(uint16_t sector, const uint8_t* data, size_t data_size, bool dryRun) {
    if ( !datafile.good() ) {
        return ERRORS::NO_MEDIA;
    }
    if ( sector >= getTotalSectors() || isSectorBad(sector) ) {
        return ERRORS::BAD_SECTOR;
    }
    if (Info->writeProtect) {
        return ERRORS::PROTECTED;
    }
    size_t which_sector = offset_sectors + sector * Info->BytesPerSector;
#ifndef NDEBUG
        std::fprintf(stderr, "[DISK] Write at 0x%04zX\n", which_sector);
#endif

    if (!dryRun) {
        datafile.seekg(which_sector, std::ios::beg);
        datafile.write( reinterpret_cast<const char*>( data ), data_size );
        datafile.flush();
    }

    return ERRORS::NONE;
} // writeSector

} // End of namespace computer
} // End of namespace trillek
