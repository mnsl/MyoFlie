CXX = g++
LIB = -L/usr/local/lib -Wl,-rpath=/usr/local/lib

CXXFLAGS = -Wall -c -std=c++11 -Iinclude -MMD
LDFLAGS = $(LIB)
EXE = Client

INCLUDE_CPP = $(wildcard *.cpp)
INCLUDE_OBJ = $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

all: $(EXE)

$(EXE): client.o #$(INCLUDE_OBJ)
	$(CXX) $< $(LDFLAGS) -o $@

client.o: client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

obj/%.o: include/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

clean:
	rm *.o && rm $(EXE)

-include $(INCLUDE_OBJ:.o=.d)
