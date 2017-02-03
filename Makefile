CXX = g++
LIB = -L/usr/local/lib -Wl,-rpath=/usr/local/lib

CXXFLAGS = -Wall -c -std=c++11 -Iinclude
LDFLAGS = $(LIB)
EXE = Client

all: $(EXE)

$(EXE): client.o
	$(CXX) $< $(LDFLAGS) -o $@

Lesson1/main.o: client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm *.o && rm $(EXE)
