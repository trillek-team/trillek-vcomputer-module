/*!
 * \brief       Parameters parser for vm tool (main.cpp)
 * \file        vm_parser.hpp
 * \copyright   The MIT License (MIT)
 *
 * Parameters parser for vm tool (main.cpp)
 */
#ifndef __VM_PARSER_HPP_
#define __VM_PARSER_HPP_ 1

#include "types.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <vector>

enum class CpuToUse {
    TR3200,
//    DCPU16N
};

struct VmParamaters {

    VmParamaters (const int argc, const char** argv) : def_dsk_file("disk.dsk"), ram_size(128*1024), clock(100000), valid_params(true), ask_help(false)   {
        // Default values
        dsk_file = def_dsk_file;
        rom_file = nullptr;

        cpu = CpuToUse::TR3200;

        // Begin to parse
        for (int i = 1; i < argc; i++) {
            const char* arg = argv[i];

            if (arg[0] == '-') { // Parameter
                if (arg[1] == '\0') {
                    valid_params = false;
                    std::fprintf(stderr, "Invalid parameter %s\n", argv[i]);
                    break; // Invalid parameter
                }
                arg++;

                if (strncmp(arg, "r", 1) == 0 || strncmp(arg, "-rom", 4) == 0) {
                    // Rom file parameter
                    i++;
                    arg = argv[i];
                    if (i >= argc || arg[0] == '-') {
                        valid_params = false;
                        std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                        break;
                    }

                    rom_file = arg;

                } else if (strncmp(arg, "d", 1) == 0 || strncmp(arg, "-disk", 5) == 0) {
                    // Disk file parameter
                    i++;
                    arg = argv[i];
                    if (i >= argc || arg[0] == '-') {
                        valid_params = false;
                        std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                        break;
                    }

                    dsk_file = arg;

                } else if (strncmp(arg, "c", 1) == 0 || strncmp(arg, "-cpu", 4) == 0) {
                    // CPU type
                    i++;
                    arg = argv[i];
                    if (i >= argc || arg[0] == '-') {
                        valid_params = false;
                        std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                        break;
                    }

                    if (strncmp(arg, "tr3200", 6) == 0 || strncmp(arg, "TR3200", 6) == 0) {
                        cpu = CpuToUse::TR3200;
                    }/* else if (strncmp(arg, "DCPU-16N", 6) == 0 || strncmp(arg, "dcpu-16n", 6) == 0) {
                        cpu = CpuToUse::DCPU16N;
                    }*/

                } else if (strncmp(arg, "m", 1) == 0 ) {
                    // Total RAM
                    i++;
                    arg = argv[i];
                    if (i >= argc || arg[0] == '-') {
                        valid_params = false;
                        std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                        break;
                    }
                    auto ram = std::strtol(arg, nullptr, 0);
                    if (ram <= 0 || ram > 1024) {
                        std::fprintf(stderr, "Invalid value for parameter %s\nUsing 128KiB\n", argv[i-1]);
                        ram = 128*1024;
                    } else {
                        ram /= 128; // I hope the compile optimization don't trash this
                        ram *= 128;
                    }

                    ram_size = ram * 1024;
                } else if (strncmp(arg, "-clock", 6) == 0 ) {
                    // Clock speed
                    i++;
                    arg = argv[i];
                    if (i >= argc || arg[0] == '-') {
                        valid_params = false;
                        std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                        break;
                    }
                    auto clk = std::atoi(arg);
                    if (clk != 100 && clk != 250 && clk != 500 && clk != 1000) {
                        std::fprintf(stderr, "Invalid value for parameter %s\nUsing 100KHz\n", argv[i-1]);
                        clk = 1100;
                    }

                    clock = clk * 1000;

                } else if (strncmp(arg, "b", 1) == 0 ) {
                    // A breakpoint
                    i++;
                    arg = argv[i];
                    if (i >= argc || arg[0] == '-') {
                        valid_params = false;
                        std::fprintf(stderr, "Missing or invalid value for parameter %s\n", argv[i-1]);
                        break;
                    }
                    trillek::DWord addr = std::strtol(arg, nullptr, 0);
                    breaks.push_back(addr);

                } else if (strncmp(arg, "h", 1) == 0 || strncmp(arg, "-help", 5) == 0) {
                    // Asked for help
                    std::printf("Virtual Computer toy Emulator\n\n");
                    std::printf("Usage:\n");
                    std::printf("\t%s -r romfile [other parameters]\n\n", argv[0]);
                    std::printf("Parameters:\n");
                    std::printf("\t-r file or --rom file : RAW binary file for the ROM 32 KiB\n");
                    std::printf("\t-d file or --disk file : Disk file\n");
                    std::printf("\t-c val or --cpu val : Sets the CPU to use, from \"tr3200\" \n");
                    std::printf("\t-m val or --disk val : How many RAM have the computer in KiB. Must be > 128 and < 1024. Will be round to a multiple of 128\n");
                    std::printf("\t--clock val : CPU clock speed in Khz. Must be 100, 250, 500 or 1000.\n");
                    std::printf("\t-b val : Inserts a breakpoint at address val (could be hexadecimal or decimal).\n");
                    std::printf("\t-h or --help : Shows this help\n");

                    ask_help = true;
                }
            }
        }

        if (rom_file == nullptr) {
            valid_params = false;
            std::fprintf(stderr, "Missing ROM file\n");
        }
    }

    const char* def_dsk_file;

    const char* rom_file;           /// Path to ROM file
    const char* dsk_file;           /// Path to Floppy disk file
    unsigned ram_size;              /// Ram size in Bytes
    CpuToUse cpu;                   /// CPU to use (default TR3200)
    unsigned clock;                 /// CPU clock speed (default 100Khz)

    std::vector<trillek::DWord> breaks; /// List of breakpoints

    bool valid_params;              /// Parsed correctly all parameters
    bool ask_help;                  /// User asked by help
};

#endif // __VM_PARSER_HPP_

