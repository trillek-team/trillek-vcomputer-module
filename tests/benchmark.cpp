/**
 * Trillek Virtual Computer - benchmark.cpp
 * Basic benchmark for Virtual Computer lib
 * Allow to see how many virtual computer can run in a single thread
 */
#include <VC.hpp>

#include <iostream>
#include <fstream>
#include <ios>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <cwctype>
#include <clocale>
#include <cstdlib>
#include <ctime>

#include <chrono>

#define N_CPUS 1000

int main(int argc, char* argv[]) {
	using namespace vm;
	using namespace vm::cpu;


	if (argc < 2) {
		std::printf("Usage: %s binary_file\n", argv[0]);
		return -1;

	}
	std::srand(std::time(0));

	unsigned troms = argc -1;
	byte_t* rom[troms];
	size_t rom_size[troms];

	for (unsigned i=0; i< troms; i++) {
		rom[i] = new byte_t[32*1024];

		std::printf("Opening file %s\n", argv[1 + i]);
    int size = vm::aux::LoadROM(argv[1], rom[i]);
    if (size < 0) {
      std::fprintf(stderr, "An error hapen when was reading the file %s\n", argv[1]);
      return -1;
    }

		std::printf("Read %u bytes and stored in ROM\n", size);
		rom_size[i] = size;
	}

  VComputer vc[N_CPUS];
	for (auto i=0; i< N_CPUS; i++) {
    // Add CPU
	  std::unique_ptr<vm::cpu::TR3200> cpu(new TR3200());
	  vc[i].SetCPU(std::move(cpu));

    // Add ROM
		auto rom_ptr = rom[i % troms];
		auto rom_s = rom_size[i % troms];
    vc[i].SetROM(rom_ptr, rom_s);

    // Add devices
    auto gcard = std::make_shared<vm::dev::tda::TDADev>();
    vc[i].AddDevice(0, gcard);

    // Reset
    vc[i].Reset();
	}

	std::cout << "Randomizing every CPU!\nExecuting a random number of cycles from 1 to 255\n";
	for (auto i=0; i< N_CPUS; i++) {
		vc[i].Tick((std::rand() % 255) + 1);
	}

	std::cout << "Running " << N_CPUS << " CPUs !\n";
	unsigned ticks = 500000;
	unsigned long ticks_count = 0;

	using namespace std::chrono;
	auto clock = high_resolution_clock::now();
	double delta;

	auto oldClock = clock;
	clock = high_resolution_clock::now();
	delta = duration_cast<microseconds>(clock - oldClock).count();

	while ( 1) {

		for (auto i=0; i< N_CPUS; i++) {
			vc[i].Tick(ticks, delta / 1000.0 );
    }

		ticks_count += ticks;

		auto oldClock = clock;
		clock = high_resolution_clock::now();
		delta = duration_cast<milliseconds>(clock - oldClock).count();


		// Speed info
		if (ticks_count > 800000) {
      std::printf("Running %u cycles in %f ms ", ticks, delta);
      double ttick = delta / ticks;
      double tclk = 1000.0 / 1000000.0; // Base clock 1Mhz
      std::printf("Ttick %f ms ", ttick);
      std::printf("Tclk %f ms ", tclk);
      std::printf("Speed of %f %% \n", 100.0f * (tclk / ttick) );
      ticks_count -= 200000;
		}
		//ticks = (vm[0].Clock() * delta * 0.000001) + 0.5f; // Rounding bug in VS

	}


  // Free ROMs
	for (unsigned i=0; i< troms; i++) {
		delete[] rom[i];
  }

	return 0;
}


