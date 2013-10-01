CXXFLAGS = -Wall -std=c++11 -g
.PHONY: clean

rc1600: main.cpp cpu.cpp ram.cpp dis_rc1600.cpp
	$(CXX) $(CXXFLAGS) -o rc1600 main.cpp cpu.cpp ram.cpp dis_rc1600.cpp

