/**
 * \brief       Virtual Computer Random No Volatile RAM (NVRAM)
 * \file        nvram.hpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of NVRAM
 */
#ifndef __NVRAM_HPP_
#define __NVRAM_HPP_ 1

#include "../types.hpp"
#include "../addr_listener.hpp"

#include <ostream>
#include <istream>

namespace trillek {
namespace computer {

class NVRAM : public AddrListener {
public:

    NVRAM();
    virtual ~NVRAM();

    virtual Byte ReadB (DWord addr);
    virtual Word ReadW (DWord addr);
    virtual DWord ReadDW (DWord addr);

    virtual void WriteB (DWord addr, Byte val);
    virtual void WriteW (DWord addr, Word val);
    virtual void WriteDW (DWord addr, DWord val);

    void Reset ();

    /**
     * Fills NVRAM with data from a input stream
     * \param stream Stream were to read the data
     * \return True if read data from the strean
     */
    bool Load (std::istream& stream);

    /**
     * Saves NVRAM data to a output stream
     * \param stream Stream were to write the data
     * \return True if writed data to the stream
     */
    bool Save (std::ostream& stream);

    const static DWord BaseAddress = 0x11F000;
private:

    Byte eprom[256];

};

} // End of namespace computer
} // End of namespace trillek



#endif // __NVRAM_HPP_
