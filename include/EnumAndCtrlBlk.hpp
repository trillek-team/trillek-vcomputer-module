#pragma once
/**
 * Trillek Virtual Computer - EnumAndCtrlBlk.hpp
 * Enumeration and Control/status address Block
 */

#include "Types.hpp"

#include "IDevice.hpp"
#include "AddrListener.hpp"

#include <tuple>
#include <memory>

namespace vm {

  /**
   * An specialized Address Listener used by VComputer to implement
   * Enumeration and Control registers in a slot
   */
  class EnumAndCtrlBlk : public AddrListener {
    public:
      /**
       * Builds the Enumeartion and Control register block for a device plugged
       * in slot XX
       */
      EnumAndCtrlBlk (unsigned slot, IDevice* dev);

      virtual ~EnumAndCtrlBlk () { }


      /**
       * Returns the address Range used by this register block
       */
      Range GetRange () const;

      byte_t ReadB (dword_t addr);
      word_t ReadW (dword_t addr);
      dword_t ReadDW (dword_t addr);

      void WriteB (dword_t addr, byte_t val);
      void WriteW (dword_t addr, word_t val);
      void WriteDW (dword_t addr, dword_t val);

    private:
      unsigned slot;  /// Slot number
      IDevice* dev;   /// Ptr to the device

      dword_t cmd;    /// Buffer used when a byte write hapens in CMD
      dword_t a;      /// Buffer used when a byte write hapens in A
      dword_t b;      /// Buffer used when a byte write hapens in B
      dword_t c;      /// Buffer used when a byte write hapens in C
      dword_t d;      /// Buffer used when a byte write hapens in D
      dword_t e;      /// Buffer used when a byte write hapens in E
  };

  typedef std::tuple<std::shared_ptr<IDevice>, EnumAndCtrlBlk*, int32_t> device_t;    /// Storage of a device

} // End of namespace vm

