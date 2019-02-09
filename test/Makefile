
CXX_FLAGS= -std=c++14 -I. -I../src

all: tests

clean:
	rm -f *.o
	rm -f tests

catch.hpp:
	wget https://github.com/catchorg/Catch2/releases/download/v2.6.0/catch.hpp

tests: main.o Color.o colorConversion.o
	g++ $(CXX_FLAGS) $^ -o $@

%.o: %.cpp catch.hpp
	g++ -c $(CXX_FLAGS) $< -o $@

Color.o: ../src/Color.cpp
	g++ -c $(CXX_FLAGS) $< -o $@


