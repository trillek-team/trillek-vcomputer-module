/**
 * Trillek Virtual Computer - M5FDD.cpp
 * Mackapar 5.25" Floppy Drive
 */

#include "M5FDD.hpp"

namespace vm {
namespace dev {
namespace m5fdd {

M5FDD::M5FDD() : state(STATE_CODES::NO_MEDIA), error(ERROR_CODES::NONE),
    busyCycles(0), pendingInterrupt(false) {
    a = b = c = d = 0;
}

M5FDD::~M5FDD() {
    if (floppy) {
        ejectFloppy();
    }
}

void M5FDD::Reset() {
    std::cout << "[M5FDD] Device reset!" << std::endl;

/* The disk should only be ejected by calling ejectDisk() on the parent drive of
 * the disk.
 * Reset() is a power cycle, not a state reset.
 * Calling reset() while writing to the disk will not corrupt any sector or the
 **VCD file.
 *
 *              if (floppy)
 *                  ejectFloppy();
 */
    busyCycles       = a = b = c = d = 0;
    pendingInterrupt = false;
    state            = floppy ==
                       NULL ? STATE_CODES::NO_MEDIA : floppy->isProtected() ? STATE_CODES::READY_WP : STATE_CODES::
                       READY;
    error = floppy == NULL ? ERROR_CODES::NO_MEDIA : ERROR_CODES::NONE;
} // Reset

bool M5FDD::DoesInterrupt(uint16_t& msg) {
    if (this->msg != 0 && pendingInterrupt) {
        msg              = this->msg;
        pendingInterrupt = false;
        return true;
    }

    pendingInterrupt = false;
    return false;
}

void M5FDD::SendCMD(word_t cmd) {
    if (vcomp == NULL) {
        return;
    }

    switch ( static_cast<COMMANDS>(cmd) ) {

    case COMMANDS::SET_INTERRUPT:
        msg = a;
        std::cout << "[M5FDD] msg set to " << msg << std::endl;
        break;

    case COMMANDS::READ_SECTOR:

        if (state == STATE_CODES::READY || state == STATE_CODES::READY_WP) {
            if (!floppy) {
                // This not should happen never, but...
                state = STATE_CODES::NO_MEDIA;
                error = ERROR_CODES::BROKEN;
                std::cout << "[M5FDD] COMMANDS::READ_SECTOR, STATE_CODES::READY, but no floppy class!" << std::endl;
                break;
            }
            // read the sector
            vm::dev::disk::ERRORS diskError = floppy->readSector(c, &sectorBuffer);
            error = static_cast<ERROR_CODES> (diskError);
            if (error == ERROR_CODES::NONE) {
                state = STATE_CODES::BUSY;
                setSector(c);
                curPosition = 0;
                dmaLocation = (b << 16) + a;
                writing     = false;
            }
            pendingInterrupt = true; // State changes, and error could
        }
        else {
            if (state == STATE_CODES::NO_MEDIA) {
                error            = ERROR_CODES::NO_MEDIA;
                pendingInterrupt = true;
            }
            else if (state == STATE_CODES::BUSY) {
                error            = ERROR_CODES::BUSY;
                pendingInterrupt = true;
            }
            std::cout << "[M5FDD] Reading set to Error: " << static_cast<int>(error) << std::endl;
        }

        break;

    case COMMANDS::WRITE_SECTOR:
        if (state == STATE_CODES::READY) {
            if (!floppy) {
                // This not should happen never, but...
                state = STATE_CODES::NO_MEDIA;
                error = ERROR_CODES::BROKEN;
                std::cout << "[M5FDD] COMMANDS::WRITE_SECTOR, STATE_CODES::READY, but no floppy class!" << std::endl;
                break;
            }
            vm::dev::disk::ERRORS diskError = floppy->writeSector(c, &sectorBuffer, true);
            error = static_cast<ERROR_CODES> (diskError);
            if (error == ERROR_CODES::NONE) {
                state = STATE_CODES::BUSY;
                setSector(c);
                curPosition = 0;
                dmaLocation = (b << 16) + a;
                writing     = true;
            }
            pendingInterrupt = true; // State changes, and error could
        }
        else {
            if (state == STATE_CODES::NO_MEDIA) {
                error            = ERROR_CODES::NO_MEDIA;
                pendingInterrupt = true;
            }
            else if (state == STATE_CODES::READY_WP) {
                error            = ERROR_CODES::PROTECTED;
                pendingInterrupt = true;
            }
            else if (state == STATE_CODES::BUSY) {
                error            = ERROR_CODES::BUSY;
                pendingInterrupt = true;
            }

            std::cout << "[M5FDD] Writing set to Error: " << static_cast<int>(error) << std::endl;
        }
        break;

    case COMMANDS::QUERY_MEDIA:
        if (floppy != NULL) {
            a = floppy->getTotalSectors();
            b = (floppy->getDescriptor()->NumSides << 8) + floppy->getDescriptor()->TracksPerSide;
            c = (floppy->getDescriptor()->SectorsPerTrack << 8) + floppy->getBytesExponent();
        }
        else {
            a = b = c = 0;
        }
        break;

    default:
        break;
    } // switch
}     // SendCMD

void M5FDD::IACK() {
    pendingInterrupt = false;
}

void M5FDD::Tick(unsigned n, const double delta) {
    for (int i = 0; i < n; i++) {
        if (busyCycles > 0 && state == STATE_CODES::BUSY) {

            //TODO: integrate with DMA system
            busyCycles--;

            // continue DMAing RAM <-> BUFFER
            if (curPosition < floppy->getDescriptor()->BytesPerSector) {
                if (writing) {
                    sectorBuffer[curPosition] = vcomp->ReadB(dmaLocation + curPosition);
                }
                else {
                    vcomp->WriteB(dmaLocation + curPosition, sectorBuffer[curPosition]);
                }
                curPosition++;
            }

            // just finished DMAing to the buffer, write it
            if (writing && curPosition == floppy->getDescriptor()->BytesPerSector) {
                floppy->writeSector(curSector, &sectorBuffer);
            }
        }
        else if (floppy && state == STATE_CODES::BUSY) {
            state            = floppy->isProtected() ? STATE_CODES::READY_WP : STATE_CODES::READY;
            pendingInterrupt = true; // State changes
        }
    }
} // Tick

void M5FDD::insertFloppy(std::shared_ptr<vm::dev::disk::Disk> floppy) {
    if (this->floppy) {
        ejectFloppy();
    }

    this->floppy = floppy;
    state        = floppy->isProtected() ? STATE_CODES::READY_WP : STATE_CODES::READY;
    error        = ERROR_CODES::NONE;
    sectorBuffer.resize(floppy->getDescriptor()->BytesPerSector);
    pendingInterrupt = true; // State changes, and error could
    std::cout << "[M5FDD] Disk inserted! " << floppy->getFilename() << std::endl;
} // insertFloppy

void M5FDD::ejectFloppy() {
    if (this->floppy) {
        this->floppy.reset(); // like = NULL
    }

    std::cout << "[M5FDD] Disk ejected!" << std::endl;

    if (state == STATE_CODES::BUSY) {
        error = ERROR_CODES::EJECT;
    }
    else {
        error = ERROR_CODES::NONE;
    }

    state            = STATE_CODES::NO_MEDIA;
    pendingInterrupt = true; // State changes, and error could
} // ejectFloppy

void M5FDD::setSector(uint16_t sector) {

    //TODO: seek timing calculations

    //TODO: disk read timing calculations
    busyCycles += 512;

    curSector = sector;
}
} // End of namespace m5fdd
} // End of namespace dev
} // End of namespace vm
