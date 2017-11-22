CXX=g++
CXXFLAGS=-Wall -std=c++11
SRCS=processor.cpp components.cpp

all:
	$(CXX) $(CXXFLAGS) -o processor processor.cpp gui.cpp components.cpp -lmenu -lncurses

clean:
	rm processor
