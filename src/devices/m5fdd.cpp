/**
 * \brief       Mackapar 5.25" Floppy Drive
 * \file        m5fdd.cpp
 * \copyright   The MIT License (MIT)
 *
 * Mackapar 5.25" Floppy Drive
 */

#include "devices/m5fdd.hpp"
#include "vs_fix.hpp"

#include <cstdio>

namespace trillek {
namespace computer {
namespace m5fdd {

M5FDD::M5FDD()  {
    this->Reset();
}

M5FDD::~M5FDD() {
    ejectFloppy();
}

void M5FDD::Reset() {
#ifndef NDEBUG
    std::cout << "[M5FDD] Device reset!" << std::endl;
#endif

/*
 * Reset() is a power cycle, but reset some internal state
 * Calling reset() while writing to the disk will not corrupt the VCD file.
 */
    busyCycles = a = b = c = d = 0;
    msg = 0;
    curHead = 0;
    curTrack = 0;
    curSector = 1;
    dmaLocation = 0;
    curPosition = 0;
    busyCycles = 0;
    pendingInterrupt = false;
    if (floppy) {
        state  = floppy->isProtected() ? STATE_CODES::READY_WP : STATE_CODES::READY;
    } else {
        state = STATE_CODES::NO_MEDIA;
    }
        error = ERROR_CODES::NONE;
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

void M5FDD::SendCMD(Word cmd) {
    if (vcomp == NULL) {
        return;
    }

    switch ( static_cast<COMMANDS>(cmd) ) {

    case COMMANDS::SET_INTERRUPT:
        msg = a;
#ifndef NDEBUG
        std::cout << "[M5FDD] msg set to " << msg << std::endl;
#endif
        break;

    case COMMANDS::READ_SECTOR:

        if (state == STATE_CODES::READY || state == STATE_CODES::READY_WP) {
            if (!floppy) {
                // This not should happen never, but...
                state = STATE_CODES::NO_MEDIA;
                error = ERROR_CODES::BROKEN;
#ifndef NDEBUG
                std::cout << "[M5FDD] COMMANDS::READ_SECTOR, STATE_CODES::READY, but no floppy class!" << std::endl;
#endif
                break;
            }

            const Byte sector = c & 0xFF;
            const Byte track = (c >> 8) & 0x7F;
            const Byte head = c >> 15;
            auto lba = CHStoLBA(track, head, sector, *(floppy->getDescriptor()));
            if (lba < 0) { // Check if is a valid CHS value
                error = ERROR_CODES::BAD_CHS;
#ifndef NDEBUG
                std::cout << "[M5FDD] bad CHS value" << std::endl;
#endif
                break;
            }
#ifndef NDEBUG
            std::fprintf(stderr, "[M5FDD] Read at LBA:%u C:%u H:%u S:%u\t",
                    lba, track, head, sector);
            std::fprintf(stderr, "@ 0x%08X\n", (b << 16) + a);
#endif

            // read the sector
            ERRORS diskError = floppy->readSector(lba, &sectorBuffer);
            error = static_cast<ERROR_CODES> (diskError);
            if (error == ERROR_CODES::NONE) {
                state = STATE_CODES::BUSY;
                setSector(track, head, sector);
                curPosition = 0;
                dmaLocation = (b << 16) + a;
                writing     = false;
            }
            pendingInterrupt = true; // State changes, and error could
        } else {
            if (state == STATE_CODES::NO_MEDIA) {
                error            = ERROR_CODES::NO_MEDIA;
                pendingInterrupt = true;
            } else if (state == STATE_CODES::BUSY) {
                error            = ERROR_CODES::BUSY;
                pendingInterrupt = true;
            }
#ifndef NDEBUG
            std::cout << "[M5FDD] Reading set to Error: " << static_cast<int>(error) << std::endl;
#endif
        }

        break;

    case COMMANDS::WRITE_SECTOR:
        if (state == STATE_CODES::READY) {
            if (!floppy) {
                // This not should happen never, but...
                state = STATE_CODES::NO_MEDIA;
                error = ERROR_CODES::BROKEN;
#ifndef NDEBUG
                std::cout << "[M5FDD] COMMANDS::WRITE_SECTOR, STATE_CODES::READY, but no floppy class!" << std::endl;
#endif
                break;
            }

            const Byte sector = c & 0xFF;
            const Byte track = (c >> 8) & 0x7F;
            const Byte head = c >> 15;
            auto lba = CHStoLBA(track, head, sector, *(floppy->getDescriptor()));
            if (lba < 0) { // Check if is a valid CHS value
                error = ERROR_CODES::BAD_CHS;
#ifndef NDEBUG
                std::cout << "[M5FDD] bad CHS value" << std::endl;
#endif
                break;
            }
#ifndef NDEBUG
            std::fprintf(stderr, "[M5FDD] Write at LBA:%u C:%u H:%u S:%u\t",
                    lba, track, head, sector);
            std::fprintf(stderr, "@ 0x%08X\n", (b << 16) + a);
#endif
            // TODO WTF ! Does write two times !!!
            ERRORS diskError = floppy->writeSector(lba, &sectorBuffer, true);
            error = static_cast<ERROR_CODES> (diskError);
            if (error == ERROR_CODES::NONE) {
                state = STATE_CODES::BUSY;
                setSector(track, head, sector);
                curPosition = 0;
                dmaLocation = (b << 16) + a;
                writing     = true;
            }
            pendingInterrupt = true; // State changes, and error could
        } else {
            if (state == STATE_CODES::NO_MEDIA) {
                error = ERROR_CODES::NO_MEDIA;
                pendingInterrupt = true;
            } else if (state == STATE_CODES::READY_WP) {
                error = ERROR_CODES::PROTECTED;
                pendingInterrupt = true;
            } else if (state == STATE_CODES::BUSY) {
                error = ERROR_CODES::BUSY;
                pendingInterrupt = true;
            }

#ifndef NDEBUG
            std::cout << "[M5FDD] Writing set to Error: " << static_cast<int>(error) << std::endl;
#endif
        }
        break;

    case COMMANDS::QUERY_MEDIA:
        if (floppy) {
            a = floppy->getTotalSectors();
            b = (floppy->getDescriptor()->NumSides << 8) + floppy->getDescriptor()->TracksPerSide;
            c = (floppy->getDescriptor()->SectorsPerTrack << 8) + floppy->getBytesExponent();
        } else { // No media
            a = b = c = 0;
        }
        break;

    default:
        break;
    } // switch
} // SendCMD

void M5FDD::IACK() {
    pendingInterrupt = false;
}

void M5FDD::Tick(unsigned n, const double delta) {
    for (unsigned i = 0; i < n; i++) {
        if (busyCycles > 0 && state == STATE_CODES::BUSY) {

            //TODO: integrate with DMA system
            busyCycles--;

            // continue DMAing RAM <-> BUFFER
            if (curPosition < floppy->getDescriptor()->BytesPerSector) {
                if (writing) { // Writing to disk
                    sectorBuffer[curPosition] = vcomp->ReadB(dmaLocation + curPosition);
                } else { // Reading from disk
                    vcomp->WriteB(dmaLocation + curPosition, sectorBuffer[curPosition]);
                }
                curPosition++;
            }

            // just finished DMAing to the buffer, write it
            if (writing && curPosition == floppy->getDescriptor()->BytesPerSector) {
                auto lba = CHStoLBA(curTrack, curHead, curSector, *(floppy->getDescriptor()));
                floppy->writeSector(lba, &sectorBuffer);
            }
        } else if (floppy && state == STATE_CODES::BUSY) {
            // Updates state
            state = floppy->isProtected() ? STATE_CODES::READY_WP : STATE_CODES::READY;
            pendingInterrupt = true; // State changes
        }
    }
} // Tick

void M5FDD::insertFloppy(std::shared_ptr<Media> floppy) {
    ejectFloppy();

    this->floppy = floppy;
    state = floppy->isProtected() ? STATE_CODES::READY_WP : STATE_CODES::READY;
    error = ERROR_CODES::NONE;
    sectorBuffer.resize(floppy->getDescriptor()->BytesPerSector);
    pendingInterrupt = true; // State changes, and error could
#ifndef NDEBUG
    std::cout << "[M5FDD] Disk inserted! " << floppy->getFilename() << std::endl;
#endif
} // insertFloppy

void M5FDD::ejectFloppy() {
    if (this->floppy) {
        this->floppy.reset(); // like = NULL
#ifndef NDEBUG
        std::cout << "[M5FDD] Disk ejected!" << std::endl;
#endif

        if (state == STATE_CODES::BUSY) {
            error = ERROR_CODES::EJECT;
        } else {
            error = ERROR_CODES::NONE;
        }
        state            = STATE_CODES::NO_MEDIA;
        pendingInterrupt = true; // State changes, and error could
    }
} // ejectFloppy

void M5FDD::setSector (uint8_t track, uint8_t head, uint8_t sector) {
    curTrack = track;
    curHead = head;
    curSector = sector;
    //TODO: seek timing calculations

    //TODO: disk read timing calculations
    busyCycles += 512;
}

} // End of namespace m5fdd
} // End of namespace computer
} // End of namespace trillek
