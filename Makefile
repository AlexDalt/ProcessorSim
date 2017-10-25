CXX=g++
CXXFLAGS=-Wall

all:
	$(CXX) $(CXXFLAGS) -o processor processor.cpp

clean:
	rm processor
