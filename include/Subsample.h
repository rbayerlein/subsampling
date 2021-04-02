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

using namespace std;

class Subsample {
public:
	Subsample(std::string s);										/*!< Constructor. Invokes read_crys_eff() to read in the crystal efficiencies from file. */
	virtual ~Subsample();											/*!< Destructor */
	bool KeepEvent(int axA, int axB, int transA, int transB); 		/*!< Function that decides if that specific crystal pair specified by the given coordinates shall be kept.
																	Takes the axial and transaxial coordinates of the two coincident crystals as input.
																	*/

	std::string GetRawInputFullPath(){return input_raw_fullpath;};
	std::string GetCrysEffFullPath(){return input_crys_eff;};

private:
	void read_crys_eff();											/*!< Function to read in the crystall efficieny from file  */
	std::string input_raw_fullpath;									/*!< Input file path for the Reconstruction_Paramters_X file (where X in 1:8) */
	std::string input_crys_eff;										/*!< Full path to the crys_eff file */
	static const unsigned int BUFFER_size = 679*840;				/*!< Size of the allocated memory into which crystal efficiencies will be read */
	static const unsigned int NUM_dummy_crystals = 7*840;			/*!< number of dummy crystals to skip. there are 7 rings of crystals */
	float crys_eff_672x840[BUFFER_size - NUM_dummy_crystals]; 		/*!< crystal efficiencies from file. Dimensions: axial * transaxial = 564480. Uses linear indexing */

	int num_ax_crys_mod=84;											/*!< Number of crystals per module (also per unit) in axial direction */
	int num_trans_crys_mod=70;										/*!< Number of crystals per module in TRANSaxial direction */
	int num_trans_crys_ring=840;									/*!< Number of crystals per transaxial ring (there is no axial ring, of course)*/

};



#endif /* SUBS_V1 */