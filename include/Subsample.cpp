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
	createLogFile = false;
	//pre-initialize LUT with zeros
	for (int i = 0; i < 679; ++i)
	{
		for (int j = 0; j < 679; ++j)
		{
			ExposureLUT[i][j] = 0;
		}
	}
	Initialize();

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

	if (createLogFile)
	{
		/// get system time to create timestamp for log file name
		std::time_t t = std::time(0);   	// get time now
	    std::tm* now = std::localtime(&t);

		/// set logfile out path
		stringstream ss_log;
		ss_log << input_config_TEMP << "UCD/subsample_" << now->tm_year+1900 << "-" << now->tm_mon + 1 
			<< "-" << now->tm_mday << "_" << now->tm_hour << ":" << now->tm_min << ".log" ;
		output_LOG = ss_log.str();
		o_LOG.open(output_LOG.c_str(), std::ios_base::app);

		o_LOG << input_raw_fullpath << endl;
	}

	cout << "====================================\nreading multi bed config file \n" << input_config << endl;

	ifstream config;
	config.open(input_config.c_str());
	if (!config) {
		cout << "could not open multi bed recon config file" << endl;
		exit(1);
	}
	for (int i = 0; i < 6; ++i)
	{
		std::string str_temp =  "";
		getline(config, str_temp);
		config_param[GetType(str_temp)] = GetParameterValue(str_temp);
	}

	num_cycles = config_param[0];
	start_ring = config_param[1];
	rings_per_bed = config_param[2];
	num_beds = config_param[3];
	bed_overlap = config_param[4];
	time_per_bed = config_param[5];



	cout << "====================================\nSUMMARY OF CONFIG PARAMETERS"<< endl;
	cout << "number of scans per bed position:\t" << num_cycles 
	<< "\nstart ring\t\t\t\t" << start_ring 
	<< "\nnumber of rings per bed position\t" << rings_per_bed
	<< "\nnumber of bed positions\t\t\t" << num_beds
	<< "\noverlap of bed positions\t\t" << bed_overlap 
	<< "\ntime per bed\t\t\t\t" << time_per_bed
	<< "\n===================================="<< endl;


	for (int cycle = 0; cycle < num_cycles; ++cycle)
	{
		for (int pos = 0; pos < num_beds; ++pos)
		{
		bed_time_start.push_back(cycle*time_per_bed*num_beds + pos*time_per_bed);
		bed_time_end.push_back(cycle*time_per_bed*num_beds + (pos+1)*time_per_bed);
		int this_ring_start = start_ring+pos*(rings_per_bed-bed_overlap);
		bed_ring_start.push_back(this_ring_start);
		bed_ring_end.push_back(this_ring_start+rings_per_bed-1);
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
	cout << "will create exposure LUT next"<< endl;

// fill log file if desired
	if (createLogFile)
	{
		o_LOG << "number of scans per bed position:\t" << num_cycles 
		<< "\nstart ring\t\t\t\t" << start_ring 
		<< "\nnumber of rings per bed position\t" << rings_per_bed
		<< "\nnumber of bed positions\t\t\t" << num_beds
		<< "\noverlap of bed positions\t\t" << bed_overlap 
		<< "\ntime per bed\t\t\t\t" << time_per_bed
		<< "\n===================================="<< endl;

		for (int i = 0; i < bed_time_start.size(); ++i)
		{
			o_LOG << "start time " << i << ": " << bed_time_start.at(i) 
			<< ". end time: " << bed_time_end.at(i)
			<< ". start ring: " << bed_ring_start.at(i)
			<< ". end ring: " << bed_ring_end.at(i)
			<< endl;
		}

		o_LOG << "first time stamp set: \t" << firstTimeStamp << "sec = " << (int)firstTimeStamp/3600 << "h:" << ((int)firstTimeStamp%3600)/60 << "min:" << ((int)firstTimeStamp%3600)%60 << "s" << endl;
		o_LOG << "will create exposure LUT next"<< endl;
		
	}

	FillExposureTable();	// fill LUT with amount of time each crystal ring pair is exposed to activity and contributes to the list mode data set

	cout << "done" << endl;
	if (createLogFile){
		o_LOG << "done" << endl;
		o_LOG.close();
	}

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
	bool Subsample::KeepEvent(int axA, int axB, int transA, int transB, int timeStamp) // Takes the axial and transaxial coordinates of the two coincident crystals as input.
	{
		// adjust input indexing (0-indexing) to indexing used in this subsample code (matlab indexing starting at 1)
		axA++;
		axB++;
		transA++;
		transB++;
		
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
	//		cout << time_elapsed << "\t" << pos << "\t" << bed_time_start.at(pos) << "\t" << bed_time_end.at(pos) << endl;
	//		cout << axA << "\t" << axB << "\t" << bed_ring_start.at(pos) << "\t" << bed_ring_end.at(pos) << " -> reject!" << endl;
			return false;
		}else{
	//		cout << time_elapsed << "\t" << pos << "\t" << bed_time_start.at(pos) << "\t" << bed_time_end.at(pos) << endl;
	//		cout << axA << "\t" << axB << "\t" << bed_ring_start.at(pos) << "\t" << bed_ring_end.at(pos) << " -> keep!" << endl;			
			return true;
		}


	}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int Subsample::GetType(string s){
	string str_find = "=";
	size_t found = s.find(str_find);
	string s_type = s.substr(0,found);
	if( s_type == "num_cycles_per_bed"){
			cout << "type: " << s_type ;
			return 0;
	}else if(s_type == "start_ring"){
			cout << "type: " << s_type ;
			return 1;
	}else if (s_type ==  "num_rings_per_bed"){
			cout << "type: " << s_type ;
			return 2;
	}else if(s_type ==  "num_beds"){
			cout << "type: " << s_type ;
			return 3;
	}else if(s_type == "num_rings_overlap"){
			cout << "type: " << s_type ;
			return 4;
	}else if(s_type == "time_per_bed"){
			cout << "type: " << s_type ;
			return 5;
	}else{
			cout << "unknown parameter type" << endl;
			exit(1);
	}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int Subsample::GetParameterValue(string s){
	string str_find = "=";
	size_t found = s.find(str_find);
	string s_val = s.substr(found+1,s.length());
	stringstream ss_temp(s_val);
	int val = 0;
	ss_temp >> val;
	cout << "; value: " << val << endl;
	return val;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Subsample::FillExposureTable(){
	// exposure time is in seconds
	// loop over all bed positions
	for (int bed = 1; bed < num_beds+1; ++bed)
	{
//		cout << "bed " << bed << endl;
		int this_bed_start = start_ring + (bed-1)*(rings_per_bed-bed_overlap);
		int this_bed_end = this_bed_start + rings_per_bed-1;
//		cout << "this_bed_start: " << this_bed_start << "; this_bed_end: " << this_bed_end << endl;
		for(int i = this_bed_start; i <this_bed_end+1; ++i){
			for(int j = this_bed_start; j <this_bed_end+1; ++j){
				ExposureLUT[i-1][j-1] += time_per_bed*num_cycles;	// [i-1] and [j-1] because C++ uses zero indexing
			}
		}
	}// end of loop over bed positions
}// end of function bracket