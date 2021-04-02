bin = process_lm_sino_explorer_singlefile_release
bin_TS = test
# TS means is test space
inc = include
src = src
output_folder = temp_out

CXX = g++
CXXFLAGS = -std=c++14

all : $(bin) $(bin_TS)
$(bin) : $(src)/main.o $(inc)/coinc.o
	$(CXX) $(CXXFLAGS) -o $(bin) $(src)/main.o $(inc)/coinc.o

main.o : $(src)/.cpp $(inc)/coinc.h

$(bin_TS) : $(src)/Testspace.o $(inc)/coinc.o $(inc)/Subsample.o
	$(CXX) $(CXXFLAGS) -o $(bin_TS) $(src)/Testspace.o $(inc)/coinc.o $(inc)/Subsample.o

$(inc)/coinc.o : $(inc)/coinc.cpp $(inc)/coinc.h $(inc)/bankPairLUT.h

$(inc)/Subsample.o : $(inc)/Subsample.cpp $(inc)/Subsample.h

.PHONY : clean

clean :
	-rm -v $(bin) $(bin_TS) $(src)/*.o $(inc)/*.o

wipe :
	-rm -v $(bin) $(bin_TS) $(src)/*.o $(inc)/*.o 
	-rm -v -r $(output_folder)/*