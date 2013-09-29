CXXFLAGS = -Wall -std=c++11
.PHONY: clean

rc1600: main.cpp cpu.cpp
	$(CXX) $(CXXFLAGS) -o rc1600 main.cpp cpu.cpp 

