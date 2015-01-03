/**
 * \brief       Enumeration and Control/status address Block
 * \file        enum_and_ctrl_blk.cpp
 * \copyright   The MIT License (MIT)
 *
 * An specialized Address Listener used by VComputer to implement
 * Enumeration and Control registers in a slot
 */

#include "vcomputer.hpp"
#include "enum_and_ctrl_blk.hpp"
#include "vs_fix.hpp"

#include <cassert>

namespace trillek {
namespace computer {

EnumAndCtrlBlk::EnumAndCtrlBlk (unsigned slot, Device* dev) :
    slot(slot), dev(dev) {
    assert (slot < MAX_N_DEVICES);
    assert (dev != nullptr);
}

Range EnumAndCtrlBlk::GetRange () const {
    DWord start = 0x110000 | (slot<<8);
    DWord end   = start + EnumCtrlBlkSize;

    return Range(start, end);
}

Byte EnumAndCtrlBlk::ReadB (DWord addr) {
    addr -= 0x110000 | (slot<<8);
    switch (addr) {
    // Enumeration stuff
    case 0:
        return 0xFF; // Presence byte
        break;

    case 1:
        return dev->DevType();
        break;

    case 2:
        return dev->DevSubType();
        break;

    case 3:
        return dev->DevID();
        break;

    case 4:
        return dev->DevVendorID();
        break;

    case 5:
        return dev->DevVendorID() >> 8;
        break;

    case 6:
        return dev->DevVendorID() >> 16;
        break;

    case 7:
        return dev->DevVendorID() >> 24;
        break;

    // Control and status stuff
    case 8:
        return 0;
        break;

    case 9:
        return 0;
        break;

    case 0x0A:
        return dev->A();
        break;

    case 0x0B:
        return dev->A() >> 8;
        break;

    case 0x0C:
        return dev->B();
        break;

    case 0x0D:
        return dev->B() >> 8;
        break;

    case 0x0E:
        return dev->C();
        break;

    case 0x0F:
        return dev->C() >> 8;
        break;

    case 0x10:
        return dev->D();
        break;

    case 0x11:
        return dev->D() >> 8;
        break;

    case 0x12:
        return dev->E();
        break;

    case 0x13:
        return dev->E() >> 8;
        break;

    default:
        return 0;
    } // switch
}     // ReadB

Word EnumAndCtrlBlk::ReadW (DWord addr) {
    return this->ReadB(addr) | (this->ReadB(addr+1) << 8);
    // TODO Improve this in the aligned cases
}

DWord EnumAndCtrlBlk::ReadDW (DWord addr) {
    return this->ReadW(addr) | (this->ReadW(addr+2) << 16);
}

void EnumAndCtrlBlk::WriteB (DWord addr, Byte val) {
    addr -= 0x110000 | (slot<<8);
    switch (addr) {
    // Control and status stuff
    // NOTE: Only the MSB byte write send the command as wll be usually the
    // last write value
    case 8: // Cmd
        cmd = (cmd & 0xFF00) | val;
        //dev->SendCMD(cmd);
        break;

    case 9:
        cmd = (cmd & 0x00FF) | (val << 8);
        dev->SendCMD(cmd);
        break;

    case 0x0A: // A reg
        a = (a & 0xFF00) | val;
        //dev->A(a);
        break;

    case 0x0B:
        a = (a & 0x00FF) | (val << 8);
        dev->A(a);
        break;

    case 0x0C: // B reg
        b = (b & 0xFF00) | val;
        //dev->B(b);
        break;

    case 0x0D:
        b = (b & 0x00FF) | (val << 8);
        dev->B(b);
        break;

    case 0x0E: // C reg
        c = (c & 0xFF00) | val;
        //dev->C(c);
        break;

    case 0x0F:
        c = (c & 0x00FF) | (val << 8);
        dev->C(c);
        break;

    case 0x10: // D reg
        d = (d & 0xFF00) | val;
        //dev->D(d);
        break;

    case 0x11:
        d = (d & 0x00FF) | (val << 8);
        dev->D(d);
        break;

    case 0x12: // E reg
        e = (e & 0xFF00) | val;
        //dev->E(e);
        break;

    case 0x13:
        e = (e & 0x00FF) | (val << 8);
        dev->E(e);
        break;

    default:
        break;
    } // switch
}     // WriteB

void EnumAndCtrlBlk::WriteW (DWord addr, Word val) {
    switch (addr) {
    // Control and status stuff
    case 8: // Cmd
        cmd = val;
        dev->SendCMD(val);
        break;

    case 0x0A: // A reg
        a = val;
        dev->A(val);
        break;

    case 0x0C: // B reg
        b = val;
        dev->B(val);
        break;

    case 0x0E: // C reg
        c = val;
        dev->C(val);
        break;

    case 0x10: // D reg
        d = val;
        dev->D(val);
        break;

    case 0x12: // E reg
        e = val;
        dev->E(val);
        break;

    default:
        this->WriteB(addr, val);
        this->WriteB(addr+1, val >> 8);
    } // switch
}     // WriteW

void EnumAndCtrlBlk::WriteDW (DWord addr, DWord val) {
    this->WriteW(addr, val);
    this->WriteW(addr+2, val >> 16);
}

} // End of namespace computer
} // End of namespace trillek
