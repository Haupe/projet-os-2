CXXFLAGS+=-std=c++17 -g -Wall -Wextra -Wpedantic -D_GNU_SOURCE 
#CXX=g++
LDLIBS+=-lpthread
SOURCES = $(wildcard *.cpp */*.cpp)
HEADERS = $(wildcard *.hpp */*.hpp)

OBJ=db.o student.o

.PHONY: main
main: smalldb check run

smalldb: smalldb.cpp queries.o db.o student.o
	$(CXX) $(LDFLAGS) smalldb.cpp -o queries.o db.o student.o $(LOADLIBES) $(LDLIBS)


queries.o: queries.cpp queries.hpp db.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c queries.cpp

db.o: db.cpp db.hpp student.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c db.cpp

student.o: student.cpp student.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c student.cpp

.PHONY: clean
clean:
	-rm *.o
