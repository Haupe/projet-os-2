CXXFLAGS+=-std=c++17 -g -Wall -Wextra -Wpedantic -D_GNU_SOURCE 
#CXX=g++
LDLIBS+=-lpthread
SOURCES = $(wildcard *.cpp */*.cpp)
HEADERS = $(wildcard *.hpp */*.hpp)

OBJ=db.o student.o queries.o

.PHONY: main
main: smalldb sdbsh

smalldb: smalldb.cpp ${OBJ}
	$(CXX) $(LDFLAGS) $^ -o $@ $(LOADLIBES) $(LDLIBS)

%.o: %.cpp %.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^

.PHONY: sdbsh
sdbsh: client.cpp 
	$(CXX) $(LDFLAGS) $^ -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	-rm *.o & rm sdbsh