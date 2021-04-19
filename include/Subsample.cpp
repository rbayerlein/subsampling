/*! \file subsample.hh
 \brief Implementation of Class Subsample.cpp 
 Subsampling of the EXPLORER data to mimic shorter scanners or damages or axial gaps for TO scanners
 This class implements a quasi-LUT to check if a coincidence between a pair of crystals A and B is to be kept for reconstruction or not. 
 Decision is made based on the crystal efficiency array created by the matlab script respiratory_gui.m
*/

#include "Subsample.h"
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Subsample::Subsample(std::string s) /*!< Takes the full path to the .raw file as input. */
	: input_raw_fullpath(s)
	{
		for (int i = 0; i < (672*840); ++i)
		{
			input_crys_eff[i] = -1;
		}
		input_crys_eff = "empty";	// pre initialize. Good coding practice or so... I don't know. I'm not a programmer 
	read_crys_eff();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Subsample::~Subsample(){		// Destructor 

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void Subsample::read_crys_eff(){

	cout << "Reading in crystal efficiency for LUT in Subsampling class..."<< endl;
	/// The file to read in is here: /UCD/Image/crys_eff_679x840.
	/// take input file path and go back to level PET. In the same level there is UCD/Image/ and there is the crys_eff_679x840 file.
	/// BUT be careful! There is an additional 7 crystals in axial direction to compensate for the gap betweeen units.

/// get file path 
	string str_find = "PET";
	string input_crys_eff_TEMP;
	size_t found = input_raw_fullpath.find(str_find);
	input_crys_eff_TEMP = input_raw_fullpath.substr(0, found);
	stringstream ss ;
	ss << input_crys_eff_TEMP << "UCD/Image/crys_eff_679x840" ;
	input_crys_eff = ss.str();
	cout << input_crys_eff << endl;

/// open file
	FILE *fInput = fopen(input_crys_eff.c_str(), "rb");
	if( fInput == NULL ) {
		cerr << input_crys_eff << " cannot be opened. Return." << endl;			// maybe use a dummy file here, to prevent the recon process from crashing...
		exit(1);
	}
/// read into BUFFER	
	float *BUFFER = new float[BUFFER_size];	
	unsigned long long ct = fread(BUFFER, sizeof(float), BUFFER_size, fInput);

	int gaps_skipped = 0;
	int rows_counter = 0;
	int trans_crys_counter = 0;
/// loop through all entries in the BUFFER and skip the 7 inter-unit gaps. Write into array crys_eff_672x840[].
	for (int i = 0; i < BUFFER_size - NUM_dummy_crystals; ++i)
	{
		trans_crys_counter++;
		if (trans_crys_counter > 840) {
			rows_counter ++;
			trans_crys_counter=1;
		}
		if(rows_counter == 85){		// to skip the crystal ring which corresponds to the inter-unit gap 
			gaps_skipped++;	
			cout << "skipping gap number " << gaps_skipped << endl;
			rows_counter=0;
		}
		crys_eff_672x840[i] = BUFFER[i+gaps_skipped*num_trans_crys_ring];
	}
	fclose(fInput);
	cout << "...done" << endl;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
	bool Subsample::KeepEvent(int axA, int axB, int transA, int transB) // Takes the axial and transaxial coordinates of the two coincident crystals as input.
	{ 
		int linearCrysIndex_A, linearCrysIndex_B;
		linearCrysIndex_A = axA*num_ax_crys_mod + transA;
		linearCrysIndex_B = axB*num_trans_crys_ring + transB;
	//	if(crys_eff_672x840[linearCrysIndex_A] == 0 || crys_eff_672x840[linearCrysIndex_B] == 0) keep = false;
		
//	ALTERNATIVE: for later, when the crystal efficiency is set to a very high value instead of zero
		if(crys_eff_672x840[linearCrysIndex_A] > 100000 || crys_eff_672x840[linearCrysIndex_B] > 100000) return false;
		
		return true;
	}
