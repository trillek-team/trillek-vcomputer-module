/*!
 * \brief       Test/Toy emulator
 * \file        main.cpp
 * \copyright   The MIT License (MIT)
 *
 * Test/Toy executable that uses the Virtual Computer lib to run a emulation
 */

#include "OS.hpp"
#include "GlEngine.hpp"
#include "AlEngine.hpp"
#include "VmParser.hpp"

#include "VC.hpp"
#include "devices/DummyDevice.hpp"
#include "TR3200/DisTR3200.hpp"
#include "devices/M5FDD.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <ios>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <cwctype>
#include <clocale>

#include <chrono>

#ifdef GLFW3_ENABLE

class KeyEventHandler : public OS::event::IKeyboardEventHandler {
    public:
        KeyEventHandler () : OS::event::IKeyboardEventHandler(),
        prev_key(vm::dev::gkeyboard::SCANCODES::SCAN_NULL),
        mod (vm::dev::gkeyboard::KEY_MODS::MOD_NONE) {
            // Register listened characters
            for (unsigned c = 0x20; c <= 0x7E; c++){
                this->chars.push_back(c);
            }
            for (unsigned c = 0xA0; c <= 0xFF; c++){
                this->chars.push_back(c);
            }
            this->chars.push_back(0x08); // Backspace
            this->chars.push_back(0x09); // Tabulator
            this->chars.push_back(0x0D); // Return
            this->chars.push_back(0x1B); // Escape
            this->chars.push_back(0x7F); // Delete (need translation)

            // Register listened keys.
            for (unsigned i = 32; i <= 265 ; i++) {
                this->keys.push_back(i);
            }

            this->keys.push_back(340);
            this->keys.push_back(341);
            this->keys.push_back(342);
            this->keys.push_back(344);
            this->keys.push_back(345);
            this->keys.push_back(346);
        }

        // Called when on the keys reported during register has a state change.
        void KeyStateChange(const unsigned int key, const OS::event::KEY_STATE state) {
            using namespace vm::dev::gkeyboard;
            if (state == OS::event::KEY_STATE::KS_DOWN) {
                prev_key = key & 0xFFFF; // Stores the "scancode"

                std::printf("Scancode : 0x%04X\n", prev_key);

                // TODO Use a general if and a lookup table
                switch (prev_key) { // Special cases that not are catch by character events
                    case GLFW_KEY_ESCAPE :
                        gk->EnforceSendKeyEvent(prev_key, KEY_ESC, mod);
                        break;

                    case GLFW_KEY_ENTER :
                        gk->EnforceSendKeyEvent(prev_key, KEY_RETURN, mod);
                        break;

                    case GLFW_KEY_TAB :
                        gk->EnforceSendKeyEvent(prev_key, KEY_TAB, mod);
                        break;

                    case GLFW_KEY_BACKSPACE :
                        gk->EnforceSendKeyEvent(prev_key, KEY_BACKSPACE, mod);
                        break;

                    case GLFW_KEY_INSERT :
                        gk->EnforceSendKeyEvent(prev_key, KEY_INSERT, mod);
                        break;

                    case GLFW_KEY_DELETE :
                        gk->EnforceSendKeyEvent(prev_key, KEY_DELETE, mod);
                        break;

                    case GLFW_KEY_RIGHT :
                        gk->EnforceSendKeyEvent(prev_key, KEY_ARROW_RIGHT, mod);
                        break;

                    case GLFW_KEY_LEFT :
                        gk->EnforceSendKeyEvent(prev_key, KEY_ARROW_LEFT, mod);
                        break;

                    case GLFW_KEY_DOWN :
                        gk->EnforceSendKeyEvent(prev_key, KEY_ARROW_DOWN, mod);
                        break;

                    case GLFW_KEY_UP :
                        gk->EnforceSendKeyEvent(prev_key, KEY_ARROW_UP, mod);
                        break;

                    case GLFW_KEY_LEFT_SHIFT:
                    case GLFW_KEY_RIGHT_SHIFT:
                        gk->EnforceSendKeyEvent(prev_key, KEY_SHIFT, mod);
                        mod |= KEY_MODS::MOD_SHIFT;
                        break;

                    case GLFW_KEY_LEFT_CONTROL:
                    case GLFW_KEY_RIGHT_CONTROL:
                        gk->EnforceSendKeyEvent(prev_key, KEY_CONTROL, mod);
                        mod |= KEY_MODS::MOD_CTRL;
                        break;

                    case GLFW_KEY_LEFT_ALT:
                    case GLFW_KEY_RIGHT_ALT:
                        gk->EnforceSendKeyEvent(prev_key, KEY_ALT, mod);
                        mod |= KEY_MODS::MOD_ALTGR;
                        break;

                    default:
                        return;
                }

            } else {
                switch (key) {
                    case GLFW_KEY_LEFT_SHIFT:
                    case GLFW_KEY_RIGHT_SHIFT:
                        mod &= (KEY_MODS::MOD_SHIFT ^ 0xFFFF);
                        break;

                    case GLFW_KEY_LEFT_CONTROL:
                    case GLFW_KEY_RIGHT_CONTROL:
                        mod &= (KEY_MODS::MOD_CTRL ^ 0xFFFF);
                        break;

                    case GLFW_KEY_LEFT_ALT:
                    case GLFW_KEY_RIGHT_ALT:
                        mod &= (KEY_MODS::MOD_ALTGR ^ 0xFFFF);
                        break;

                    default:
                        return;
                }
            }
        }

        void CharDown(const unsigned int c) {
            unsigned chr = c;
            if (gk) {
                if (chr == 0x7F) {
                    chr = vm::dev::gkeyboard::KEY_DELETE;
                }

                //std::printf("Character %u='%c' mod=%u -> nº events stored : ", chr, chr, mod);
                vm::dword_t scancode = prev_key & 0xFFFF;
                gk->EnforceSendKeyEvent(scancode, c & 0xFF, mod);
                //std::printf("%u \n", gk->E());
            }
        }

        std::shared_ptr<vm::dev::gkeyboard::GKeyboardDev> gk;
        unsigned int prev_key;
        vm::word_t mod;
};

#endif

void print_regs(const vm::cpu::TR3200State& state);
void print_pc(const vm::cpu::TR3200State& state, const vm::VComputer& vc);
//void print_stack(const vm::cpu::TR3200& cpu, const vm::ram::Mem& ram);

int main(int argc, char* argv[]) {
    using namespace vm;
    using namespace vm::cpu;

    byte_t* rom = nullptr;
    size_t rom_size = 0;

    VmParamaters options(argc, (const char**)argv); // Parse parameters

    if (options.valid_params == false) {
        return -1;
    } else if (options.ask_help) {
        return 0;
    }

    rom = new byte_t[32*1024];

    std::printf("Opening ROM image %s\n", options.rom_file);

    int size = vm::aux::LoadROM(options.rom_file, rom);
    if (size < 0) {
        std::fprintf(stderr, "An error occurred while reading file %s\n", argv[1]);
        return -1;
    }

    std::printf("Read %d bytes and stored in ROM\n", size);
    rom_size = size;

    // Create the Virtual Machine
    VComputer vc(options.ram_size);
    std::printf("Creating Virtual Computer with %d KiB of RAM\n", options.ram_size / 1024);

    if (options.cpu == CpuToUse::DCPU16N) {
        std::printf("Using CPU DCPU-16N\n");
        std::unique_ptr<vm::ICPU> cpu(new DCPU16N(options.clock) );
        vc.SetCPU(std::move(cpu));
    } else {
        std::printf("Using CPU TR3200\n");
        std::unique_ptr<vm::ICPU> cpu(new TR3200(options.clock) );
        vc.SetCPU(std::move(cpu));
    }
    std::printf("CPU clock speed set to %u KHz \n", options.clock / 1000);
    vc.SetROM(rom, rom_size);

    // Add devices to the Virtual Machine
    auto gcard = std::make_shared<vm::dev::tda::TDADev>();
#ifdef GLFW3_ENABLE
    vm::dev::tda::TDAScreen gcard_screen = {0};
#endif
    vc.AddDevice(5, gcard);

    auto gk = std::make_shared<vm::dev::gkeyboard::GKeyboardDev>();
    vc.AddDevice(4, gk);

    auto ddev = std::make_shared<vm::DummyDevice>();
    vc.AddDevice(10, ddev);

    // Floppy drive
    auto fd = std::make_shared<vm::dev::m5fdd::M35FD>();
    vc.AddDevice(6, fd);
    auto floppy = std::make_shared<vm::dev::m5fdd::M35_Floppy>(options.dsk_file);
    std::printf("Floppy image file %s \n", options.dsk_file);
    fd->insertFloppy(floppy);

    while (! options.breaks.empty()) {
        auto brk = options.breaks.back();
        options.breaks.pop_back();
        vc.SetBreakPoint(brk);
        std::printf("Inserting break point at 0x%06X\n", brk);
    }

#ifdef OPENAL_ENABLE
    AlEngine al;
    if (al.Init() ) {
        std::cout << "OpenAL Initiated\n";
    } else {
        std::cerr << "Error initializasing OpenAL\n";
    }
#endif

    vc.On();  // Powering it !

    std::cout << "Run program (r) or Step Mode (s) ?\n";
    char mode;
    std::cin >> mode;
    std::getchar();


    bool debug = false;
    if (mode == 's' || mode == 'S') {
        debug = true;
    }

#ifdef GLFW3_ENABLE
    GlEngine gl;
    OS::OS glfwos;
    if (!glfwos.InitializeWindow(1024, 768, "Trillek Virtual Computer demo emulator")) {
        std::clog << "Failed creating the window or context.";
        return -1;
    }

    if (gl.initGL(glfwos) != 0) {
        std::cerr << "Error initiating OpenGL\n";
        return -1;
    }

    std::printf("Initiated OpenGL\n");

    KeyEventHandler keyhandler;
    keyhandler.gk = gk;
    glfwos.RegisterKeyboardEventHandler(&keyhandler);

    gl.SetTextureCB ([&gcard, &gcard_screen] (void* tdata) {
        // Update Texture callback
        gcard->DumpScreen (gcard_screen);
        gcard->DoVSync();

        vm::dword_t* tex = (vm::dword_t*)tdata;
        TDAtoRGBATexture(gcard_screen, tex); // Write the texture to the PBO buffer
    });

#endif

    std::cout << "Running!\n";
    unsigned ticks = 16050;
    unsigned long ticks_count = 0;

    using namespace std::chrono;
    auto clock = high_resolution_clock::now();
    double delta; // Time delta in seconds

    int c = ' ';
    bool loop = true;
    vm::cpu::TR3200State cpu_state;

    // Delay here to enforce initial delta != 0
    for (long i=0; i< 600000; i++) {
        ;
    }

    while ( loop) {
        // Calcs delta time

        auto oldClock = clock;
        clock = high_resolution_clock::now();
        if (!debug) { // Calc delta
            delta = duration_cast<milliseconds>(clock - oldClock).count();
        } else {
            delta = 0.04;
        }



#ifdef GLFW3_ENABLE
        if (glfwos.Closing()) {
            loop = false;
            continue;
        }
#endif

        if (debug) { // Print PC and instruction BEFORE executing it
            vc.GetState((void*) &cpu_state, sizeof(cpu_state));
            print_pc(cpu_state, vc);
        }

        if (!debug) {
            double ds = delta / 1000.0;
            if (ds == 0) {
                std::fprintf(stderr, "Wops! ds is 0 seconds !!!\t delta = %f\n", delta);
                ds = 0.0000001;
            }
            ticks_count += ticks;
            ticks = vc.Update(ds);

            // Speed info
            if (ticks_count > 400000) {
                std::printf("Running %u cycles in %f ms ", ticks, delta);
                double ttick = delta / ticks;
                const double tclk = 1000.0 / 1000000.0; // Base clock 1Mhz
                std::printf("Ttick %f ms ", ttick); // Time take in one cycle
                std::printf("Tclk %f ms ", tclk); // Time that should take one cycle
                std::printf("Speed of %f %% \n", 100.0f * (tclk / ttick) );
                ticks_count -= 400000;
            }

        } else {
            ticks = vc.Step(delta / 1000.0 );

            vc.GetState((void*) &cpu_state, sizeof(cpu_state));
            print_regs(cpu_state);
            //print_stack(vm.CPU(), vm.RAM());
        }

        if (vc.isHalted()) { // Here is a breakpoint !
            std::printf("\t\tBREAKPOINT\n\nSwithching to Step mode\n\n");
            debug = true;

            vc.GetState((void*) &cpu_state, sizeof(cpu_state));
            print_pc(cpu_state, vc);
        }

        if (debug) {

            while (1) { // Parse "command"
                c = std::getchar();

                if (c == 'q' || c == 'Q') {
                    loop = false;
                    break;
                }

                if (c == 'r' || c == 'R') {
                    clock = high_resolution_clock::now();
                    vc.Resume();
                    vc.Step(delta / 1000.0 );
                    debug = false;
                    break;
                }

                if (c == 's' || c == 'S' || c =='\n') {
                    vc.Resume();
                    debug = true;
                    break;
                }
            }
#ifdef GLFW3_ENABLE
            gl.UpdScreen (glfwos, 0);
#endif
        } else {
#ifdef GLFW3_ENABLE
            gl.UpdScreen (glfwos, delta / 1000.0);
#endif
        }

    }

#ifdef GLFW3_ENABLE
    glfwos.Terminate();
#endif

    if (rom != nullptr) {
        delete[] rom;
    }
    return 0;
}

// Alias to special registers
#define REG_Y     (11)
#define BP        (12)
#define SP        (13)
#define REG_IA    (14)
#define REG_FLAGS (15)

// Operation in Flags bits
#define GET_CF(x)          ((x) & 0x1)
#define SET_ON_CF(x)       (x |= 0x1)
#define SET_OFF_CF(x)      (x &= 0xFFFFFFFE)

#define GET_OF(x)          (((x) & 0x2) >> 1)
#define SET_ON_OF(x)       (x |= 0x2)
#define SET_OFF_OF(x)      (x &= 0xFFFFFFFD)

#define GET_DE(x)          (((x) & 0x4) >> 2)
#define SET_ON_DE(x)       (x |= 0x4)
#define SET_OFF_DE(x)      (x &= 0xFFFFFFFB)

#define GET_IF(x)          (((x) & 0x8) >> 3)
#define SET_ON_IF(x)       (x |= 0x8)
#define SET_OFF_IF(x)      (x &= 0xFFFFFFF7)

// Enable bits that change what does the CPU
#define GET_EI(x)          (((x) & 0x100) >> 8)
#define SET_ON_EI(x)       (x |= 0x100)
#define SET_OFF_EI(x)      (x &= 0xFFFFFEFF)

#define GET_ESS(x)         (((x) & 0x200) >> 9)
#define SET_ON_ESS(x)      (x |= 0x200)
#define SET_OFF_ESS(x)     (x &= 0xFFFFFDFF)

// Internal alias to Y Flags and IA registers
#define RY      r[REG_Y]
#define IA      r[REG_IA]
#define FLAGS   r[REG_FLAGS]

void print_regs(const vm::cpu::TR3200State& state) {
    // Print registers

    for (int i=0; i < 11; i++) {
        std::printf("%%r%2d= 0x%08X ", i, state.r[i]);
        if (i == 3 || i == 7 || i == 11 || i == 15 || i == 19 || i == 23 || i == 27 || i == 31)
            std::printf("\n");
    }
    std::printf("%%y= 0x%08X\n", state.r[REG_Y]);

    std::printf("%%ia= 0x%08X ", state.r[REG_IA]);
    auto flags = state.r[REG_FLAGS];
    std::printf("%%flags= 0x%08X ", flags);
    std::printf("%%bp= 0x%08X ",  state.r[BP]);
    std::printf("%%sp= 0x%08X\n", state.r[SP]);

    std::printf("%%pc= 0x%08X \n", state.pc);
    std::printf("ESS: %d EI: %d \t IF: %d DE %d OF: %d CF: %d\n",
            GET_ESS(flags), GET_EI(flags), GET_IF(flags) , GET_DE(flags) , GET_OF(flags) , GET_CF(flags));
    std::printf("\n");

}

void print_pc(const vm::cpu::TR3200State& state, const vm::VComputer& vc) {
    vm::dword_t val = vc.ReadDW(state.pc);

    std::printf("\tPC : 0x%08X > 0x%08X ", state.pc, val);
    std::cout << vm::cpu::DisassemblyTR3200(vc,  state.pc) << std::endl;
}

/*
   void print_stack(const vm::cpu::TR3200& cpu, const vm::ram::Mem& ram) {
   std::printf("STACK:\n");

   for (size_t i =0; i <5*4; i +=4) {
   auto val = ram.RD(cpu.Reg(SP)+ i);

   std::printf("0x%08X\n", val);
   if (((size_t)(cpu.Reg(SP)) + i) >= 0xFFFFFFFF)
   break;
   }
   }
   */



