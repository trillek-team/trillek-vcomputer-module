#pragma once
/**
 * Trillek Virtual Computer - AddrListener.hpp
 * Defines an interface for Address Listeners
 */

#include "Types.hpp"

#include <cassert>

namespace vm {

  /**
   * Interface for a Address Listener
   */
  class AddrListener {
    public:
      virtual ~AddrListener() { }

      virtual byte_t ReadB (dword_t addr) = 0;
      virtual word_t ReadW (dword_t addr) = 0;
      virtual dword_t ReadDW (dword_t addr) = 0;

      virtual void WriteB (dword_t addr, byte_t val) = 0;
      virtual void WriteW (dword_t addr, word_t val) = 0;
      virtual void WriteDW (dword_t addr, dword_t val) = 0;

  };

  /**
   * Range of 24bit addresses/address
   * Used to store/search an AddrListener stored in a tree
   */
  struct Range {
    dword_t start;
    dword_t end;

    /**
     * Build a Range for listening/search a single address
     * Must be a 24 bit address
     * @param addr Address
     */
    Range (dword_t addr) : start(addr), end(addr) {
      assert(addr <= 0xFFFFFF);
    }

    /**
     * Build a Range for listening a range of addresses.
     * Must be a 24 bit address and start <= end
     * @param start Begin address
     * @param end End address
     */
    Range (dword_t start, dword_t end) : start(start & 0xFFFFFF), end(end & 0xFFFFFF) {
      assert (start <= end);
      assert (end <= 0xFFFFFF);
    }

    /**
     * Comparation operator required by std::map
     * We forbid overlaping ranges, so comparing two ranges for weak ordering is easy
     */
    bool operator<(const Range& other) const {
      return (end < other.start);
    }

  };


} // End of namespace vm

