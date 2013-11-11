#include "CDA.hpp"


namespace vm {
namespace cda {

#if (BYTE_ORDER != LITTLE_ENDIAN)
// Big Endian -> RGB
const dword_t CDA::pallete[] = {    
    0x000000, // Black
    0x0000CD, // Dark Blue
    0x00CD00, // Dark Green
    0x00CDCD, // Dark Cyan
    0xCD0000, // Dark Red
    0xCD00CD, // Dark Magenta
    0xAA5500, // Brown
    0xCDCDCD, // Light Gray
    // Bright colors
    0x555555, // Dark Grey
    0x0000FF, // Blue
    0x00FF00, // Green
    0x00FFFF, // Cyan
    0xFF0000, // Red
    0xFF00FF, // Magenta
    0xFFFF00, // Yellow
    0xFFFFFF, // White
};
#else
// Little Endian -> BGR
const dword_t CDA::pallete[] = {    
    0x000000, // Black
    0xCD0000, // Dark Blue
    0x00CD00, // Dark Green
    0xCDCD00, // Dark Cyan
    0x0000CD, // Dark Red
    0xCD00CD, // Dark Magenta
    0x0055AA, // Brown
    0xCDCDCD, // Light Gray
    // Bright colors
    0x555555, // Dark Grey
    0xFF0000, // Blue
    0x00FF00, // Green
    0xFFFF00, // Cyan
    0x0000FF, // Red
    0xFF00FF, // Magenta
    0x00FFFF, // Yellow
    0xFFFFFF, // White
};
#endif

} // End of namespace cda
} // End of namespace vm

