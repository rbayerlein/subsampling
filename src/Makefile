bin = ../combine_listmode

CXX = g++
CXXFLAGS = -std=c++14

$(bin) : main_combine_lm.o
	$(CXX) $(CXXFLAGS) -o $(bin) main_combine_lm.o

main_combine_lm.o : main_combine_lm.cpp

.PHONY : clean

clean :
	-rm -v $(bin) main_combine_lm.o
