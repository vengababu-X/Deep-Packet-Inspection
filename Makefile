CXX=g++
CXXFLAGS=-Wall -Iinclude
LIBS=-lpcap

SRC=src/main.cpp src/Sniffer.cpp src/Parser.cpp src/FlowTracker.cpp src/DPI.cpp src/Stats.cpp
OUT=packet_analyzer

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)
