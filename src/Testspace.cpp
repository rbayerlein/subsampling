#include <iostream>
#include "../include/coinc.h"

#define BUFFER_SIZE 65536	// in bytes

using namespace std;

int main() {
	cout << "Start..." << endl;

	string infile_fullpath = "/run/user/1000/gvfs/afp-volume:host=EXPLORER_PHYS.local,user=Phys,volume=EXPLORER_PHYS_MAIN/Shared_EXPLORER_Data/20200124/cylinder_phantom3_E-20200124-172216-11_172342/PET/RawData/1.2.156.112605.18587648329783.200125012343.9.6520.125134/1.2.156.112605.18587648329783.200125012731.9.13476.18310.8.raw";
	FILE *pInputFile = fopen(infile_fullpath.c_str(), "rb");
	if (pInputFile == NULL) { // check return value of file pointer
		cerr << infile_fullpath << " cannot be opened." << endl;
		exit(1);
	}

	uint64_t *pRawBuffer = new uint64_t[BUFFER_SIZE];
	unsigned long long byte_location = 0; // for event identification

	int year00, month00, day00, hour00, minute00, second00, milli00 = 0;

	bool found_first_timestamp = false;
	while (!found_first_timestamp) { // stop when first timestamp found
		unsigned long long read_count = fread(pRawBuffer, sizeof(uint64_t), BUFFER_SIZE, pInputFile); // returns BUFFER_SIZE events; read unless EOF
				// reads n elements (n=BUFFER_SIZE) into a block of memory (specified as pRawBuffer), each with a size of "sizeof(uint64_t)"
				// uint64_t is unsigned int with exactly 64 bit
		for (unsigned long long i = 0; i < read_count; i++) { // loop through each event
			// start check event type:
			if (COINC::IsTimestamp(pRawBuffer[i])) {
				year00 = COINC::GetYear(pRawBuffer[i]);
				month00 = COINC::GetMonth(pRawBuffer[i]);
				day00 = COINC::GetDay(pRawBuffer[i]);
				hour00 = COINC::GetHour(pRawBuffer[i]);
				minute00 = COINC::GetMinute(pRawBuffer[i]);
				second00 = COINC::GetSecond(pRawBuffer[i]);
				milli00 = COINC::GetMillisecond(pRawBuffer[i]);
				found_first_timestamp = true;
				cout << "found first time stamp at byte location " << byte_location << endl;
				char strout[1024];
				sprintf(strout, "%s%dY-%dM-%dD-%dh-%dm-%ds-%dms.", "time stamp: ", year00, month00, day00, hour00, minute00, second00, milli00);
				cout << strout << endl;
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