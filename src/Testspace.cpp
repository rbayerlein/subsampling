#include <iostream>
#include <fstream>
#include <sstream>

#include "../include/coinc.h"
#include "../include/Subsample.h"
//#include "../include/Subsample.cpp"

#include <math.h> // actually would use <cmath> but this is used in the main function. so keep it.
//#include <cmath>

#define BUFFER_SIZE 65536	// in bytes

using namespace std;


uint64_t *pRawBuffer = new uint64_t[BUFFER_SIZE];
unsigned long long byte_location = 0; // for event identification

int year00, month00, day00, hour00, minute00, second00, milli00 = 0;

int crys1, crys2 = 0;
int transA, transB, axA, axB = 0;
int bankk = 0;
int modA, modB = 0;
int unitA, unitB = 0;

string infile_fullpath;
string outfolder;

	/// ************	Main Function Start  ****************///


int main(int argc, char* argv[]) {
	cout << "Start..." << endl;

	if (argc != 2) {
		cout << "Usage: " << argv[0] << " [scan_details_fullpath]" << endl;
		return 1;
	}


// Read config file
	string scan_details_fullpath;
	scan_details_fullpath = argv[1];

	ifstream detes;
	detes.open(scan_details_fullpath.c_str());
	if (!detes) {
		cout << "could not open read listmode config file" << endl;
		return 1;
	}

	int subint = 0;
	float subtime = 0.0;

	string str_temp = "";

	// path to output folder
	getline(detes, str_temp);
	outfolder = str_temp;
	str_temp = "";

	// path to .lm data
	getline(detes, str_temp);
	infile_fullpath = str_temp;
  

	FILE *pInputFile = fopen(infile_fullpath.c_str(), "rb");
	if (pInputFile == NULL) { // check return value of file pointer
		cerr << infile_fullpath << " cannot be opened." << endl;
		exit(1);
	}


	/// ************		Load LUTs  ****************///
	string fdir_code = "lut/";

	// open bank lut
	int lutsum = 0;
	string bank_lut_path = fdir_code;
	bank_lut_path.append("bank_lut");
	ifstream lut_read;
	lut_read.open(bank_lut_path.c_str(), ios::in | ios::binary);
	if (!lut_read) {
		cout << "could not open bank LUT file" << endl;
		cout << bank_lut_path << endl;
		return 1;
	}
	int bank_lut[54][2];
	for (int ii = 0; ii < 108; ii++) {
		if (ii < 54) {
			lut_read.read(reinterpret_cast<char*>(&bank_lut[ii][0]),
					sizeof(int));
			lutsum = lutsum + bank_lut[ii][0];
		} else {
			lut_read.read(reinterpret_cast<char*>(&bank_lut[ii - 54][1]),
					sizeof(int));
			lutsum = lutsum + bank_lut[ii - 54][1];
		}
	}

	if (lutsum != 594) {
		cout << "Bank LUT incorrect!" << endl;
		return 1;
	}

	//Subsample* SUBS;
	//SUBS = new Subsample(temp_str);
	Subsample SUBS(infile_fullpath);


	/// ************	Main Test Space Start  ****************///
	int eventCounter=0;
	while (eventCounter < 100) { // stop when first timestamp found
		unsigned long long read_count = fread(pRawBuffer, sizeof(uint64_t), BUFFER_SIZE, pInputFile); // returns BUFFER_SIZE events; read unless EOF
				// reads n elements (n=BUFFER_SIZE) into a block of memory (specified as pRawBuffer), each with a size of "sizeof(uint64_t)"
				// uint64_t is unsigned int with exactly 64 bit
		for (unsigned long long i = 0; i < read_count; i++) { // loop through each event
			// start check event type:
			if (COINC::IsCoinc(pRawBuffer[i])) {
				unitA = COINC::GetUiA(pRawBuffer[i]);	// 0:7 NOT 1:8
				unitB = COINC::GetUiB(pRawBuffer[i]);

				crys1 = COINC::GetCrysA(pRawBuffer[i]);
				crys2 = COINC::GetCrysB(pRawBuffer[i]);

				bankk = COINC::GetBankPair(pRawBuffer[i]) - 1;	// returns bank pair index 0:53

				transA = crys1 % 70; // transaxial crystal ID within a bank
				transB = crys2 % 70;

				modA = bank_lut[bankk][0];	// returns bank A
				modB = bank_lut[bankk][1];	// and bank B

				transA = transA + (modA * 70);	// absolute transaxial crystal ID A
				transB = transB + (modB * 70);	// there are 70 transaxial crystals per bank (bank = 2*module)

				axA = floor(crys1 / 70) + (unitA * 84);	// absolute axial crystal ID A
				axB = floor(crys2 / 70) + (unitB * 84);	// there are 84 axial crystals per module/unit
				cout << "event " << eventCounter << endl;
				cout << "coincidences A,B (trans/ax):\t(" << transA << "/" << axA << "),\t(" << transB << "/" << axB << ")" << endl;
				cout << "keep event (1=yes):\t" << SUBS.KeepEvent(axA, axB, transA, transB) << endl;

				eventCounter++;
				//char strout[1024];
				//sprintf(strout, "%s%dY-%dM-%dD-%dh-%dm-%ds-%dms.", "time stamp: ", year00, month00, day00, hour00, minute00, second00, milli00);
				//cout << strout << endl;
				byte_location+=8;
				break;
			}else if (COINC::IsBedPos(pRawBuffer[i])) {
				byte_location += 8; // move to next event (8 bytes)
				continue;
			} else if (COINC::IsBlockRate(pRawBuffer[i])) {
				byte_location += 8; // move to next event (8 bytes)
				continue;
			} else if (COINC::IsCoinc(pRawBuffer[i])) {
				byte_location += 8; // move to next event (8 bytes)
				continue;
			} else if (COINC::IsPdRate(pRawBuffer[i])) {
				byte_location += 8; // move to next event (8 bytes)
				continue;
			} else {
				cout << "Unknown event detected at " << byte_location << endl;
				byte_location += 8; // move to next event (8 bytes) - continue for loop
			}
		}
	}

	fclose(pInputFile);
	cout << "done." << endl;
}