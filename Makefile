CXX = g++
LIB = -L/usr/local/lib -Wl,-rpath=/usr/local/lib -lusb-1.0

CXXFLAGS = -Wall -c -std=c++11 -Iinclude -MMD
LDFLAGS = $(LIB)
EXE = Client

INCLUDE_CPP = CCrazyflie.cpp CCrazyRadio.cpp CCRTPPacket.cpp CTOC.cpp
INCLUDE_OBJ = CCrazyflie.o CCrazyRadio.o CCRTPPacket.o CTOC.o

all: $(EXE)

$(EXE): client.o $(INCLUDE_OBJ)
	$(CXX) $< $(INCLUDE_OBJ) $(LDFLAGS) -o $@

client.o: client.cpp $(INCLUDE_CPP)
	$(CXX) $(CXXFLAGS) $< $(INCLUDE_CPP) -o $@

obj/%.o: $(INCLUDE_CPP)
	g++ $(CXXFLAGS) -c -o $@ $(INCLUDE_CPP)

clean:
	rm *.o && rm *.d && rm $(EXE)

-include $(INCLUDE_OBJ:.o=.d)
