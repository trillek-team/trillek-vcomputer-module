/**
* Trillek Virtual Computer - DCPU16N.cpp
* Implementation of the DCPU-16N CPU
*/

#include "DCPU16N.hpp"
#include "DCPU16N_macros.hpp"

namespace vm {
  namespace cpu {

    DCPU16N::DCPU16N(unsigned clock) : ICPU(), cpu_clock(clock) {
      this->Reset();
    }

    DCPU16N::~DCPU16N() {}

    void DCPU16N::Reset()
    {
      int i;

      // Generic initialization
      std::fill_n(r, 8, 0);
      pc = 0;
      sp = 0;
      ex = 0;
      ia = 0;
      for(i = 0; i < 16; i++) emu[i] = i << 12;
      iqp = 0;
      iqc = 0;
      madraw = 0;
      phase = 0;
      acu = 0;
      bytemode = false;
      bytehigh = false;
      skip = false;
      fire = false;

      // point EMU at ROM (page 0x100)
      emu[0] = 0x00100000;

    }

    unsigned DCPU16N::Step()
    {
      unsigned x = 0;
      do {
        Tick(1);
        x++;
      } while((!fire) && (phase != 0));
      return x;
    }

    void DCPU16N::Tick(unsigned n)
    {
      dword_t cfa;
      register int32_t s32;
      word_t opca;
      while(n--) {
        switch(phase) {
        case DCPU16N_PHASE_OPFETCH:
          cfa = emu[(pc >> 12) & 0xf] | (pc & 0x0fff);
          opcl = (((word_t)vcomp->ReadB(cfa + 1)) << 8) | (word_t)vcomp->ReadB(cfa);
          pc += 2;
          phase = DCPU16N_PHASE_UAREAD;
          break;
        case DCPU16N_PHASE_NWAFETCH:
          cfa = emu[(pc >> 12) & 0xf] | (pc & 0x0fff);
          fetchh = (((word_t)vcomp->ReadB(cfa + 1)) << 8) | (word_t)vcomp->ReadB(cfa);
          pc += 2;
          if(addradd) {
            fetchh += acu;
            addradd = false;
          }
          acu = fetchh;
          if(addrdec) {
            phase = DCPU16N_PHASE_ACUFETCH;
          }
          else {
            phase = DCPU16N_PHASE_UBREAD;
          }
          break;
        case DCPU16N_PHASE_NWBFETCH:
          cfa = emu[(pc >> 12) & 0xf] | (pc & 0x0fff);
          fetchh = (((word_t)vcomp->ReadB(cfa + 1)) << 8) | (word_t)vcomp->ReadB(cfa);
          pc += 2;
          if(addradd) {
            fetchh += bcu;
            addradd = false;
          }
          bcu = fetchh;
          if(addrdec) {
            phase = DCPU16N_PHASE_BCUFETCH;
          }
          else {
            phase = DCPU16N_PHASE_EXEC;
          }
          break;
        case DCPU16N_PHASE_UAREAD:
          if((opcl & 0x001f) != 0 || (opcl & 0x03e0) != 0) {
            opca = opcl >> 10;
            if(opca & 0x0020) {
              acu = (word_t)(0xffffu + (opca & 0x1f));
            }
            else {
              if(opca & 0x010) {
                if(opca & 0x08) {
                  switch(opca & 0x7) {
                  case 0: // POP [SP++]
                    acu = sp++;
                    phase = DCPU16N_PHASE_ACUFETCH;
                    addrdec = true;
                    break;
                  case 1: // [SP]
                    acu = sp;
                    phase = DCPU16N_PHASE_ACUFETCH;
                    addrdec = true;
                    break;
                  case 2: // [SP + nextword]
                    acu = sp;
                    phase = DCPU16N_PHASE_NWAFETCH;
                    addradd = true;
                    addrdec = true;
                    break;
                  case 3: // SP
                    acu = sp;
                    break;
                  case 4: // PC
                    acu = pc;
                    break;
                  case 5: // EX
                    acu = ex;
                    break;
                  case 6: // [nextword]
                    phase = DCPU16N_PHASE_NWAFETCH;
                    addrdec = true;
                    break;
                  case 7: // nextword
                    phase = DCPU16N_PHASE_NWAFETCH;
                    break;
                  }
                  if(addrdec || phase == DCPU16N_PHASE_NWAFETCH)
                    break;
                }
                else { // [REG + nextword]
                  acu = r[opca & 0x7];
                  if(opca & 0x08) {
                    phase = DCPU16N_PHASE_NWAFETCH;
                    addrdec = true;
                    addradd = true;
                    break;
                  }
                }
              }
              else { // REG
                acu = r[opca & 0x7];
                if(opca & 0x08) { // [REG]
                  phase = DCPU16N_PHASE_ACUFETCH;
                  addrdec = true;
                  break;
                }
              }
            }
          }
        case DCPU16N_PHASE_ACUFETCH:
          if(addrdec) {
            addrdec = false;
            aca = emu[(acu >> 12) & 0xf] | (acu & 0x0fff);
            acu = (((word_t)vcomp->ReadB(aca + 1)) << 8) | (word_t)vcomp->ReadB(aca);
          }
        case DCPU16N_PHASE_UBREAD:
          if((opcl & 0x001f) != 0) {
            opca = (opcl >> 5) & 0x01f;
            if(opca & 0x010) {
              if(opca & 0x08) { // b table
                switch(opca & 0x7) {
                case 0: // PUSH [--SP] (READ)
                  bcu = --sp;
                  phase = DCPU16N_PHASE_BCUFETCH;
                  addrdec = true;
                  break;
                case 1: // [SP] (READ)
                  bcu = sp;
                  phase = DCPU16N_PHASE_BCUFETCH;
                  addrdec = true;
                  break;
                case 2: // [SP+nextword] (READ)
                  bcu = sp;
                  phase = DCPU16N_PHASE_NWBFETCH;
                  addradd = true;
                  addrdec = true;
                  break;
                case 3: // SP (READ)
                  bcu = sp;
                  break;
                case 4: // PC (READ)
                  bcu = pc;
                  break;
                case 5: // EX (READ)
                  bcu = ex;
                  break;
                case 6: // [nextword] (READ)
                  phase = DCPU16N_PHASE_NWBFETCH;
                  addrdec = true;
                  break;
                case 7: // nextword (READ)
                  phase = DCPU16N_PHASE_NWBFETCH;
                  break;
                }
                if(addrdec || phase == DCPU16N_PHASE_NWBFETCH)
                  break;
              }
              else { // [REG + nextword]
                bcu = r[opca & 0x7];
                phase = DCPU16N_PHASE_NWBFETCH;
                addrdec = true;
                addradd = true;
                break;
              }
            }
            else { // REG
              bcu = r[opca & 0x7];
              if(opca & 0x08) { // [REG]
                phase = DCPU16N_PHASE_BCUFETCH;
                addrdec = true;
                break;
              }
            }
          }
        case DCPU16N_PHASE_BCUFETCH:
          if(addrdec) {
            addrdec = false;
            bca = emu[(bcu >> 12) & 0xf] | (bcu & 0x0fff);
            bcu = (((word_t)vcomp->ReadB(bca + 1)) << 8) | (word_t)vcomp->ReadB(bca);
          }
        case DCPU16N_PHASE_EXEC:
          if((opcl & 0x001f) != 0) {
            wrt = (opcl >> 5) & 0x1f;
            switch(opcl & 0x001f) {
            case 0x01: // SET (wb ra)
              bcu = acu;
              break;
            case 0x02: // ADD (rwb ra)
              cfa = bcu;
              cfa += acu;
              bcu = (word_t)cfa;
              ex = (word_t)(cfa >> 16);
              break;
            case 0x03: // SUB (rwb ra)
              s32 = (int16_t)bcu;
              s32 -= (int16_t)acu;
              bcu = (word_t)s32;
              ex = (word_t)(s32 >> 16);
              break;
            case 0x04: // MUL (rwb ra)
              cfa = bcu;
              cfa *= acu;
              bcu = (word_t)cfa;
              ex = (word_t)(cfa >> 16);
              // TODO waste cycles
              break;
            case 0x05: // MLI (rwb ra)
              s32 = (int16_t)bcu;
              s32 *= (int16_t)acu;
              bcu = (word_t)s32;
              ex = (word_t)(s32 >> 16);
              break;
            case 0x06: // DIV (rwb ra)
              if(acu) {
                cfa = (((dword_t)bcu) << 16) / acu;
                ex = (word_t)cfa;
                bcu = (word_t)(cfa >> 16);
              }
              else {
                bcu = 0;
                ex = 0;
              }
              break;
            case 0x07: // DVI (rwb ra)
              if(acu) {
                if(((int16_t)bcu) % ((int16_t)acu)) {
                  ex = ( ((int32_t)bcu) << 16 ) / ((int16_t)acu);
                }
                else {
                  ex = 0;
                }
                bcu = (word_t)( ((int16_t)bcu) / ((int16_t)acu) );
              }
              else {
                bcu = 0;
                ex = 0;
              }
              break;
            case 0x08: // MOD (rwb ra)
              if(acu) {
                bcu %= acu;
              }
              else {
                bcu = 0;
              }
              break;
            case 0x09: // MDI (rwb ra)
              if(acu) {
                bcu = (word_t)( ((int16_t)bcu) % ((int16_t)acu) );
              }
              else {
                bcu = 0;
              }
              break;
            case 0x0a: // AND (rwb ra)
              bcu &= acu;
              break;
            case 0x0b: // BOR (rwb ra)
              bcu |= acu;
              break;
            case 0x0c: // XOR (rwb ra)
              bcu ^= acu;
              break;
            case 0x0d: // SHR (rwb ra)
              cfa = bcu << 16;
              cfa >>= acu;
              ex = (word_t)cfa;
              bcu = (word_t)(cfa >> 16);
              break;
            case 0x0e: // ASR (rwb ra)
              s32 = (int16_t)bcu;
              ex = (word_t)(bcu << (16 - acu));
              bcu = (word_t)(s32 >> acu);
              break;
            case 0x0f: // SHL (rwb ra)
              cfa = bcu;
              cfa <<= acu;
              ex = (word_t)(cfa >> 16);
              bcu = (word_t)cfa;
              break;
            case 0x10: // IFB (rb ra)
              if((acu & bcu)) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x11: // IFC (rb ra)
              if(!(acu & bcu)) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x12: // IFE (rb ra)
              if(!(acu == bcu)) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x13: // IFN (rb ra)
              if(!(acu != bcu)) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x14: // IFG (rb ra)
              if(!(bcu > acu)) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x15: // IFA (rb ra)
              if(! ( ((int16_t)bcu) > ((int16_t)acu)) ) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x16: // IFL (rb ra)
              if(!(bcu < acu)) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            case 0x17: // IFU (rb ra)
              if(! ( ((int16_t)bcu) < ((int16_t)acu) ) ) {
                phase = DCPU16N_PHASE_EXECSKIP;
              }
              break;
            //case 0x18:
            //  break;
            //case 0x19:
            //  break;
            case 0x1a: // ADX (rwb ra)
              cfa = bcu;
              cfa += acu + ex;
              ex = (word_t)(cfa >> 16);
              bcu = (word_t)cfa;
              break;
            case 0x1b: // SBX (rwb ra)
              cfa = bcu;
              cfa = cfa - acu + ex;
              ex = (word_t)(cfa >> 16);
              bcu = (word_t)cfa;
              break;
            case 0x1c: // HWW (rb ra)
              IOWrite(bcu, acu);
              break;
            case 0x1d: // HWR (rb wa)
              bcu = IORead(bcu);
              wrt = (opcl >> 10);
              break;
            case 0x1e: // STI (wb ra)
              bcu = acu;
              r[6]++; r[7]++;
              break;
            case 0x1f: // STD (wb ra)
              bcu = acu;
              r[6]--; r[7]--;
              break;
            }
          }
          else if((opcl & 0x03e0) != 0) {
            wrt = (opcl >> 10);
            switch((opcl >> 5) & 0x001f) {
            case 0x01: // JSR (ra)
              break;
            case 0x02: // BSR (ra)
              break;
            //case 0x03:
            //  break;
            //case 0x04:
            //  break;
            case 0x05: // NEG (rwa)
              break;
            //case 0x06:
            //  break;
            case 0x07: // HCF (^w^OMGFTWBBQa)
              break;
            case 0x08: // INT (ra)
              break;
            case 0x09: // IAG (wa)
              break;
            case 0x0a: // IAS (ra)
              break;
            case 0x0b: // RFI (ra)
              break;
            case 0x0c: // IAQ (ra)
              break;
            //case 0x0d:
            //  break;
            //case 0x0e:
            //  break;
            //case 0x0f:
            //  break;
            case 0x10: // MMW (ra)
              break;
            case 0x11: // MMR (rwa)
              break;
            //case 0x12:
            //  break;
            //case 0x13:
            //  break;
            case 0x14: // SXB (rwa)
              break;
            case 0x15: // SWP (rwa)
              break;
            //case 0x16:
            //  break;
            //case 0x17:
            //  break;
            //case 0x18:
            //  break;
            //case 0x19:
            //  break;
            //case 0x1a:
            //  break;
            //case 0x1b:
            //  break;
            //case 0x1c:
            //  break;
            //case 0x1d:
            //  break;
            //case 0x1e:
            //  break;
            //case 0x1f:
            //  break;
            }
          }
          else {
            wrt = 0x003f;
            switch((opcl >> 10) & 0x001f) {
            case 0x00: // HLT
              break;
            case 0x01: // SLP
              break;
            //case 0x02:
            //  break;
            //case 0x03:
            //  break;
            case 0x04: // BYT (rv)
              break;
            case 0x10: // SKP
              break;
            }
          }

        case DCPU16N_PHASE_UBWRITE:
          if(wrt) {
            if(wrt & 0x0020) {
              // nothing
            }
            else {
              if(wrt & 0x010) {
                if(wrt & 0x08) {
                  switch(wrt & 0x7) {
                  case 0: // PUSH [--SP]
                    addrdec = true;
                    break;
                  case 1: // [SP]
                    addrdec = true;
                    break;
                  case 2: // [SP + nextword]
                    addrdec = true;
                    break;
                  case 3: // SP
                    sp = bcu;
                    break;
                  case 4: // PC
                    pc = bcu;
                    break;
                  case 5: // EX
                    ex = bcu;
                    break;
                  case 6: // [nextword]
                    addrdec = true;
                    break;
                  case 7:
                    break;
                  }
                }
                else { // [REG + nextword]
                  addrdec = true;
                }
              }
              else {
                if(wrt & 0x08) { // [REG]
                  addrdec = true;
                }
                else { // REG
                  r[wrt & 0x7] = bcu;
                }
              }
            }
          }
        case DCPU16N_PHASE_BCUWRITE:
          if(addrdec) {
            addrdec = false;
            vcomp->WriteB(bca + 1, (byte_t)(bcu >> 8));
            vcomp->WriteB(bca, (byte_t)(bcu & 0x0ff));
          }
          break;
        case DCPU16N_PHASE_EXECW:
          // TODO
          break;
        case DCPU16N_PHASE_SLEEP:
          // TODO
          // Check Interrupts here
          break;
        case DCPU16N_PHASE_EXECSKIP:
          break;
        default:
          phase = 0;
          break;
        }
      }
    }

    bool DCPU16N::SendInterrupt(word_t msg)
    {
      return true;
    }

    word_t DCPU16N::IORead(word_t addr)
    {
      return vcomp->ReadW(0x00110000 | addr);
    }

    void DCPU16N::IOWrite(word_t addr, word_t v)
    {
      vcomp->WriteW(0x00110000 | addr, v);
    }

    void DCPU16N::GetState(void* ptr, std::size_t& size) const
    {
      size = 0;
    }

    bool DCPU16N::SetState(const void* ptr, std::size_t size)
    {
      return false;
    }
  }
}
