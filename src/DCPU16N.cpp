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
            cfa = emu[(acu >> 12) & 0xf] | (acu & 0x0fff);
            acu = (((word_t)vcomp->ReadB(cfa + 1)) << 8) | (word_t)vcomp->ReadB(cfa);
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
            cfa = emu[(bcu >> 12) & 0xf] | (bcu & 0x0fff);
            bcu = (((word_t)vcomp->ReadB(cfa + 1)) << 8) | (word_t)vcomp->ReadB(cfa);
          }
        case DCPU16N_PHASE_EXEC:
          // TODO
        case DCPU16N_PHASE_UBWRITE:
          // TODO
        case DCPU16N_PHASE_BCUWRITE:
          if(addrdec) {
            addrdec = false;
            cfa = emu[(bcu >> 12) & 0xf] | (bcu & 0x0fff);
            vcomp->WriteB(cfa + 1, (byte_t)(acu >> 8));
            vcomp->WriteB(cfa, (byte_t)(acu & 0x0ff));
          }
          break;
        case DCPU16N_PHASE_EXECW:
          // TODO
          break;
        case DCPU16N_PHASE_SLEEP:
          // TODO
          // Check Interrupts here
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
