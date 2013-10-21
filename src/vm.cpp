#include "cpu.hpp"
#include "dis_rc3200.hpp"

#include <iostream>
#include <ios>
#include <iomanip> 
#include <cstdio>
#include <algorithm>
#include <string>
#include <cwctype>
#include <clocale>

//#include <SFML/Graphics.hpp>
//#include <SFML/Graphics/RenderWindow.hpp>

#define FRAMERATE (60)

std::size_t prg_size = 45*4;
vm::dword_t prg[] = {
    0x80008020,  // 000h SET %r0, 1
    0x80008021,  // 004h SET %r1, 1
    0x80008042,  // 008h SET %r2, 2 ..
    0x80008063,
    0x80008084,
    0x800080A5,
    0x800080C6,
    0x800080E7,
    0x80008108,
    0x80008129,
    0x8000814A,
    0x8000816B,
    0x8000818C,
    0x800081AD,
    0x800081CE,
    0x800081EF,   // 03Ch SET %r15 = 15
    0x80008210,   // 040h SET %r16 = 16
    0x80008231,   // 044h SET %r17 = 17
    0x80008252,   
    0x80008273,   
    0x80008294,   
    0x800082B5,   
    0x800082D6,   
    0x800082F7,   
    0x80008318,   
    0x80008339,   
    0x8000835A,   
    0x8000837B,   
    0x8000839C,   
    0x800083BD,   
    0x800083DE,   // 078h SET %r30 = 30
    0x800083FF,   // 07Ch SET %r31 = 31
    0x8000801E,   // 080h SET %r30 = 0
    0x8000801F,   // 084h SET %r31 = 0
    0x8000C001,   // 088h SET %r1 = 0xBEBECAFE
    0xBEBECAFE,   // 08Ch literal value
    0x8000002A,   // 090h CPY %r1, %r10
    0x40000000,   // 094h NOT %r0
    0x40010003,   // 098h NEG %r3 (%r3 = -3 = 0xFFFFFFFD)
    0x40020004,   // 09Ch XCHG %r4
    0x40030005,   // 0A0h XCHG.W %r5
    0x8000FFE7,   // 0A4h SET %r7, -1 (%r5 = 0xFFFFFFFF)
    0x8000FFC6,   // 0A8h SET %r6, -2 (%r6 = 0xFFFFFFFE)
    0xC000208C,   // 0ACh ADD %r8, %r4, %r12 (%r12 = %r8 +%r4 = 0x408)
    0xC000A020,   // 0B0h ADD %r8, 1, %r0 (%r0 = %r8 +1 = 9)
    0x2034,  // 02Eh SXTBD r4       (r4  == 0xFFFF)
    0x4025,  // 030h ADD r5, r2     (r5  == 0x0003) CF=1
    0x4111,  // 032h ADD r1, 1      (r1  == 0xFFFF)
    0x4225,  // 034h SUB r5, r2     (r5  == 0x0005) CF=1
    0x4316,  // 036h SUB r6, 1      (r6  == 0x0005)
    0x62FA,  // 038h SET r10, 0x7FFF
    0x7FFF,  // 03Ah literal
    0x411A,  // 03Ch ADD r10, 1     (r10 == 0x8000) OF=1
    0x6061,  // 03Eh SWP r1 ,r6
    0x6116,  // 040h CPY r6, r1     (r1 == r6 == 0x0005)
    0x8001,  // 042h LOAD [r0], r1          (r1 == 0x6210)
    0x9061,  // 044h LOAD [r0 + r6], r1     (r1 == 0x3362)
    0xA012,  // 046h LOAD.B [r0+1], r2      (r2 == 0xFF62)
    0xB082,  // 048h LOAD.B [r0+r8], r2     (r2 == 0xFF44)
    0x6456,  // 04Ah BEQ r6 == 5  (true)
    0x62F6,  // 04Ch SET r6, 0xCAFE (not should happen)
    0xCAFE,  // 04Eh literal
    0x6201,  // 050h SET r1, 0
    0x6401,  // 052h BEQ r1, 0  (true)
    0x6501,  // 054h BNEQ r1, 0 (false, but chained)
    0x62F6,  // 056h SET r6, 0xCAFE (not should happen)
    0xCAFE,  // 058h literal
    0x2042,  // 05Ah PUSH r2  (SP = FFFE and [FFFF] = FF44) 
    0x205B,  // 05Ch POP r11 (SP = 0 and r11 = FF44)
    0x21B1,  // 05Eh SETIS 1 (Interrupts in segment 1)
    0x20F2,  // 060h SETIA 2
    0x20D6,  // 062h INT 6
    0x215B,  // 064h SETDS 0xB
    0x62F1,  // 068h SET r1, 'A'
    0x0041,  // 06Ah
    0x6202,  // 06Ch SET r2, 0
    0xC201,  // 06Eh STORE [r2], r1  (type A)
    0x62F2,  // 070h SET r2, 80*2
    0x00A0,  // 072h
    0x4111,  // 074h ADD r1, 1 
    0xC201,  // 076h STORE [r2], r1  (type B)
    0x62F2,  // 078h SET r2, 80*4
    0x0140,  // 07Ah
    0x4111,  // 07Ch ADD r1, 1 
    0xC201,  // 07Eh STORE [r2], r1  (type C)
    0x2150,  // 080h SETDS 0
    0xC001,  // 082h STORE [0], r1
    0x8001,  // 084h LOAD [0], r1 (shoud be 0x6210)
    0x0000,
    0x0000,
    0x0000,
};

std::size_t isr_size = 2*2;
vm::word_t isr[] = {
	0x00000000,  // 000h NOP
    0x0004,  // 002h RFI
};

void print_regs(const vm::cpu::CpuState& state);
void print_pc(const vm::cpu::CpuState& state, const vm::ram::Mem& ram);
void print_stack(const vm::cpu::CpuState& state, const vm::ram::Mem& ram);

int main()
{
    using namespace vm;
    using namespace vm::cpu;

    RC3200 cpu;

    cpu.reset();
	
	auto prg_blq = cpu.ram.addBlock(0, 0x8000, true); // ROM
	cpu.ram.addBlock(0x8000, 0x8000); // Free ram
	auto isr_blq = cpu.ram.addBlock(0x10000, 0x10000);
	
    auto mda_blq = cpu.ram.addBlock(0xB0000, 0x10000);
   
    std::cout << "Allocted memory: " << cpu.ram.allocateBlocks()/1024 
              << "KiB" << std::endl;

    {
        auto prg_sptr = prg_blq.lock();
        auto isr_sptr = isr_blq.lock();

        std::copy_n((byte_t*)prg, prg_size, prg_sptr->getPtr()); // Copy program
        std::copy_n((byte_t*)isr, isr_size, isr_sptr->getPtr()); // Copy ISR
    }
    
	std::cout << "Run program (r) or Step Mode (s) ?\n";
    char mode;
    std::cin >> mode;
    std::getchar();

    //sf::RenderWindow mda_window;
    //mda_window.create(sf::VideoMode(640, 406), "RC1600 prototype");
    //mda_window.setFramerateLimit(FRAMERATE);

    bool debug = false;
    if (mode == 's' || mode == 'S') {
        debug = true;
    }
   
    /*
    sf::Font font;
    if (!font.loadFromFile("./assets/font/cour.ttf")) {
        std::cerr << "Error loading font\n";
        exit(0);
    }
    sf::Text text;
    text.setFont(font);
    text.setColor(sf::Color::Green);
    text.setCharacterSize(14); // 14 pixels*/

    std::cout << "Running!\n";
    //sf::Clock clock; 
    unsigned ticks;
    unsigned long ticks_count = 0; 
    //mda_window.clear(sf::Color::Black);
    //mda_window.display();

    int c = ' ';
    while ( 1) {//mda_window.isOpen()) {

        // T period of a 1MHz signal = 1 microseconds
        //const auto delta=clock.getElapsedTime().asMicroseconds(); 
        //clock.restart();
        //double tmp = cpu.getClock()/ (double)(FRAMERATE);
        //ticks= tmp+0.5;
        
        if (debug) {
            print_pc(cpu.getState(), cpu.ram);
            if (cpu.getState().skiping)
                std::printf("Skiping!\n");
            if (cpu.getState().sleeping)
                std::printf("ZZZZzzzz...\n");
        }

        if (!debug)
            cpu.tick(ticks);
        else
            ticks = cpu.step();

        ticks_count += ticks;

        // Print to the stdout a 80x25 MDA like screen at 80000h
        /*auto mda_ram = mda_blq.lock();
        if (mda_ram) {
            std::wstring txt = L"";
            unsigned col = 0, row = 0;
            dword_t i = 0;
            for(row =0; row < 25 && i < mda_ram->size(); row++) {
                for(col =0; col < 80 && i < mda_ram->size(); col++) {
                    wchar_t c = (wchar_t)(mda_ram->getPtr()[i]);
                    i +=2;
                    if (std::iswgraph(c))
                        txt.push_back(c);
                    else
                        txt.push_back(L' ');
                }
                txt.push_back(L'\n');
            }
            text.setString(txt);
        }*/

        // Speed info
        if (ticks_count > 100000 && !debug) {
            std::cout << "Running " << ticks; /*<< " cycles in " << delta << " uS"
                      << " Speed of " 
                      << 100 * (((ticks * 1000000.0) / cpu.getClock()) / delta)
                      << "% \n";*/
            std::cout << std::endl;
            ticks_count -= 100000;
        }

        // Poll events
        /*sf::Event ev;
        while (mda_window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                mda_window.close();
                break;
            }
        }*/


        /*mda_window.clear(sf::Color(0,0,0, 0xAA));
        // Update here
        mda_window.draw(text);
        mda_window.display();*/

        if (debug) {
            std::printf("Takes %u cycles\n", cpu.getState().wait_cycles);
            print_regs(cpu.getState());
            print_stack(cpu.getState(), cpu.ram);
            c = std::getchar();
            if (c == 'q' || c == 'Q')
                break;
                //mda_window.close();

        }

    }

    return 0;
}


void print_regs(const vm::cpu::CpuState& state)
{
    // Print registers
    for (int i=0; i < 32; i++) {
        std::printf("r%2d= 0x%08x ", i, state.r[i]);
        if (i == 3 || i == 7 || i == 11 || i == 15 || i == 19 || i == 23 || i == 27 || i == 31)
            std::printf("\n");
    }

    std::printf(" PC= 0x%08x ", state.pc);
    std::printf("IA= 0x%08x ", state.ia);
    std::printf("FLAGS= 0x%08x \n", state.flags);
    std::printf("EDE: %d EOE: %d ESS: %d EI: %d \t IF: %d DE %d OF: %d CF: %d\n",
            GET_EDE(state.flags), GET_EOE(state.flags), GET_ESS(state.flags),
            GET_EI(state.flags),
            GET_IF(state.flags), GET_DE(state.flags), GET_OF(state.flags),
            GET_CF(state.flags));
    std::printf("\n");

}

void print_pc(const vm::cpu::CpuState& state, const vm::ram::Mem&  ram)
{
    std::printf("\t[PC]= 0x%02x%02x%02x%02x ", 
            ram.rb(state.pc + 3), 
            ram.rb(state.pc + 2), 
            ram.rb(state.pc + 1), 
            ram.rb(state.pc )); // Little Endian
    std::cout << vm::cpu::disassembly(ram,  state.pc) << std::endl;  
}

void print_stack(const vm::cpu::CpuState& state, const vm::ram::Mem& ram)
{
    std::printf("STACK:\n");

    for (size_t i =0; i <6; i++) {
        std::printf("%02Xh ", ram.rb(state.r[SP]+i));
        if (((size_t)(state.r[SP]) + i) >= 0xFFFFFFFF)
            break;
    }
    std::printf("\n");
}

