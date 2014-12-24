/**
 * \brief       Enumeration and Control/status address Block
 * \file        enum_and_ctrl_blk.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __ENUMANDCTROLBLK_HPP_
#define __ENUMANDCTROLBLK_HPP_ 1

#include "types.hpp"
#include "vc_dll.hpp"

#include "device.hpp"
#include "addr_listener.hpp"

#include <tuple>
#include <memory>

namespace trillek {
namespace computer {

/**
 * An specialized Address Listener used by VComputer to implement
 * Enumeration and Control registers in a slot
 */
class DECLDIR EnumAndCtrlBlk : public AddrListener {
public:

    /**
     * Builds the Enumeartion and Control register block for a device plugged
     * in slot XX
     */
    EnumAndCtrlBlk (unsigned slot, IDevice* dev);

    virtual ~EnumAndCtrlBlk () {
    }

    /**
     * Returns the address Range used by this register block
     */
    Range GetRange () const;

    Byte ReadB (DWord addr);
    Word ReadW (DWord addr);
    DWord ReadDW (DWord addr);

    void WriteB (DWord addr, Byte val);
    void WriteW (DWord addr, Word val);
    void WriteDW (DWord addr, DWord val);

private:

    unsigned slot; /// Slot number
    IDevice* dev;  /// Ptr to the device

    DWord cmd; /// Buffer used when a byte write hapens in CMD
    DWord a;   /// Buffer used when a byte write hapens in A
    DWord b;   /// Buffer used when a byte write hapens in B
    DWord c;   /// Buffer used when a byte write hapens in C
    DWord d;   /// Buffer used when a byte write hapens in D
    DWord e;   /// Buffer used when a byte write hapens in E
};

typedef std::tuple<std::shared_ptr<IDevice>, EnumAndCtrlBlk*, int32_t> device_t; /// Storage of a device

} // End of namespace computer
} // End of namespace trillek

#endif // __ENUMANDCTROLBLK_HPP_
