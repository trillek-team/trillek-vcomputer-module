/*!
 * \brief       Creates a floppy disk file
 * \file        makedisk.cpp
 * \copyright   LGPL v3
 *
 * Creates a floppy disk file and inserts a binary file on it
 */

#include "devices/media.hpp"
#include "auxiliar.hpp"

#include <fstream>
#include <iostream>
#include <cstdio>
#include <memory>
#include <string>

trillek::computer::DiskDescriptor diskdescriptor;

const char* help = "makedisk\n\n"
                   "Usage:\n\tmakedisk -o outputfile [other parameters]\n\n"
                   "Parameters:\n"
                   "\t-o file : Output file\n"
                   "\t-i file : RAW binary file to be included on the floppy disk image\n"
                   "\t-s sectors : Sectors per track (8 to 36)\n"
                   "\t-t tracks : Tracks per side (between 40 and 80)\n"
                   "\t-1 : Creates a floppy with only a sigle side\n"
                   "\t-160 : Creates a 160KiB disk with a single side, 8 sectors and 40 tracks.\n"
                   "\t-320 : Creates a 320KiB disk with 8 sectors and 40 tracks. Default floppy size\n"
                   "\t-360 : Creates a 360KiB disk with 9 sectors and 40 tracks.\n"
                   "\t-640 : Creates a 640KiB disk with 8 sectors and 80 tracks.\n"
                   "\t-720 : Creates a 720KiB disk with 9 sectors and 80 tracks.\n"
                   "\t-1200 : Creates a 1200KiB disk with 15 sectors and 80 tracks.\n"
                   "\t-h : Shows this help\n";

int main(int argc, char* argv[]) {
    using namespace trillek;
    using namespace trillek::computer;
    
    if (argc <= 1) {
        std::fprintf(stderr, "Invalid number of parameters.\n");
        std::printf("%s", help);
    }
    
    // Data
    const char* outfile = nullptr;
    const char* infile  = nullptr;
    
    // Disk Descriptor - A typical 320KiB disk
    diskdescriptor.TypeDisk = DiskType::FLOPPY;
    diskdescriptor.writeProtect = false;
    diskdescriptor.NumSides = 2;
    diskdescriptor.TracksPerSide = 40;
    diskdescriptor.SectorsPerTrack = 8;
    diskdescriptor.BytesPerSector = 512;
   
    // Check parameters
    for (int i=1; i< argc; i++) {
        const char* arg = argv[i];
        if (arg[0] == '-') { // Parameter
            if (arg[1] == '\0') {
                std::fprintf(stderr, "Invalid parameter %s\n", argv[i]);
                return -1; // Invalid parameter
            }
            arg++;
            
            if (strncmp(arg, "h", 1) == 0 ) {
                // Show help
                std::printf("%s", help);
                return 0;
            } else if (strncmp(arg, "o", 1) == 0 ) {
                // Output file parameter
                i++;
                arg = argv[i];
                if (i >= argc || arg[0] == '-') {
                    std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                    return -1;
                }
                
                outfile = arg;
            } else if (strncmp(arg, "i", 1) == 0 ) {
                // Input raw file parameter
                i++;
                arg = argv[i];
                if (i >= argc || arg[0] == '-') {
                    std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                    return -1;
                }
                
                infile = arg;
            } else if (strncmp(arg, "s", 1) == 0 ) {
                // Sectors parameter
                i++;
                arg = argv[i];
                
                if (i >= argc || arg[0] == '-') {
                    std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                    return -1;
                }
                long tmp = std::strtol(arg, nullptr, 0);
                if (tmp <= 0 || tmp > 36) {
                    std::fprintf(stderr, "Invalid number of sectors per track\n");
                    return -1;
                }
                diskdescriptor.SectorsPerTrack = (uint8_t)tmp;

            } else if (strncmp(arg, "t", 1) == 0 ) {
                // Tracks parameter
                i++;
                arg = argv[i];
                
                if (i >= argc || arg[0] == '-') {
                    std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                    return -1;
                }
                long tmp = std::strtol(arg, nullptr, 0);
                if (tmp <= 0 || tmp > 80) {
                    std::fprintf(stderr, "Invalid number of tracks\n");
                    return -1;
                }
                diskdescriptor.TracksPerSide = (uint8_t)tmp;
                
            } else if (strncmp(arg, "1", 1) == 0 ) {
                // Single side
                i++;
                diskdescriptor.NumSides = 1;
                
            } else if (strncmp(arg, "160", 3) == 0 ) {
                // 160KiB disk
                i++;
                diskdescriptor.NumSides = 1;
                diskdescriptor.TracksPerSide = 40;
                diskdescriptor.SectorsPerTrack = 8;
                
            } else if (strncmp(arg, "320", 3) == 0 ) {
                // 160KiB disk
                i++;
                diskdescriptor.NumSides = 2;
                diskdescriptor.TracksPerSide = 40;
                diskdescriptor.SectorsPerTrack = 8;
                
            } else if (strncmp(arg, "360", 3) == 0 ) {
                // 160KiB disk
                i++;
                diskdescriptor.NumSides = 2;
                diskdescriptor.TracksPerSide = 40;
                diskdescriptor.SectorsPerTrack = 9;
                
            } else if (strncmp(arg, "640", 3) == 0 ) {
                // 160KiB disk
                i++;
                diskdescriptor.NumSides = 2;
                diskdescriptor.TracksPerSide = 80;
                diskdescriptor.SectorsPerTrack = 8;
                
            } else if (strncmp(arg, "720", 3) == 0 ) {
                // 160KiB disk
                i++;
                diskdescriptor.NumSides = 2;
                diskdescriptor.TracksPerSide = 80;
                diskdescriptor.SectorsPerTrack = 9;
                
            } else if (strncmp(arg, "1200", 4) == 0 ) {
                // 160KiB disk
                i++;
                diskdescriptor.NumSides = 2;
                diskdescriptor.TracksPerSide = 80;
                diskdescriptor.SectorsPerTrack = 15;
            }
        }
    }
    
    if (outfile == nullptr) {
        std::fprintf(stderr, "Missing output file.\n");
        return -1;
    }
    // Create a disk using the data 
    Media fd(outfile, diskdescriptor);
    
    try {
        std::fstream f(infile, std::ios::in | std::ios::binary);
        if ( !f.is_open() ) {
            return -1;
        }
        std::streamoff size;
        
        auto begin = f.tellg();
        f.seekg (0, std::ios::end);
        auto end = f.tellg();
        f.seekg (0, std::ios::beg);
        
        size = end - begin;
        std::printf("RAW file of %zu bytes. ", size );
        // Calculate the efective top sector of the disk. 
        size_t top_sector = (size / 512) +1;
        top_sector = top_sector < fd.getTotalSectors() ? top_sector : fd.getTotalSectors();
        std::printf("Total Sectors %zu (%zu bytes).\n", top_sector, top_sector*512 );
        
        Byte sector[512];
        for (size_t s=0; s < top_sector; s++) {
            f.read(reinterpret_cast<char*>(sector), 512);
            ERRORS err = fd.writeSector(s, sector, 512);
            if (err != ERRORS::NONE) {
                std::fprintf(stderr, "Error reading RAW binary file or writing data to disk file\n");
                return -1;
            }
        }
        
    }
    catch (...) {
        std::fprintf(stderr, "Error reading RAW binary file or writing data to disk file\n");
        return -1;
    }
    
    return 0;
}