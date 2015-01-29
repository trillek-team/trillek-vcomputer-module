/**//**************************************************************************
 *   PBM to TDA font tool
 *   This program reads a plain text PBM image file, and generates a 8x8 font
 *   from it.
 *   It outputs in .DAT format or in a list of HEX values
 *
 *   For more information about PBM file format, see:
 *      http://en.wikipedia.org/wiki/Netpbm_format#PBM_example
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <cstdint>

#include "types.hpp"

using namespace trillek;

enum OUTPUT_FORMAT {
    DAT,
    HEX_DUMP,
};

void print_help(std::string program_name)
{
    using namespace std;
    cout << "pbm2font\n";
    cout << "usage : " << program_name << " [-options] <input-file>\n";
    cout << "--------------------------------------------------------\n";
    cout << "  options:" << endl;
    cout << "    -output <filename> (-o) : output filename\n";
    cout << "    -format=<output_format> : use the following format\n";
    cout << "            dat -> Uses universal .dat output format\n";
    cout << "            hex_dump -> Generates a hexadecimal dump\n";
    cout << "            By defaults, the ouput format is \"dat\"\n";
}

int main (int argc, char **argv)
{

    OUTPUT_FORMAT format = OUTPUT_FORMAT::DAT;
    std::string filename;
    std::ifstream fin;
    std::ofstream fout;
    bool use_fout = false;

    //TODO make a function that parse argument into a program struct
    for (int k=1; k < argc; k++) { //parse arguments
        if (argv[k][0] == '-') {
            std::string opt = argv[k];
            std::cerr << opt << '\n';

            if (opt=="--help"||opt=="-help"||opt=="-h") {
                std::string pn = argv[0];
                pn.erase(0,pn.find_last_of('\\')+1); //windows
                pn.erase(0,pn.find_last_of('/')+1); //linux
                print_help(pn);
                return 0;
            } else if ((opt == "-output" || opt == "-o") && argc > k+1) {
                if (fout.is_open())
                    fout.close(); // Safeguard

                fout.open(argv[++k]);
                use_fout = fout.is_open();
            } else if  (opt == "-output" || opt == "-o") {
              std::cerr  << "Option " + opt +
                        " requiered another argument it will be ignored here" << std::endl;
            } else if (opt == "-format=dat" ) {
                format = OUTPUT_FORMAT::DAT;
            } else if (opt == "-format=hex_dump") {
                format = OUTPUT_FORMAT::HEX_DUMP;
            } else {
              std::cerr << "Unknown option " + opt + " it will be ignored !" << std::endl;
            }
        } else {
            filename = argv[k];
        }
    }
    if (filename.size() <= 0) {
      std::cerr << "Missing or invalid input filename" << std::endl;
      return -1;
    }

    fin.open(filename);
    if (! fin.is_open()) {
        std::cerr << "Error opening input file " + filename << std::endl;
        return -1;
    }

    if (use_fout) {
        fout << std::hex;
    } else {
        std::cout << std::hex;
    }

    // Read the PBM file
    std::string str;
    std::getline(fin, str);

    // Check header
    if (str.size() <= 0 || str.substr(0, 2) != "P1") {
        std::cerr << "Invalid input file. Must be a ASCII PBM file. Aborting." << std::endl;
        return -1;
    }

    long width = -1;
    long height = -1;

    size_t num_glyphs = 0;

    bool framebuffer = false;
    while (std::getline(fin, str)) { // Read frambeuffer size
        // Remove comments
        auto pos = str.find("#");
        if (pos != std::string::npos) {
            str.resize(pos);
        }

        if (str.size() <= 0)
            continue;

        // Reads framebuffer size
        pos = str.find(" ");
        if (pos == std::string::npos)
            continue; // Not valid line

        width = std::atol(str.substr(0, pos).c_str());
        height = std::atol(str.substr(pos).c_str());

        // Calcs the number of glyphs (8x8 font)
        num_glyphs = (width / 8) * (height / 8);

        framebuffer = true;
        break;
    }

    if (!framebuffer) {
        std::cerr << "Invalid file format. Missing framebuffer size" << std::endl;
        return -1;
    }

    // reads the frame buffer itself
    //
    const unsigned glyphs_row = width/8; // Number of glyphs per row in the framebuffer
    const unsigned num_bytes = 8*num_glyphs;

    // We store her the glyphs
	Byte* glyphs = new Byte[num_bytes]({ 0 });

    std::cerr << "PBM file of X: " << width << " Y: " << height << std::endl;
    std::cerr << "Num of glyphs: " << num_glyphs << std::endl;

    char c;
    bool comment = false;
    unsigned x = 0;
    unsigned y = 0;
    unsigned addr = 0;
    unsigned glyph = 0;
    // TODO Check that this is working as it must be
    while ((c = fin.get()) != std::char_traits<char>::eof() ) {
      using namespace std;
      if (c == '\r' || c == '\n' ) {
        comment = false;
        continue;
      } else if (c == '#') {
        comment = true;
        continue;
      } else if (comment || ! isalnum(c)) {
        continue;
      }

      glyph = (x/8) + glyphs_row * (y/8);
      addr = glyph*8 + y%8;
      glyphs[addr] |= ((c == '1') << (7-(x%8)));

      if (++x >= glyphs_row *8) {
        x = 0;
        y++;
      }

    }

    // Output
    for (unsigned i=0; i< num_glyphs*8; i++) {
      if (format == OUTPUT_FORMAT::DAT) { // DAT output
        if (use_fout) {
          fout << "dat 0x" << std::setfill('0') << std::setw(2) << (int)glyphs[i] << std::endl;
        } else {
          std::cout << "dat 0x" << std::setfill('0') << std::setw(2) << (int)glyphs[i] << std::endl;
        }

      } else {                            // HEX output
        if (use_fout) {
          if (i % 8 == 0) { // Puts address
            fout << std::setw(0) << "0x";
            fout << std::setfill('0') << std::setw(4);
            fout << (int)i << ":";
            fout << std::setw(0);
          }
          fout << " ";
          fout << std::setfill('0') << std::setw(2);
          fout << (int)glyphs[i];
          if (i % 8 == 7 || i+1 == num_glyphs*8)
              fout << std::endl;
        } else {
          if (i % 8 == 0) {
            std::cout << std::setw(0) << "0x";
            std::cout << std::setfill('0') << std::setw(4);
            std::cout << (int)i << ":";
            std::cout << std::setw(0);
          }
          std::cout << " ";
          std::cout << std::setfill('0') << std::setw(2);
          std::cout << (int)glyphs[i];
          if (i % 8 == 7 || i+1 == num_glyphs*8)
              std::cout << std::endl;
        }
      }
    }

    delete[] glyphs;
    return 0;
}
