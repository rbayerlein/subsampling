// Inserts coincidence data from one dataset (e.g. point source) into another dataset
// Eric Berg; July 2019
// Edwin Leung; April 2020

#define BUFFER_SIZE 65536

#include <iostream>
#include <fstream>
#include <math.h> // deprecated - consider using <cmath>
#include <random>
#include <chrono>
#include <algorithm> // all_of

using namespace std;

struct ids {
	short txID1, axID1, txID2, axID2, tof;
};

class Buffer {
public:
	ids *pBuf;
	unsigned long long index;
	unsigned long long read_count;

	Buffer();
	virtual ~Buffer();
};

Buffer::Buffer() {
	pBuf = new ids[BUFFER_SIZE];
	index = 0;
	read_count = 0;
}

Buffer::~Buffer() {
	delete[] pBuf;
}

class BufferF {
public:
	float *pBuf;
	unsigned long long index;
	unsigned long long read_count;

	BufferF();
	virtual ~BufferF();
};

BufferF::BufferF() {
	pBuf = new float[BUFFER_SIZE];
	index = 0;
	read_count = 0;
}

BufferF::~BufferF() {
	delete[] pBuf;
}

int main(int argc, char **argv) {

	// check arguments
	if (argc != 10) {
		cout << "Usage: " << argv[0]
				<< " [fname_out] [fname1_in] ... [fname8_in]" << endl;
		cout << "All filenames must be specified with .lm extension." << endl;
		return 1;
	}

	// variables that are set by user in GUI
	cout << "Reading in user variables" << endl;
	string outname; // outname_p
	outname = argv[1]; // .lm
	string outname_s = outname.substr(0, outname.find_last_of("."))
			+ ".add_fac";
	string outname_m = outname.substr(0, outname.find_last_of("."))
			+ ".mul_fac";

	vector<string> inname_p(8, ""); // 8 files
	vector<string> inname_s(8, ""); // 8 files
	vector<string> inname_m(8, ""); // 8 files
	for (auto i = 0; i < 8; i++) { // 8 files
		inname_p.at(i) = argv[i + 2]; // i+2 because files 1-8 are argv[2-9] (0-index)
		inname_s.at(i) = inname_p.at(i).substr(0,
				inname_p.at(i).find_last_of(".")) + ".add_fac";
		inname_m.at(i) = inname_p.at(i).substr(0,
				inname_p.at(i).find_last_of(".")) + ".mul_fac";
	}

	// open input files
	cout << "Opening input files" << endl;
	FILE **pInFile_p = new FILE*[8]; // 8 files
	FILE **pInFile_s = new FILE*[8]; // 8 files
	FILE **pInFile_m = new FILE*[8]; // 8 files
	Buffer *pInBuf_p = new Buffer[8]; // 8 files
	BufferF *pInBuf_s = new BufferF[8]; // 8 files
	BufferF *pInBuf_m = new BufferF[8]; // 8 files

	for (auto i = 0; i < 8; i++) { // 8 files
		pInFile_p[i] = fopen(inname_p.at(i).c_str(), "rb"); // open list-mode file
		pInFile_s[i] = fopen(inname_s.at(i).c_str(), "rb"); // open .add_fac file
		pInFile_m[i] = fopen(inname_m.at(i).c_str(), "rb"); // open .mul_fac file
		if (pInFile_p[i] == NULL) {
			cout << "Cannot open " << inname_p.at(i) << endl;
			exit(1);
		}
		if (pInFile_s[i] == NULL) {
			cout << "Cannot open " << inname_s.at(i) << endl;
			exit(1);
		}
		if (pInFile_m[i] == NULL) {
			cout << "Cannot open " << inname_m.at(i) << endl;
			exit(1);
		}
	}

	cout << "Opening output files" << endl;
	// open output files
	FILE *pOutFile = fopen(outname.c_str(), "wb");
	FILE *pOutFile_s = fopen(outname_s.c_str(), "wb");
	FILE *pOutFile_m = fopen(outname_m.c_str(), "wb");
	Buffer outBuf;
	BufferF outBuf_s, outBuf_m;

	long long max_events_file = 0;

	vector<long long> file_size_p(8, 0); // 8 files
	vector<long long> num_events_p(8, 0); // 8 files


	cout << "getting number of events for each file" << endl;
	// get number of events for each file
	for (auto i = 0; i < 8; i++) { // 8 files
		ifstream ifs;
		ifs.open(inname_p.at(i).c_str(), ios::in | ios::binary); // open list-mode file
		ifs.seekg(0, ifs.end);
		file_size_p.at(i) = ifs.tellg(); //get size of list mode file
		num_events_p.at(i) = file_size_p.at(i) / 10; // 10-byte events
		if (num_events_p.at(i) > max_events_file) {
			max_events_file = num_events_p.at(i);
		}
		ifs.seekg(0, ifs.beg); // rewind to the beginning of the file
	}

	cout << "Calculating probability for each file" << endl;
	// calculate probability for each file
	vector<double> p_prob(8, 0); // 8 files
	for (auto i = 0; i < 8; i++) { // 8 files
		p_prob.at(i) = (double) num_events_p.at(i) / (double) max_events_file;
		cout << i << "\t" << p_prob.at(i) << endl;
		if ( p_prob.at(i) == 0) {
			p_prob.at(i) = 0.01;
			cout << "empty file found. Setting probability to " << p_prob.at(i) << endl;
		}
	}

	// **************		Main Run Program		***************************
	// ************************************************************************

	// random number generator
	cout << "Generating random number" << endl;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // why unsigned int not unsigned long long?
	default_random_engine generator(seed);
	uniform_real_distribution<double> distribution(0.0, 1.0);

	vector<bool> run_fread(8, true); // 8 files
	bool run = true;

	// initial read
	cout << "filling buffers" << endl;
	for (auto i = 0; i < 8; i++) { // 8 files
		pInBuf_p[i].read_count = fread(pInBuf_p[i].pBuf, sizeof(ids),
		BUFFER_SIZE, pInFile_p[i]);
		pInBuf_s[i].read_count = fread(pInBuf_s[i].pBuf, sizeof(float),
		BUFFER_SIZE, pInFile_s[i]);
		pInBuf_m[i].read_count = fread(pInBuf_m[i].pBuf, sizeof(float),
		BUFFER_SIZE, pInFile_m[i]);
	}

	// main loop
	while (run) {

		double r_num = distribution(generator); // generate random number

		for (auto i = 0; i < 8; i++) { // 8 files

		//	cout << "running file number " << i+1 << endl;
			// start probability check
			if (r_num < p_prob.at(i) && run_fread.at(i)) {
		//		cout << "correct r_num and run_fread is true" << endl;
				if (pInBuf_p[i].index < pInBuf_p[i].read_count) {
		//			cout << "buffer index smaller than read count" << endl;
		//			cout << "\t" << pInBuf_p[i].index << "\t" << pInBuf_p[i].read_count << endl;
					outBuf.pBuf[outBuf.index] =
							pInBuf_p[i].pBuf[pInBuf_p[i].index];
					pInBuf_p[i].index++;
					if (pInBuf_p[i].index == BUFFER_SIZE) {
						pInBuf_p[i].read_count = fread(pInBuf_p[i].pBuf,
								sizeof(ids),
								BUFFER_SIZE, pInFile_p[i]);
						pInBuf_p[i].index = 0;
					}
					outBuf.index++;
					if (outBuf.index == BUFFER_SIZE) {
						fwrite(outBuf.pBuf, sizeof(ids), BUFFER_SIZE, pOutFile);
						outBuf.index = 0;
					}

					outBuf_s.pBuf[outBuf_s.index] =
							pInBuf_s[i].pBuf[pInBuf_s[i].index];
					pInBuf_s[i].index++;
					if (pInBuf_s[i].index == BUFFER_SIZE) {
						pInBuf_s[i].read_count = fread(pInBuf_s[i].pBuf,
								sizeof(float),
								BUFFER_SIZE, pInFile_s[i]);
						pInBuf_s[i].index = 0;
					}
					outBuf_s.index++;
					if (outBuf_s.index == BUFFER_SIZE) {
						fwrite(outBuf_s.pBuf, sizeof(float), BUFFER_SIZE,
								pOutFile_s);
						outBuf_s.index = 0;
					}

					outBuf_m.pBuf[outBuf_m.index] =
							pInBuf_m[i].pBuf[pInBuf_m[i].index];
					pInBuf_m[i].index++;
					if (pInBuf_m[i].index == BUFFER_SIZE) {
						pInBuf_m[i].read_count = fread(pInBuf_m[i].pBuf,
								sizeof(float),
								BUFFER_SIZE, pInFile_m[i]);
						pInBuf_m[i].index = 0;
					}
					outBuf_m.index++;
					if (outBuf_m.index == BUFFER_SIZE) {
						fwrite(outBuf_m.pBuf, sizeof(float), BUFFER_SIZE,
								pOutFile_m);
						outBuf_m.index = 0;
					}
				} else { // feof
					cout << "Buffer index has reached read count. file " << i << endl;
					run_fread.at(i) = false;
					fclose(pInFile_p[i]);
					fclose(pInFile_s[i]);
					fclose(pInFile_m[i]);
				}
			} // end probability check
		} // end file for loop
		//cout << "checking feof for all files" << endl;
		// check feof for all files
		if (all_of(run_fread.begin(), run_fread.end(), [](bool i) {
			return !i;
		})) {
			run = false;
		}

	} // end main loop

	// flush buffers
	fwrite(outBuf.pBuf, sizeof(ids), outBuf.index, pOutFile);
	fwrite(outBuf_s.pBuf, sizeof(float), outBuf_s.index, pOutFile_s);
	fwrite(outBuf_m.pBuf, sizeof(float), outBuf_m.index, pOutFile_m);

	// clean up

	fclose(pOutFile);
	fclose(pOutFile_s);
	fclose(pOutFile_m);

	delete[] pInBuf_p;
	delete[] pInBuf_s;
	delete[] pInBuf_m;

	delete[] pInFile_p;
	delete[] pInFile_s;
	delete[] pInFile_m;

	cout << "Listmode data compilation complete." << endl;

	return 0;

}
