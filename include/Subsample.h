/*!	
	\file subsample.hh
	\brief Definition of Class Subsample.cpp 

	Subsampling of the EXPLORER data to mimic shorter scanners or damages or axial gaps for TO scanners
	This class implements a quasi-LUT to check if a coincidence between a pair of crystals A and B is to be kept for reconstruction or not. 
	Decision is made based on the crystal efficiency array created by the matlab script respiratory_gui.m
*/

#ifndef SUBS_V1
#define SUBS_V1

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <vector>
#include <ctime>

using namespace std;

class Subsample {
public:
	Subsample(std::string s, int t);										/*!< Constructor. Invokes read_crys_eff() to read in the crystal efficiencies from file. */
	virtual ~Subsample();											/*!< Destructor */
	bool KeepEvent(int axA, int axB, int transA, int transB, int timeStamp); 		/*!< Function that decides if that specific crystal pair specified by the given coordinates shall be kept.
																	Takes the axial and transaxial coordinates of the two coincident crystals as input.
																	*/

	std::string GetRawInputFullPath(){return input_raw_fullpath;};	
	int GetType(std::string s_temp);								/*!< Function that takes the string from the config file and returns the number of the type*/
	int GetParameterValue(std::string s_temp);							/*!< Function that takes the string from the config file and returns the parameter value*/

	int GetTimePerBed(){return time_per_bed;};
	int GetBedOverlap(){return bed_overlap;};
	int GetNumBeds(){return num_beds;};
	int GetStartRing(){return start_ring;};
	int GetRingsPerBed(){return rings_per_bed;};
	int GetNumCycles(){return num_cycles;};

	int GetTotalLORExposure(int axA, int axB){return ExposureLUT[axA][axB];};	/*!< Function that returns the total time (in seconds) that LOR contributes to the data set*/

private:
	void Initialize();												/*!< Function to initialize bed positions and numbers of cycles etc */
	void read_crys_eff();											/*!< Function to read in the crystall efficieny from file  */
	void FillExposureTable();
	std::string input_raw_fullpath;									/*!< Input file path for the Reconstruction_Paramters_X file (where X in 1:8) */
	std::string input_config;										/*!< Full path to the config file */
	static const unsigned int BUFFER_size = 679*840;				/*!< Size of the allocated memory into which crystal efficiencies will be read */
	static const unsigned int NUM_dummy_crystals = 7*840;			/*!< number of dummy crystals to skip. there are 7 rings of crystals */
	float crys_eff_672x840[BUFFER_size - NUM_dummy_crystals]; 		/*!< crystal efficiencies from file. Dimensions: axial * transaxial = 564480. Uses linear indexing */

	int num_ax_crys_mod=84;											/*!< Number of crystals per module (also per unit) in axial direction */
	int num_trans_crys_mod=70;										/*!< Number of crystals per module in TRANSaxial direction */
	int num_trans_crys_ring=840;									/*!< Number of crystals per transaxial ring (there is no axial ring, of course)*/
	int num_units=8;												/*!< Number of units (and modules) in axial direction*/

	bool createLogFile;
	std::string output_LOG;
	ofstream o_LOG;

	std::vector<int> bed_time_start;								/*!< start time for each bed position; included in current interval*/
	std::vector<int> bed_time_end;									/*!< end time; excluded from current interval */
	std::vector<int> bed_ring_start;								/*!< Start ring for each bed position */
	std::vector<int> bed_ring_end;									/*!< End ring for each bed position */

	int num_beds;													/*!< number of bed positions. Does not include multiple scans of same position*/
	int start_ring;													/*!< first ring of first bed position*/
	int rings_per_bed;												/*!< number of rings per bed position*/
	int bed_overlap;												/*!< overlap between bed positions as value between 0 and 1.*/
	int num_cycles;													/*!< number cycles per bed position. Usually 1.*/
	int time_per_bed;

	int firstTimeStamp;												/*!< first time stamp in the data set */
	int config_param[6];											/*!< config parameters like number of beds, overlap, start ring...*/

	int ExposureLUT[679][679];										/*!< contains the times that each pair of crystal rings contributes to the data set (in seconds) */
};



#endif /* SUBS_V1 */