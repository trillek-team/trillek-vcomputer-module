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
    return (track * descriptor.NumSides + head)* descriptor.SectorsPerTrack + sector -1;
}

Media::Media(const std::string& filename) : HEADER_VERSION(1) {

    // Check if file exists
    datafile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if ( !datafile.good() ) {
#ifndef NDEBUG
        std::cout << "[DISK] File could not be opened: " << filename.c_str() << std::endl;
#endif
        datafile.close();
        return;
    }

    char temp[3];
    datafile.read(temp, 3);
    if (std::memcmp(temp, HEADER_MAGIC, 3) != 0) {
#ifndef NDEBUG
        std::cout << "[DISK] File not a disk image: " << filename.c_str() << std::endl;
#endif
        datafile.close();
        return;
    }

    datafile.read(temp, 1);
    if (temp[0] != HEADER_VERSION) {
#ifndef NDEBUG
        std::cout << "[DISK] File is wrong version: " << filename.c_str() << std::endl;
#endif
        datafile.close();
        return;
    }

    Info.reset(new DiskDescriptor);

    /* Read meta info from disk */
    datafile.read(reinterpret_cast<char*>(&Info->TypeDisk), 1);
    datafile.read(reinterpret_cast<char*>(&Info->writeProtect), 1);
    datafile.read(reinterpret_cast<char*>(&Info->NumSides), 1);
    datafile.read(reinterpret_cast<char*>(&Info->TracksPerSide), 1);
    datafile.read(reinterpret_cast<char*>(&Info->SectorsPerTrack), 1);
    datafile.read(reinterpret_cast<char*>(&Info->BytesPerSector), 2);

    /* Get bad sector bitmap from the file */
    int bitmapSize = getTotalSectors() / 8;
    bitmapSize += getTotalSectors() % 8 != 0 ? 1 : 0;
    badSectors  = std::vector<uint8_t>(bitmapSize);

    datafile.seekg(HEADER_SIZE + getTotalSectors() * Info->BytesPerSector, std::fstream::beg);
    datafile.read( reinterpret_cast<char*>( badSectors.data() ), badSectors.size() );

#ifndef NDEBUG
    std::cout << "[DISK] File loaded: " << filename.c_str() << std::endl;
#endif
}

Media::Media(const std::string& filename, DiskDescriptor* info)  : HEADER_VERSION(1) {

    Info.reset(info);

    // create new file
#ifndef NDEBUG
    std::cout << "[DISK] Creating file: " << filename.c_str() << std::endl;
#endif

    datafile.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    /* write file header */
    datafile.write(HEADER_MAGIC, 3);
    datafile.write(&HEADER_VERSION, 1);
    datafile.write(reinterpret_cast<char*>(&Info->TypeDisk), 1);
    datafile.write(reinterpret_cast<char*>(&Info->writeProtect), 1);
    datafile.write(reinterpret_cast<char*>(&Info->NumSides), 1);
    datafile.write(reinterpret_cast<char*>(&Info->TracksPerSide), 1);
    datafile.write(reinterpret_cast<char*>(&Info->SectorsPerTrack), 1);
    datafile.write(reinterpret_cast<char*>(&Info->BytesPerSector), 2);

    char* sector = new char[Info->BytesPerSector];
    std::memset(sector, 0, Info->BytesPerSector);
    datafile.seekg(HEADER_SIZE, std::fstream::beg);
    for (uint16_t i = 0; i < getTotalSectors(); i++) {
        datafile.write(sector, Info->BytesPerSector);
    }
    delete[] sector;

    /* Get bad sector bitmap from the file */
    int bitmapSize = getTotalSectors() / 8;
    bitmapSize += getTotalSectors() % 8 != 0 ? 1 : 0;
    badSectors  = std::vector<uint8_t>(bitmapSize);
    badSectors.assign(badSectors.size(), 0);

    datafile.seekg(HEADER_SIZE + getTotalSectors() * Info->BytesPerSector, std::fstream::beg);
    datafile.write( reinterpret_cast<char*>( badSectors.data() ), badSectors.size() );

    datafile.flush();
}

Media::Media(const std::string& filename, const DiskDescriptor& info)  : HEADER_VERSION(1) {
    DiskDescriptor* tmpInfo = new DiskDescriptor();
    std::memmove(tmpInfo, &info, sizeof(DiskDescriptor));
    Info.reset(tmpInfo);

    // create new file
#ifndef NDEBUG
    std::cout << "[DISK] Creating file: " << filename.c_str() << std::endl;
#endif

    datafile.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    /* write file header */
    datafile.write(HEADER_MAGIC, 3);
    datafile.write(&HEADER_VERSION, 1);
    datafile.write(reinterpret_cast<char*>(&Info->TypeDisk), 1);
    datafile.write(reinterpret_cast<char*>(&Info->writeProtect), 1);
    datafile.write(reinterpret_cast<char*>(&Info->NumSides), 1);
    datafile.write(reinterpret_cast<char*>(&Info->TracksPerSide), 1);
    datafile.write(reinterpret_cast<char*>(&Info->SectorsPerTrack), 1);
    datafile.write(reinterpret_cast<char*>(&Info->BytesPerSector), 2);

    char* sector = new char[Info->BytesPerSector];
    std::memset(sector, 0, Info->BytesPerSector);
    datafile.seekg(HEADER_SIZE, std::fstream::beg);
    for (uint16_t i = 0; i < getTotalSectors(); i++) {
        datafile.write(sector, Info->BytesPerSector);
    }
    delete[] sector;

    /* Get bad sector bitmap from the file */
    int bitmapSize = getTotalSectors() / 8;
    bitmapSize += getTotalSectors() % 8 != 0 ? 1 : 0;
    badSectors  = std::vector<uint8_t>(bitmapSize);
    badSectors.assign(badSectors.size(), 0);

    datafile.seekg(HEADER_SIZE + getTotalSectors() * Info->BytesPerSector, std::fstream::beg);
    datafile.write( reinterpret_cast<char*>( badSectors.data() ), badSectors.size() );

    datafile.flush();
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

uint8_t Media::getBytesExponent() const {
    return log2(Info->BytesPerSector);
}


bool Media::isSectorBad(uint16_t sector) const {
    if ( !datafile.good() || sector >= getTotalSectors() ) {
        return true;
    }

    return badSectors[sector / 8] & 128 >> (sector % 8);
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
        badSectors[opt_sector_8] |= 128 >> (sector % 8);
    }
    else {
        badSectors[opt_sector_8] &= ~( 128 >> (sector % 8) );
    }

    datafile.seekg(HEADER_SIZE + getTotalSectors() * Info->BytesPerSector + opt_sector_8, std::ios::beg);
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
#ifndef NDEBUG
        std::fprintf(stderr, "[DISK] Read at 0x%04X\n",(HEADER_SIZE + sector * Info->BytesPerSector) );
#endif

    datafile.seekg(HEADER_SIZE + sector * Info->BytesPerSector, std::ios::beg);
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

    if (!dryRun) {
#ifndef NDEBUG
        std::fprintf(stderr, "[DISK] Write at 0x%04X\n",(HEADER_SIZE + sector * Info->BytesPerSector) );
#endif
        datafile.seekg(HEADER_SIZE + sector * Info->BytesPerSector, std::ios::beg);
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
#ifndef NDEBUG
        std::fprintf(stderr, "[DISK] Write at 0x%04X\n",(HEADER_SIZE + sector * Info->BytesPerSector) );
#endif

    if (!dryRun) {
        datafile.seekg(HEADER_SIZE + sector * Info->BytesPerSector, std::ios::beg);
        datafile.write( reinterpret_cast<const char*>( data ), data_size );
        datafile.flush();
    }

    return ERRORS::NONE;
} // writeSector

} // End of namespace computer
} // End of namespace trillek
