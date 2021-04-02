bin = process_lm_sino_explorer_singlefile_release
inc = include
src = src
output_folder = temp_out

CXX = g++
CXXFLAGS = -std=c++14

$(bin) : $(src)/main.o $(inc)/coinc.o
	$(CXX) $(CXXFLAGS) -o $(bin) $(src)/main.o $(inc)/coinc.o

main.o : $(src)/.cpp $(inc)/coinc.h

$(inc)/coinc.o : $(inc)/coinc.cpp $(inc)/coinc.h $(inc)/bankPairLUT.h

.PHONY : clean

clean :
	-rm -v $(bin) $(src)/main.o $(inc)/coinc.o

wipe :
	-rm -v $(bin) $(src)/main.o $(inc)/coinc.o 
	-rm -v -r $(output_folder)/*