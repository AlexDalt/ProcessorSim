CXX=g++
CXXFLAGS=-Wall
SRCS=processor.cpp components.cpp

all:
	$(CXX) $(CXXFLAGS) -o processor processor.cpp components.cpp

clean:
	rm processor
