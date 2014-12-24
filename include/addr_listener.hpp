/**
 * \brief       Defines an interface for Address Listeners
 * \file        AddrListener.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __ADDRLISTENER_HPP_
#define __ADDRLISTENER_HPP_ 1

#include "types.hpp"
#include "vc_dll.hpp"

#include <cassert>

namespace trillek {
namespace computer {

/**
 * Interface for a Address Listener
 */
class DECLDIR AddrListener {
public:

    virtual ~AddrListener() {
    }

    virtual Byte ReadB (DWord addr)   = 0;
    virtual Word ReadW (DWord addr)   = 0;
    virtual DWord ReadDW (DWord addr) = 0;

    virtual void WriteB (DWord addr, Byte val)   = 0;
    virtual void WriteW (DWord addr, Word val)   = 0;
    virtual void WriteDW (DWord addr, DWord val) = 0;
};

/**
 * Range of 24bit addresses/address
 * Used to store/search an AddrListener stored in a tree
 */
struct DECLDIR Range {
    DWord start;
    DWord end;

    /**
     * Build a Range for listening/search a single address
     * Must be a 24 bit address
     * @param addr Address
     */
    Range (DWord addr) : start(addr), end(addr) {
        assert(addr <= 0xFFFFFF);
    }

    /**
     * Build a Range for listening a range of addresses.
     * Must be a 24 bit address and start <= end
     * @param start Begin address
     * @param end End address
     */
    Range (DWord start, DWord end) : start(start & 0xFFFFFF), end(end & 0xFFFFFF) {
        assert (start <= end);
        assert (end <= 0xFFFFFF);
    }

    /**
     * Comparation operator required by std::map
     * We forbid overlaping ranges, so comparing two ranges for weak ordering is
     **easy
     */
    bool operator<(const Range& other) const {
        return (end < other.start);
    }
};

} // End of namespace computer
} // End of namespace trillek

#endif // __ADDRLISTENER_HPP_
