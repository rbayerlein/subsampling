/*! \file subsample.hh
 \brief Implementation of Class Subsample.cpp 
 Subsampling of the EXPLORER data to mimic shorter scanners or damages or axial gaps for TO scanners
 This class implements a quasi-LUT to check if a coincidence between a pair of crystals A and B is to be kept for reconstruction or not. 
 Decision is made based on the crystal efficiency array created by the matlab script respiratory_gui.m
*/

#include "Subsample.h"
using namespace std;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Subsample::Subsample(std::string s, int t) /*!< Takes the full path to the .raw file as input. */
	: input_raw_fullpath(s)
	, firstTimeStamp(t)
{
	Initialize();
	debugCounter=0;
//	output_DEBUG= "DEBUG.txt";
//	o_DEBUG.open(output_DEBUG.c_str(), std::ios_base::app);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Subsample::~Subsample(){		// Destructor 

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Subsample::Initialize(){
	/// get file path to config file
	std::string str_find = "PET";
	std::string input_config_TEMP;
	size_t found = input_raw_fullpath.find(str_find);
	input_config_TEMP = input_raw_fullpath.substr(0, found);
	stringstream ss ;
	ss << input_config_TEMP << "UCD/multi_bed_recon.config" ;
	input_config = ss.str();
	cout << "====================================\nreading multi bed config file \n" << input_config << endl;


	ifstream config;
	config.open(input_config.c_str());
	if (!config) {
		cout << "could not open multi bed recon config file" << endl;
		exit(1);
	}
	std::string str_temp =  "";
	getline(config, str_temp);
	stringstream ss_temp(str_temp);
	ss_temp >> num_cycles;

	str_temp =  "";
	getline(config, str_temp);
	stringstream ss_temp2(str_temp);
	ss_temp2 >> start_ring;

	str_temp =  "";
	getline(config, str_temp);
	stringstream ss_temp3(str_temp);
	ss_temp3 >> rings_per_bed;

	str_temp =  "";
	getline(config, str_temp);
	stringstream ss_temp4(str_temp);
	ss_temp4 >> num_beds;	

	str_temp =  "";
	getline(config, str_temp);
	stringstream ss_temp5(str_temp);
	ss_temp5 >> bed_overlap;	


	str_temp =  "";
	getline(config, str_temp);
	stringstream ss_temp6(str_temp);
	ss_temp6 >>	time_per_bed;

	cout << "number of scans per bed position:\t" << num_cycles 
	<< "\nstart ring\t\t\t\t" << start_ring 
	<< "\nnumber of rings per bed position\t" << rings_per_bed
	<< "\nnumber of bed positions\t\t\t" << num_beds
	<< "\noverlap of bed positions\t\t" << bed_overlap 
	<< "\n===================================="<< endl;

	for (int cycle = 0; cycle < num_cycles; ++cycle)
	{
		for (int pos = 0; pos < num_beds; ++pos)
		{
		bed_time_start.push_back(cycle*time_per_bed*num_beds + pos*time_per_bed);
		bed_time_end.push_back(cycle*time_per_bed*num_beds + (pos+1)*time_per_bed);
		bed_ring_start.push_back(start_ring+pos*rings_per_bed);
		bed_ring_end.push_back(start_ring+(pos+1)*rings_per_bed-1);
		}
	}

	for (int i = 0; i < bed_time_start.size(); ++i)
	{
		cout << "start time " << i << ": " << bed_time_start.at(i) 
		<< ". end time: " << bed_time_end.at(i)
		<< ". start ring: " << bed_ring_start.at(i)
		<< ". end ring: " << bed_ring_end.at(i)
		<< endl;
	}

	cout << "first time stamp set: \t" << firstTimeStamp << "sec = " << (int)firstTimeStamp/3600 << "h:" << ((int)firstTimeStamp%3600)/60 << "min:" << ((int)firstTimeStamp%3600)%60 << "s" << endl;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
	bool Subsample::KeepEvent(int axA, int axB, int transA, int transB, int timeStamp) // Takes the axial and transaxial coordinates of the two coincident crystals as input.
	{ 
		int time_elapsed = timeStamp - firstTimeStamp;	// time elapsed since first time stamp. This essentially determines the bed position
		int pos = 0;
		while(pos < num_beds*num_cycles){	// go through all possible bed possitions and find the one with matching time frame
			if(time_elapsed>=bed_time_end.at(pos)) {
				pos++;
			}else{
				break;
			}
		}

		// if pos exceeds total number of bed positions, then time stamp of current event is larger than the maximum of the last bed position -> reject!
		if(pos >= num_beds*num_cycles) return false;	

		if(axA < bed_ring_start.at(pos) || axA > bed_ring_end.at(pos) || axB < bed_ring_start.at(pos) || axB > bed_ring_end.at(pos)) {
			if(debugCounter<20){
			//	debugCounter++;
				cout << time_elapsed << "\t" << pos << "\t" << bed_time_start.at(pos) << "\t" << bed_time_end.at(pos) << endl;
				cout << axA << "\t" << axB << "\t" << bed_ring_start.at(pos) << "\t" << bed_ring_end.at(pos) << " -> reject!" << endl;
			}
			return false;
		}else{
			if(debugCounter<20){
				debugCounter++;
				cout << time_elapsed << "\t" << pos << "\t" << bed_time_start.at(pos) << "\t" << bed_time_end.at(pos) << endl;
				cout << axA << "\t" << axB << "\t" << bed_ring_start.at(pos) << "\t" << bed_ring_end.at(pos) << " -> keep!" << endl;
			}
			return true;
		}

		// int linearCrysIndex_A, linearCrysIndex_B;

		// linearCrysIndex_A = num_ax_crys_mod * num_units * (transA - 1) + axA;
		// linearCrysIndex_B = num_ax_crys_mod * num_units * (transB - 1) + axB;
		
		// if(crys_eff_672x840[linearCrysIndex_A] > 100000 || crys_eff_672x840[linearCrysIndex_B] > 100000){
		// 	return false;
		// }
		// return true;
	}




//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/*
void Subsample::read_crys_eff(){

	cout << "Reading in crystal efficiency for LUT in Subsampling class..."<< endl;
//	o_DEBUG << "Reading in crystal efficiency for LUT in Subsampling class..."<< endl;
	/// The file to read in is here: /UCD/Image/crys_eff_679x840.
	/// take input file path and go back to level PET. In the same level there is UCD/Image/ and there is the crys_eff_679x840 file.
	/// BUT be careful! There is an additional 7 crystals in axial direction to compensate for the gap betweeen units.

/// get file path 
	std::string str_find = "PET";
	std::string input_crys_eff_TEMP;
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
	int column_counter=0;
/// loop through all entries in the BUFFER and skip the 7 inter-unit gaps. Write into array crys_eff_672x840[].
	for (int i = 0; i < BUFFER_size - NUM_dummy_crystals; ++i)
	{
		rows_counter++;
		if(rows_counter == 85){		// to skip the crystal ring which corresponds to the inter-unit gap 
			gaps_skipped++;	
			rows_counter=1;
		}
		if(i>1 && (i % 672 == 0)){
			
			rows_counter=1;
			gaps_skipped--;
			column_counter++;
		}
		crys_eff_672x840[i] = BUFFER[i+gaps_skipped];
	}
	std::string crysEff_out_filepath = "crys_eff_readIn";

	FILE *crysEff_Out = fopen(crysEff_out_filepath.c_str(), "wb");
	fwrite(crys_eff_672x840, sizeof(float), BUFFER_size - NUM_dummy_crystals, crysEff_Out);
	fclose(crysEff_Out);
	fclose(fInput);
	cout << "...done" << endl;
//	o_DEBUG << "...done" << endl;
}
*/
