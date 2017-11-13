CXX=g++
CXXFLAGS=-Wall
SRCS=processor.cpp components.cpp

all:
	$(CXX) $(CXXFLAGS) -o processor processor.cpp gui.cpp components.cpp -lncurses

clean:
	rm processor
