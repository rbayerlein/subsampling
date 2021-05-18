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

using namespace std;

class Subsample {
public:
	Subsample(std::string s, int t);										/*!< Constructor. Invokes read_crys_eff() to read in the crystal efficiencies from file. */
	virtual ~Subsample();											/*!< Destructor */
	bool KeepEvent(int axA, int axB, int transA, int transB, int timeStamp); 		/*!< Function that decides if that specific crystal pair specified by the given coordinates shall be kept.
																	Takes the axial and transaxial coordinates of the two coincident crystals as input.
																	*/

	std::string GetRawInputFullPath(){return input_raw_fullpath;};

private:
	void Initialize();												/*!< Function to initialize bed positions and numbers of cycles etc */
	void read_crys_eff();											/*!< Function to read in the crystall efficieny from file  */
	std::string input_raw_fullpath;									/*!< Input file path for the Reconstruction_Paramters_X file (where X in 1:8) */
	std::string input_config;										/*!< Full path to the config file */
	static const unsigned int BUFFER_size = 679*840;				/*!< Size of the allocated memory into which crystal efficiencies will be read */
	static const unsigned int NUM_dummy_crystals = 7*840;			/*!< number of dummy crystals to skip. there are 7 rings of crystals */
	float crys_eff_672x840[BUFFER_size - NUM_dummy_crystals]; 		/*!< crystal efficiencies from file. Dimensions: axial * transaxial = 564480. Uses linear indexing */

	int num_ax_crys_mod=84;											/*!< Number of crystals per module (also per unit) in axial direction */
	int num_trans_crys_mod=70;										/*!< Number of crystals per module in TRANSaxial direction */
	int num_trans_crys_ring=840;									/*!< Number of crystals per transaxial ring (there is no axial ring, of course)*/
	int num_units=8;												/*!< Number of units (and modules) in axial direction*/

	std::string output_DEBUG;
	ofstream o_DEBUG;

	std::vector<int> bed_time_start;								/*!< start time for each bed position; included in current interval*/
	std::vector<int> bed_time_end;									/*!< end time; excluded from current interval */
	std::vector<int> bed_ring_start;								/*!< Start ring for each bed position */
	std::vector<int> bed_ring_end;									/*!< End ring for each bed position */

	int num_beds;													/*!< number of bed positions. Does not include multiple scans of same position*/
	int start_ring;													/*!< first ring of first bed position*/
	int rings_per_bed;												/*!< number of rings per bed position*/
	float bed_overlap;												/*!< overlap between bed positions as value between 0 and 1.*/
	int num_cycles;													/*!< number cycles per bed position. Usually 1.*/
	int time_per_bed;

	int firstTimeStamp;												/*!< first time stamp in the data set */
	int debugCounter;

};



#endif /* SUBS_V1 */