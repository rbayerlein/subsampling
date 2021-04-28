#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h> // deprecated - try <cmath> later
#include <unistd.h> // for debugging only: to use function sleep()

#include "../include/coinc.h"
#include "../include/Subsample.h"

/*! \main function for reading in lm raw data */
/// \file main.cpp
/// \brief Implementation of raw data reading in list mode
/// this is the original client-side lm decoder that was modified to incorporate the subsample class

#define PI 3.141592653589793
#define BUFFER_SIZE 65536

using namespace std;

struct ids {
	short txID1, axID1, txID2, axID2, tof;
};

int main(int argc, char **argv) {

	if (argc != 2) {
		cout << "Usage: " << argv[0] << " [scan_details_fullpath]" << endl;
		return 1;
	}

	// ******************* File info ******************************************
	// variables that are set by user in GUI

	// file names, directories
	string infile_fullpath;
	string outfolder;
	string str_temp;

	// dynamic framing
	string static_dynamic;
	bool dyn = false;
	vector<int> frames;
	vector<float> frame_t;
	bool ft1 = true;
	int vec_len = 0;

	bool write_lmfile = false;
	bool write_sino = false;
	bool write_sino_block = true;
	bool write_histo_img = true;

	bool rand_sub = false;

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

	// path to output folder
	getline(detes, str_temp);
	outfolder = str_temp;
	str_temp = "";

	// path to .lm data
	getline(detes, str_temp);
	infile_fullpath = str_temp;

	str_temp = "";

	// get raw data number (1 to 8)
	string str_find = ".raw";
	size_t ext_ind = infile_fullpath.find(str_find);
	char raw_num_char = infile_fullpath[ext_ind - 1];
	string raw_num(1, raw_num_char);
	cout << "File number = " << raw_num << endl;

	// Static or dynamic (1,2)
	getline(detes, str_temp);
	stringstream convert2(str_temp);
	int subint2;
	convert2 >> subint2;
	bool run_recon = false;
	if (subint2 == 1) {
		run_recon = true;
	}

	convert2.str("");
	convert2.clear();
	str_temp = "";

	// dynamic framing (1, 100, 2, 200, etc)
	getline(detes, str_temp);
	stringstream convert3(str_temp);
	int subint3;
	convert3 >> subint3;
	static_dynamic = str_temp;

	if (subint3 == 0) {
		dyn = false;
	} else {
		dyn = true;
	}
	stringstream sd(static_dynamic);
	while (sd.good()) {
		string substr;

		getline(sd, substr, ',');
		stringstream convert3(substr);

		if (ft1) {
			convert3 >> subint;
			convert3.str("");
			convert3.clear();
			frames.push_back((int) subint);
			vec_len++;
		} else {
			convert3 >> subtime;
			convert3.str("");
			convert3.clear();
			frame_t.push_back((double) subtime);
		}
		ft1 = !ft1;
	}
	str_temp = "";

	// write processed listmode data (0 or 1)
	getline(detes, str_temp);
	stringstream convert4(str_temp);
	convert4 >> subint;
	convert4.str("");
	convert4.clear();
	if (subint == 1) {
		write_lmfile = true;
	} else {
		write_lmfile = false;
	}
	subint = 0;
	str_temp = "";

	// write 3D sinograms (0 or 1)
	getline(detes, str_temp);
	stringstream convert5(str_temp);
	convert5 >> subint;
	convert5.str("");
	convert5.clear();
	if (subint == 1) {
		write_sino = true;
		cout << "write sino TRUE" << endl;
	} else {
		write_sino = false;
		cout << "write sino FALSE" << endl;
	}
	subint = 0;
	str_temp = "";

	// write block sinograms (0 or 1)
	getline(detes, str_temp);
	stringstream convert6(str_temp);
	convert6 >> subint;
	convert6.str("");
	convert6.clear();
	if (subint == 1) {
		write_sino_block = true;
		cout << "write block sino TRUE" << endl;
	} else {
		write_sino_block = false;
		cout << "write block sino FALSE" << endl;
	}
	subint = 0;
	str_temp = "";

	// write TOF histo image
	getline(detes, str_temp);
	stringstream convert7(str_temp);
	convert7 >> subint;
	convert7.str("");
	convert7.clear();
	if (subint == 1) {
		write_histo_img = true;
		cout << "write TOF HISTO IMG = TRUE" << endl;
	} else {
		write_histo_img = false;
		cout << "write TOF HISTO IMG = FALSE" << endl;
	}
	subint = 0;
	str_temp = "";

	detes.close();

	// dynamic framing 
	int tot_frames = 0;
	for (int i = 0; i < vec_len; i++) {
		tot_frames = tot_frames + frames[i];
	}
	if (dyn) {
		cout << "Dynamic imaging" << endl;
		cout << "total frames = " << tot_frames << endl;
	} else {
		cout << "Static imaging" << endl;
	}
	vector<int> frame_length(tot_frames);
	vector<double> frame_length_s(tot_frames);
	int frame_count = 0;
	int cur_pos = 0;
	for (int i = 0; i < tot_frames; i++) {
		if (i == (frames[cur_pos] + frame_count)) {
			frame_count = frame_count + frames[cur_pos];
			cur_pos++;

		}
		frame_length[i] = (int) (1000 * frame_t[cur_pos]);
		frame_length_s[i] = (frame_t[cur_pos]);

		if (dyn) {
			cout << "Frame " << i << " = " << frame_length_s[i] << " seconds"
					<< endl;

		}
	}

	///////////////////////////////////////////////////////////////////////////
	//  ********************* Scanner parameters and global variables *********

	// scanner parameters
	int num_bins_sino = 549 * 420;
	int num_bins_sino2 = num_bins_sino / 6;
	int num_bins_sino_block = 91 * 60;
	int num_bins_sino_module = 21 * 12;
	int num_slice = 1343;
	int num_slice_block = 223;
	int num_crystals = 840;
	int num_crystals_ax_wgap = 679;
	int num_crystals_all = 564480;
	int num_blocks = 120 * 8 * 14;
	int num_mod = 24 * 8;

	double tof_res = 450.0;

	// for reading lm file	

	uint64_t *pRawBuffer = new uint64_t[BUFFER_SIZE];

	int crys1, crys2 = 0;
	int transA, transB, axA, axB = 0;
	int bankk = 0;
	int modA, modB = 0;
	int uid = 0;
	int unitA, unitB, unit_diff = 0;

	signed short dtemp = 0;
	short transcA, transcB, crysaxA, crysaxB = 0;

	// time tag variables
	int year1, month1, day1, hour1, minute1, second1, milli1 = 0;
	int year00, month00, day00, hour00, minute00, second00, milli00 = 0;
	int ss0, ssdiff = 0;

	// block count rate variables
	vector<double> block_rate_new(num_blocks);
	vector<double> block_rate(num_blocks);
	vector<double> block_rate_counts(num_blocks);

	vector<double> module_rate_new(num_mod);
	vector<double> module_rate(num_mod);
	vector<double> module_rate_counts(num_mod);

	int blkX, blkXa, blkXb, blkY, blkYa, blkYb, blk_abs, blk_absA, blk_absB = 0;
	int br_temp = 0;
	double singles_c1, singles_c2 = 0.0;
	double rstemp = 0.0;
	for (int br = 0; br < num_blocks; br++) {
		block_rate[br] = 0.0;
		block_rate_new[br] = 0.0;
		block_rate_counts[br] = 0.0;
	}

	// coincidence counters
	vector<double> prompt_rate_new(num_mod);
	vector<double> prompt_rate(num_mod);
	vector<double> random_rate_new(num_mod);
	vector<double> random_rate(num_mod);

	for (int mr = 0; mr < num_mod; mr++) {
		module_rate[mr] = 0.0;
		module_rate_new[mr] = 0.0;
		module_rate_counts[mr] = 0.0;

		prompt_rate[mr] = 0.0;
		prompt_rate_new[mr] = 0.0;

		random_rate[mr] = 0.0;
		random_rate_new[mr] = 0.0;

	}

	// sinogram variables
	long ind, ind2, ind_block, ind_block2, ind_module1, ind_module2,
			ind_module_trans = 0;
	int nv, nu, nvtemp, nutemp, ntemp = 0;
	int sino_ax_span = 10000;
	int block_ax_span = 2;
	int num_lor_modpair = 4323270;
	int michel_ind = 0;

	vector<int> sino_block(num_bins_sino_block * num_slice_block);
	vector<int> sino_block_r(num_bins_sino_block * num_slice_block);

	vector<double> sino_module_r_new(num_bins_sino_module * 8 * 8);
	vector<double> sino_module_r_avg(num_bins_sino_module * 8 * 8);

	vector<double> sino_module_new(num_bins_sino_module * 8 * 8);
	vector<double> sino_module_avg(num_bins_sino_module * 8 * 8);

	for (int modi = 0; modi < (num_bins_sino_module * 8 * 8); modi++) {
		sino_module_r_avg[modi] = 0;
		sino_module_r_new[modi] = 0;
		sino_module_avg[modi] = 0;
		sino_module_new[modi] = 0;
	}
	bool write_singles = true;
	bool write_module_sinos = true;
	bool write_coincidences = true;

	// HistoTOF image variables
	int histo_img_size = 128 * 128 * 324;
	vector<float> histo_image(histo_img_size);
	double c_xx, c_yy, c_zz, v_xx, v_yy, v_zz, l_v = 0.0;
	int p_xx, p_yy, p_zz, ind_histo = 0;
	double img_counts = 0.0;

	double tof_x, tof_y, tof_z = 0.0;

	// randoms
	float sector_counts[24][24] = { 0.0 };
	vector<float> sector_flood(570360 * 24);
	float rtemp = 0.0;
	int sec1, sec2 = 0;
	int ct1, ct2, rt1, rt2 = 0;
	float lorsum1, lorsum2, lorsumall = 0.0;
	float t_window = 1.0;
	int r_frame = 2000;
	int r_frame_end = 2000; // getting initial count rates for the first 2000 ms
	bool r_singles = false;

	float mtemp = 1.0;

	// dead time
	double tau_s = 0.37E-6;
	double xx, ff, mm, xxnew, floodtemp = 0.0;
	double DTtemp, DTtemp1, DTtemp2 = 1.0;
	double singles_bkg = 36000.0;
	double DT_fac[192][192] = { 1.0 };

	// dynamic 
	int frame_start = 0.0;
	int frame_end = 0.0;
	int frame_num = 0;
	int time_s, time_s0 = 0;
	double time_sd = 0.0;

	frame_end = frame_start + frame_length[0];

	// counters
	double cou = 100000000.0;
	double num_prompts, num_randoms, num_coinc = 0;

	// to run 
	bool write_lm = false;
	string runrecon_str = "";
	string removeraws_str = "";

	///////////////////////////////////////////////////////////////////////////

	// ********************  Read Listmode  ***********************************

	string infile_fullpath1 = infile_fullpath;
	FILE *pInputFile = fopen(infile_fullpath1.c_str(), "rb");
	if (pInputFile == NULL) { // check return value of file pointer
		cerr << infile_fullpath1 << " cannot be opened." << endl;
		exit(1);
	}

	// check input file size
	ifstream infile1;
	infile1.open(infile_fullpath1.c_str(), ios::in | ios::binary); // open list mode file
	infile1.seekg(0, infile1.end);
	long long file_size1 = infile1.tellg(); //get size (events) of list mode file
	if (file_size1 % 8 != 0) {
		cerr << infile_fullpath1 << " file size is not divisible by 8!" << endl;
		exit(1);
	}
	long long num_events = file_size1 / 8;
	cout << num_events << " total events" << endl;
	infile1.close();

	stringstream ss;
	ss << "lm_reorder_f" << frame_num;
	string fname_out = ss.str();

	string outfile_fullpath_p = outfolder + fname_out + "_prompts." + raw_num
			+ ".lm";
	string outfile_fullpath_r = outfolder + fname_out + "_randoms." + raw_num
			+ ".lm";
	string outfile_fullpath_s = outfolder + fname_out + "_prompts." + raw_num
			+ ".add_fac";
	string outfile_fullpath_m = outfolder + fname_out + "_prompts." + raw_num
			+ ".mul_fac";

	ss << "";
	ss.clear();
	str_temp = "";
	fname_out = "";

	FILE *pPromptFile = fopen(outfile_fullpath_p.c_str(), "wb"); // initial output pointer (pointer will be reused for subsequent prompt files)
	ids *pids_p = new ids[BUFFER_SIZE]; // prompt output buffer
	unsigned long long p_index = 0; // prompt buffer index

	//ofstream outfile_r;	//randoms lm output
	//outfile_r.open(outfile_fullpath_r.c_str(), ios::out | ios::binary); //create binary file containing new crystal + time data

	ofstream outfile_s;
	outfile_s.open(outfile_fullpath_s.c_str(), ios::out | ios::binary);

	ofstream outfile_m;
	outfile_m.open(outfile_fullpath_m.c_str(), ios::out | ios::binary);

	outfile_fullpath_p = "";
	outfile_fullpath_r = "";
	outfile_fullpath_s = "";
	outfile_fullpath_m = "";

	// count rates
	// singles
	string outfile_fullpath_singles = outfolder;
	outfile_fullpath_singles.append("singles.");
	outfile_fullpath_singles.append(raw_num);
	outfile_fullpath_singles.append(".raw");

	ofstream outfile_singles;
	outfile_singles.open(outfile_fullpath_singles.c_str(),
			ios::out | ios::binary);

	string outfile_fullpath_singles_mod = outfolder;
	outfile_fullpath_singles_mod.append("singles_module.");
	outfile_fullpath_singles_mod.append(raw_num);
	outfile_fullpath_singles_mod.append(".raw");

	ofstream outfile_singles_module;
	outfile_singles_module.open(outfile_fullpath_singles_mod.c_str(),
			ios::out | ios::binary);

	// coincidences

	string outfile_fullpath_prompts = outfolder;
	string outfile_fullpath_randoms = outfolder;
	outfile_fullpath_prompts.append("prompts.");
	outfile_fullpath_randoms.append("randoms.");
	outfile_fullpath_prompts.append(raw_num);
	outfile_fullpath_randoms.append(raw_num);
	outfile_fullpath_prompts.append(".raw");
	outfile_fullpath_randoms.append(".raw");

	ofstream outfile_prompts_rate;

	ofstream outfile_randoms_rate;

	outfile_prompts_rate.open(outfile_fullpath_prompts.c_str(),
			ios::out | ios::binary);

	outfile_randoms_rate.open(outfile_fullpath_randoms.c_str(),
			ios::out | ios::binary);

	// prompt, random module pair sinograms
	string outfile_fullpath_pmod_sino = outfolder;
	string outfile_fullpath_rmod_sino = outfolder;
	outfile_fullpath_pmod_sino.append("prompts_sino.");
	outfile_fullpath_pmod_sino.append(raw_num);
	outfile_fullpath_rmod_sino.append("randoms_sino.");
	outfile_fullpath_rmod_sino.append(raw_num);
	outfile_fullpath_pmod_sino.append(".raw");
	outfile_fullpath_rmod_sino.append(".raw");

	ofstream outfile_pmod_sino;
	outfile_pmod_sino.open(outfile_fullpath_pmod_sino.c_str(),
			ios::out | ios::binary);

	ofstream outfile_rmod_sino;
	outfile_rmod_sino.open(outfile_fullpath_rmod_sino.c_str(),
			ios::out | ios::binary);

	// ssrb sino names
	string fname_sino;
	string sino_fullpath_p;
	string sino_fullpath_r;

	// block sino names
	string fname_sino_block;
	string sino_fullpath_block_p;
	string sino_fullpath_block_r;

	string fname_histo_img;
	string histo_img_fullpath;

	string frameinfo_fullpath;

	// ************		Load LUTs  ****************//
//	string fdir_code = "/home/eberg/code/explorer-master/read_lm/lut/";
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

	// crystal index - sinogram bin LUTs
	vector<int> index_crystalpairs_transaxial_int16_1(num_bins_sino);
	vector<int> index_crystalpairs_transaxial_int16_2(num_bins_sino);

	string fpath_crysidx = fdir_code;
	fpath_crysidx.append("index_crystalpairs_transaxial_2x549x420_int16");
	ifstream crysidx_file;
	crysidx_file.open(fpath_crysidx.c_str(), ios::in | ios::binary); //open list mode file

	if (!crysidx_file) {
		crysidx_file.close();
		cout << "Cannot open crys_idx file\nQUIT\n\n";

		return 1;
	}

	for (int ns = 0; ns < num_bins_sino; ns++) {

		crysidx_file.read(
				reinterpret_cast<char*>(&index_crystalpairs_transaxial_int16_1[ns]),
				sizeof(signed short));
		crysidx_file.read(
				reinterpret_cast<char*>(&index_crystalpairs_transaxial_int16_2[ns]),
				sizeof(signed short));
	}

	int noindex_crystalpairs_transaxial_int16[840][840];

	for (int jj = 0; jj < num_crystals; jj++) {
		for (int kk = 0; kk < num_crystals; kk++) {
			noindex_crystalpairs_transaxial_int16[jj][kk] = -1;
		}
	}

	for (int nss = 0; nss < num_bins_sino; nss++) {
		nv = index_crystalpairs_transaxial_int16_1[nss];
		nu = index_crystalpairs_transaxial_int16_2[nss];
		noindex_crystalpairs_transaxial_int16[nv][nu] = nss;
		noindex_crystalpairs_transaxial_int16[nu][nv] = nss;
	}

	// block index - block sinogram bin LUTs
	vector<int> index_blockpairs_transaxial_int16_1(num_bins_sino_block);
	vector<int> index_blockpairs_transaxial_int16_2(num_bins_sino_block);

	string fpath_blockidx = fdir_code;
	fpath_blockidx.append("index_blockpairs_transaxial_2x91x60_int16");
	ifstream blockidx_file;
	blockidx_file.open(fpath_blockidx.c_str(), ios::in | ios::binary); //open list mode file

	if (!blockidx_file) {
		blockidx_file.close();
		cout << "Cannot open block_idx file\nQUIT\n\n";

		return 1;
	}

	for (int nbs = 0; nbs < num_bins_sino_block; nbs++) {

		blockidx_file.read(
				reinterpret_cast<char*>(&index_blockpairs_transaxial_int16_1[nbs]),
				sizeof(signed short));
		blockidx_file.read(
				reinterpret_cast<char*>(&index_blockpairs_transaxial_int16_2[nbs]),
				sizeof(signed short));
	}

	int noindex_blockpairs_transaxial_int16[120][120];

	for (int jbj = 0; jbj < 120; jbj++) {
		for (int kbk = 0; kbk < 120; kbk++) {
			noindex_blockpairs_transaxial_int16[jbj][kbk] = -1;
		}
	}

	for (int nbss = 0; nbss < num_bins_sino_block; nbss++) {
		nv = index_blockpairs_transaxial_int16_1[nbss];
		nu = index_blockpairs_transaxial_int16_2[nbss];
		noindex_blockpairs_transaxial_int16[nv][nu] = nbss;
		noindex_blockpairs_transaxial_int16[nu][nv] = nbss;
	}

	// module index - module sinogram bin LUTs
	vector<int> index_modulepairs_transaxial_int16_1(num_bins_sino_module);
	vector<int> index_modulepairs_transaxial_int16_2(num_bins_sino_module);

	string fpath_modidx = fdir_code;
	fpath_modidx.append("index_modulepairs_transaxial_2x21x12_int16");
	ifstream modidx_file;
	modidx_file.open(fpath_modidx.c_str(), ios::in | ios::binary); //open list mode file

	if (!modidx_file) {
		modidx_file.close();
		cout << "Cannot open module_idx file\nQUIT\n\n";

		return 1;
	}

	for (int nms = 0; nms < num_bins_sino_module; nms++) {

		modidx_file.read(
				reinterpret_cast<char*>(&index_modulepairs_transaxial_int16_1[nms]),
				sizeof(signed short));
		modidx_file.read(
				reinterpret_cast<char*>(&index_modulepairs_transaxial_int16_2[nms]),
				sizeof(signed short));
	}

	int noindex_modulepairs_transaxial_int16[24][24];

	for (int jmj = 0; jmj < 24; jmj++) {
		for (int kmk = 0; kmk < 24; kmk++) {
			noindex_modulepairs_transaxial_int16[jmj][kmk] = -1;
		}
	}

	for (int nmss = 0; nmss < num_bins_sino_module; nmss++) {
		nv = index_modulepairs_transaxial_int16_1[nmss];
		nu = index_modulepairs_transaxial_int16_2[nmss];
		noindex_modulepairs_transaxial_int16[nv][nu] = nmss;
		noindex_modulepairs_transaxial_int16[nu][nv] = nmss;
	}

	// crystal position x,y,z
	string rx_lut = fdir_code;
	string ry_lut = fdir_code;
	string zz_lut = fdir_code;
	rx_lut.append("rx");
	ry_lut.append("ry");
	zz_lut.append("zz");

	ifstream rx_read;
	ifstream ry_read;
	ifstream zz_read;

	rx_read.open(rx_lut.c_str(), ios::in | ios::binary);
	ry_read.open(ry_lut.c_str(), ios::in | ios::binary);
	zz_read.open(zz_lut.c_str(), ios::in | ios::binary);

	vector<double> rx(num_crystals);
	vector<double> ry(num_crystals);
	vector<double> zz(num_crystals_ax_wgap);

	for (int rr = 0; rr < num_crystals; rr++) {
		rx_read.read(reinterpret_cast<char*>(&rx[rr]), sizeof(double));
		ry_read.read(reinterpret_cast<char*>(&ry[rr]), sizeof(double));
	}

	for (int zzz = 0; zzz < num_crystals_ax_wgap; zzz++) {
		zz_read.read(reinterpret_cast<char*>(&zz[zzz]), sizeof(double));
	}
	rx_read.close();
	ry_read.close();
	zz_read.close();

	// michelogram LUT for ring pairs
	string michel_lut_fullpath = fdir_code;
	michel_lut_fullpath.append("michel_lut");

	ifstream michel_lut_read;
	michel_lut_read.open(michel_lut_fullpath.c_str(), ios::in | ios::binary);

	vector<int> michel_lut(672 * 672);

	for (int ml = 0; ml < (672 * 672); ml++) {
		michel_lut_read.read(reinterpret_cast<char*>(&michel_lut[ml]),
				sizeof(float));
	}

	michel_lut_read.close();

	// normalization file
	string ncfile_fullpath = infile_fullpath;
	ncfile_fullpath.erase(infile_fullpath.length() - 3, 3);
	ncfile_fullpath.append("nc");
	cout << ncfile_fullpath << "\n";

	bool use_nc_default = false;

	vector<float> nc_crys(num_crystals_all);
	vector<float> plaeff(672 * 672);

	ifstream nc_read;
	nc_read.open(ncfile_fullpath.c_str(), ios::in | ios::binary);

	if (!nc_read) {
		cout << "Could not open nc file, use default value (1) \n";
		use_nc_default = true;
		for (int nci = 0; nci < num_crystals_all; nci++) {
			nc_crys[nci] = 1.0;
		}
	}

	if (!use_nc_default) {
		nc_read.seekg(3095517 * 4, nc_read.beg);

		for (int nc_c = 0; nc_c < num_crystals_all; nc_c++) {
			nc_read.read(reinterpret_cast<char*>(&nc_crys[nc_c]),
					sizeof(float));
		}

		nc_read.seekg(2079453 * 4, nc_read.beg);
		for (int nc_p = 0; nc_p < (672 * 672); nc_p++) {
			nc_read.read(reinterpret_cast<char*>(&plaeff[nc_p]), sizeof(float));
		}
	}

	// timing LUTs
	string tLUT_temp = infile_fullpath1;
	int str_len = tLUT_temp.length();
	tLUT_temp.erase(str_len - 3, 3);

	string tLUT_hw = tLUT_temp;
	string tLUT_sw = tLUT_temp;
	//string tLUT_tdc = tLUT_temp;

	tLUT_hw.append("hw");
	tLUT_sw.append("sw");
	//tLUT_tdc.append("tdc");  

	vector<double> t_sw(num_crystals_all);
	vector<int> t_hw(num_crystals_all);
	//vector<double> t_tdc(168*560*128); 

	ifstream t_sw_read;
	ifstream t_hw_read;
	t_sw_read.open(tLUT_sw.c_str(), ios::in | ios::binary);
	t_hw_read.open(tLUT_hw.c_str(), ios::in | ios::binary);

	bool use_toff_default = false;

	if (!t_sw_read || !t_hw_read) {
		cout << "could not open timing offset files \n";
		use_toff_default = true;
	}

	for (int tt1 = 0; tt1 < (num_crystals_all); tt1++) {
		if (use_toff_default) {
			t_sw[tt1] = 0.0;
			t_hw[tt1] = 0;
		} else {
			t_sw_read.read(reinterpret_cast<char*>(&t_sw[tt1]), sizeof(double));
			t_hw_read.read(reinterpret_cast<char*>(&t_hw[tt1]), sizeof(int));
		}
	}

	//ifstream t_tdc_read; 
	//t_tdc_read.open(tLUT_tdc.c_str(), ios::in | ios::binary); 

	//if (!t_tdc_read) {
	//cout << "could not open TDC file\n"; 
	//}

	//for (int tt2 = 0; tt2<(168*560*128); tt2++) { 
	//	t_tdc_read.read(reinterpret_cast<char *>(&t_tdc[tt2]), sizeof(double)); 
	//}

	// timing offset file
	//string TO_SW_path = fdir_code; 
	//TO_SW_path.append("TO_SW");
	// //cout << TO_SW_path << "\n";
	//ifstream TOSW_read; 
	//TOSW_read.open(TO_SW_path.c_str(), ios::in | ios::binary);
	//vector<double> toff_sw(num_crystals_all); 
	//if (!TOSW_read) {
	//	cout << "could not open timing offset file \n"; 
	//}
	//for (int tt=0; tt<num_crystals_all; tt++) {
	//	TOSW_read.read(reinterpret_cast<char *>(&toff_sw[tt]), sizeof(double));
	//}

	vector<float> coinc_window(8); // ns
	coinc_window[0] = 4500.0;
	coinc_window[1] = 4800.0;
	coinc_window[2] = 5400.0;
	coinc_window[3] = 6100.0;
	coinc_window[4] = 6900.0;
	coinc_window[5] = 6900.0;
	coinc_window[6] = 6900.0;
	coinc_window[7] = 6900.0;
	coinc_window[8] = 6900.0;

	// ========================= Main Run Program =========================
	// ====================================================================

	cout << "\nFinding start time...\n";

	unsigned long long byte_location = 0; // for event identification

	bool found_first_timestamp = false;
	while (!found_first_timestamp) { // stop when first timestamp found
		unsigned long long read_count = fread(pRawBuffer, sizeof(uint64_t),
		BUFFER_SIZE, pInputFile); // returns BUFFER_SIZE events read unless EOF
		for (unsigned long long i = 0; i < read_count; i++) { // loop through each event
			// start check event type
			if (COINC::IsTimestamp(pRawBuffer[i])) {
				year00 = COINC::GetYear(pRawBuffer[i]);
				month00 = COINC::GetMonth(pRawBuffer[i]);
				day00 = COINC::GetDay(pRawBuffer[i]);
				hour00 = COINC::GetHour(pRawBuffer[i]);
				minute00 = COINC::GetMinute(pRawBuffer[i]);
				second00 = COINC::GetSecond(pRawBuffer[i]);
				milli00 = COINC::GetMillisecond(pRawBuffer[i]);
				found_first_timestamp = true;
				byte_location += 8; // move to event after first timestamp (8 bytes)
				break; // break out of for loop
			} else if (COINC::IsBedPos(pRawBuffer[i])) {
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
			// end check event type
		} // end for loop
	} // end while loop
	cout << "Scan start: " << year00 << "/" << month00 << "/" << day00 << " "
			<< hour00 << ":" << minute00 << ":" << second00 << "." << milli00
			<< endl;

	// ========================= get initial count rates =========================

	ss0 = milli00; // temporary assignment of start time

	int scan_start_ms = ss0;

	long int start_offset = byte_location; // position after first timestamp
	cout << "Event after first timestamp is located at: " << byte_location
			<< endl;
	fseek(pInputFile, start_offset, SEEK_SET); // rewind to after first timestamp

	cout << "*** Getting initial count rates ***" << endl;

	bool found_initial_count_rate = false;
	bool keep_event = true;
	while (!found_initial_count_rate) { // stop when initial count rate found
		unsigned long long read_count = fread(pRawBuffer, sizeof(uint64_t),
		BUFFER_SIZE, pInputFile); // returns BUFFER_SIZE events read unless EOF
		for (unsigned long long i = 0; i < read_count; i++) { // loop through each event
			// start check event type
			if (COINC::IsBlockRate(pRawBuffer[i])) {
				uid = COINC::GetUnitID(pRawBuffer[i]);
				blkX = COINC::GetBlockIDX(pRawBuffer[i]); // absolute block IDX
				br_temp = COINC::GetBlockRate(pRawBuffer[i]);
				blkY = COINC::GetBlockIDY(pRawBuffer[i]) + (uid * 14); // absolute block IDY
				modA = floor(blkX / 5) + (24 * floor(blkY / 14));
				ind = blkX + (120 * blkY); // global block ID
				block_rate_new[ind] = block_rate_new[ind]
						+ ((double) br_temp * 10.0);
				block_rate_counts[ind] += 1.0;
				module_rate_new[modA] = module_rate_new[modA]
						+ ((double) br_temp * 10.0);
				module_rate_counts[modA] = module_rate_counts[modA]
						+ (1.0 / 70.0);
				byte_location += 8; // move to next event (8 bytes)
			} else if (COINC::IsTimestamp(pRawBuffer[i])) {
				day1 = COINC::GetDay(pRawBuffer[i]);
				year1 = COINC::GetYear(pRawBuffer[i]);
				month1 = COINC::GetMonth(pRawBuffer[i]);
				milli1 = COINC::GetMillisecond(pRawBuffer[i]);
				second1 = COINC::GetSecond(pRawBuffer[i]);
				minute1 = COINC::GetMinute(pRawBuffer[i]);
				hour1 = COINC::GetHour(pRawBuffer[i]);

				ssdiff = milli1 - ss0; // ss0 here is milli00 i.e. frame start time (ms only)
				if (ssdiff < 0) {
					ssdiff = ssdiff + 1000; // milli1 is current time (ms only) so add 1000 ms when negative
				}
				time_s = time_s + ssdiff; // increments by 1 ms normally (since ssdiff is always relative to last timestamp)
				ss0 = milli1; // ss0 is now current ms (so ss0 in a way is temporary in nature)

				if (time_s >= r_frame_end) { // stop the while loop when this condition is satisfied
					// add assorted data into various vectors
					for (int rk = 0; rk < num_bins_sino_module * 8 * 8; rk++) {
						sino_module_r_avg[rk] = sino_module_r_new[rk];
						sino_module_r_new[rk] = 0;
						sino_module_avg[rk] = sino_module_new[rk];
						sino_module_new[rk] = 0;
					}
					for (int bk = 0; bk < num_blocks; bk++) {
						if (block_rate_counts[bk] > 1.0) {
							block_rate[bk] = block_rate_new[bk]
									/ block_rate_counts[bk];
						} else {
							block_rate[bk] = 00.0;
						}
						block_rate_new[bk] = 0.0;
						block_rate_counts[bk] = 0.0;
					}
					for (int mk = 0; mk < num_mod; mk++) {
						prompt_rate[mk] = prompt_rate_new[mk];
						prompt_rate_new[mk] = 0.0;

						random_rate[mk] = random_rate_new[mk];
						random_rate_new[mk] = 0.0;
						if (module_rate_counts[mk] > 1.0) {
							module_rate[mk] = module_rate_new[mk]
									/ module_rate_counts[mk];
						} else {
							module_rate[mk] = 0.0;
						}
						module_rate_new[mk] = 0.0;
						module_rate_counts[mk] = 0.0;
					}
					for (int bbi = 0; bbi < (24 * 8); bbi++) { // make dead-time block pair array
						for (int bbj = 0; bbj < (24 * 8); bbj++) {
							DTtemp1 = 1.0;
							DTtemp2 = 1.0;
							// 1
							floodtemp = module_rate[bbi] - singles_bkg;
							if (floodtemp < 10.0) {
								floodtemp = 10.0;
							}
							xx = floodtemp;
							ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
							mm = (tau_s * xx - 1.0) * exp(-1.0 * tau_s * xx);
							xxnew = xx - (ff / mm);
							// Newton root finder
							for (int iii = 0; iii < 5; iii++) {
								xx = xxnew;
								xxnew = 0.0;
								ff = 0.0;
								ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
								mm = 0.0;
								mm = (tau_s * xx - 1.0)
										* exp(-1.0 * tau_s * xx);
								xxnew = xx - (ff / mm);
							}
							xx = 0.0;
							mm = 0.0;
							ff = 0.0;

							DTtemp1 = xxnew / floodtemp;
							xxnew = 0.0;

							floodtemp = 0.0;
							// 2
							floodtemp = module_rate[bbj] - singles_bkg;
							if (floodtemp < 10.0) {
								floodtemp = 10.0;
							}
							xx = floodtemp;
							ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
							mm = (tau_s * xx - 1.0) * exp(-1.0 * tau_s * xx);
							xxnew = xx - (ff / mm);
							// Newton root finder
							for (int iii = 0; iii < 5; iii++) {
								xx = xxnew;
								xxnew = 0.0;
								ff = 0.0;
								ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
								mm = 0.0;
								mm = (tau_s * xx - 1.0)
										* exp(-1.0 * tau_s * xx);
								xxnew = xx - (ff / mm);
							}
							xx = 0.0;
							mm = 0.0;
							ff = 0.0;

							DTtemp2 = xxnew / floodtemp;
							xxnew = 0.0;

							//DTtemp = (DTtemp1 + DTtemp2) / 2.0;
							DTtemp = sqrt(DTtemp1) * sqrt(DTtemp2);

							//if (DTtemp < 0.99) {
							//DTtemp = 1.0;
							//}
							//if (isnan(DTtemp) || isinf(DTtemp)) {
							//DTtemp = 1.0;
							//}
							//if (DTtemp > 3.0) {
							//DTtemp = 3.0;
							//}

							//DT_fac[i][j] = DT_fac[i][j] + floodtemp * DTtemp;
							//DT_fac[bbi][bbj] = 1.0 / DTtemp;
							DT_fac[bbi][bbj] = DTtemp;
						}
					} //end making all dead time block pair array
					found_initial_count_rate = true;
					byte_location += 8; // move to next event (8 bytes)
					break; // break out of for loop
				} // end if frame complete (i.e. else continue)
			} else if (COINC::IsCoinc(pRawBuffer[i])) {
				if (num_coinc > cou) { // counter for monitoring
					cout << "num events = " << num_coinc << "\n";
					cou = cou + 100000000.0;
				}
				num_coinc += 1.0;

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
/*
				keep_event = SUBS.KeepEvent(axA, axB, transA, transB);
				if (!keep_event){
				//	cout << "coincidences A,B (trans/ax):\t(" << transA << "/" << axA << "),\t(" << transB << "/" << axB << ")" << endl;
				//	cout << "keep event " << num_coinc << " (1=yes):\t" << keep_event << endl;
					num_coinc -= 1.0;
					continue;
				}
*/
				blkXa = floor(transA / 7);
				blkXb = floor(transB / 7);
				blkYa = floor(axA / 6);
				blkYb = floor(axB / 6);
				blk_absA = blkXa + (120 * blkYa);
				blk_absB = blkXb + (120 * blkYb);

				modA = floor(transA / 35);
				modB = floor(transB / 35);

				ind_block = noindex_blockpairs_transaxial_int16[blkXa][blkXb];

				ind_module_trans =
						noindex_modulepairs_transaxial_int16[modA][modB];

				ind_module1 = ind_module_trans + (num_bins_sino_module * unitA)
						+ (num_bins_sino_module * 8 * unitB);
				ind_module2 = ind_module_trans + (num_bins_sino_module * unitB)
						+ (num_bins_sino_module * 8 * unitA);

				if (ind_module_trans >= 0) {
					if (COINC::IsDelayFlag(pRawBuffer[i])) { // bin to delay related stuff
						random_rate_new[modA + 24 * unitA] += 1.0;
						random_rate_new[modB + 24 * unitB] += 1.0;
						sino_module_r_new[ind_module1] += 1.0;
						sino_module_r_new[ind_module2] += +1.0;
					} else { // bin to prompt related stuff
						prompt_rate_new[modA + 24 * unitA] += 1.0;
						prompt_rate_new[modB + 24 * unitB] += 1.0;
						sino_module_new[ind_module1] += 1.0;
						sino_module_new[ind_module2] += 1.0;
					}
				}
				byte_location += 8; // move to next event (8 bytes)
			} else if (COINC::IsBedPos(pRawBuffer[i])) {
				byte_location += 8; // move to next event (8 bytes)
				continue;
			} else if (COINC::IsPdRate(pRawBuffer[i])) {
				byte_location += 8; // move to next event (8 bytes)
				continue;
			} else {
				cout << "Unknown event detected at " << byte_location << endl;
				byte_location += 8; // move to next event (8 bytes) - continue for loop
			}
			// end check event type
		} // end for loop
	} // end while loop

	// ========================= go through the entire dataset =========================

	fseek(pInputFile, start_offset, SEEK_SET); // rewind to after first timestamp
	byte_location = start_offset; // rewind

	ss0 = scan_start_ms; // reset ss0 since it is temporary in nature

	double block_sing_sum = 0.0;
	for (int bk = 0; bk < num_blocks; bk++) {
		block_sing_sum = block_sing_sum + block_rate[bk];
	}
	double mod_sing_sum = 0.0;
	for (int mk = 0; mk < num_mod; mk++) {
		mod_sing_sum = mod_sing_sum + module_rate[mk];
	}

	cout << block_sing_sum << ", " << mod_sing_sum << "\n";

	cout << "\n\n*** Processing Frame 0... *** \n\n";

	bool next_frame = false;
	time_s = 0; // reset overall time

	int TEST_COUNTER = 0;
	bool run = true;
	while (run) {
//		TEST_COUNTER++;
//		if (TEST_COUNTER>1) {
//			cout << "TEST_COUNTER larger 1"<< endl;
//			break;
//		}
		unsigned long long read_count = fread(pRawBuffer, sizeof(uint64_t),
		BUFFER_SIZE, pInputFile); // returns BUFFER_SIZE events read unless EOF
		for (unsigned long long i = 0; i < read_count; i++) { // loop through each event
			if (byte_location + 8 > (file_size1 - 8)) { // hard-coded implementation to not use events at the end of the input file (legacy)
				next_frame = true; // stop reading file if done
			}
			// start check event type
			if (COINC::IsBlockRate(pRawBuffer[i])) {

				uid = COINC::GetUnitID(pRawBuffer[i]);
				blkX = COINC::GetBlockIDX(pRawBuffer[i]);
				br_temp = COINC::GetBlockRate(pRawBuffer[i]);
				blkY = COINC::GetBlockIDY(pRawBuffer[i]) + (uid * 14);
				modA = floor(blkX / 5) + (24 * floor(blkY / 14));
				ind = blkX + (120 * blkY); // global block ID

				block_rate_new[ind] = block_rate_new[ind]
						+ ((double) br_temp * 10.0);
				block_rate_counts[ind] += 1.0;
				module_rate_new[modA] = module_rate_new[modA]
						+ ((double) br_temp * 10.0);
				module_rate_counts[modA] = module_rate_counts[modA]
						+ (1.0 / 70.0);
			} else if (COINC::IsTimestamp(pRawBuffer[i]) || next_frame) { // if timestamp or near eof (next_frame flag)
				day1 = COINC::GetDay(pRawBuffer[i]);
				year1 = COINC::GetYear(pRawBuffer[i]);
				month1 = COINC::GetMonth(pRawBuffer[i]);
				milli1 = COINC::GetMillisecond(pRawBuffer[i]);
				second1 = COINC::GetSecond(pRawBuffer[i]);
				minute1 = COINC::GetMinute(pRawBuffer[i]);
				hour1 = COINC::GetHour(pRawBuffer[i]);

				ssdiff = milli1 - ss0;
				if (ssdiff < 0) {
					ssdiff = ssdiff + 1000;
				}
				time_s = time_s + ssdiff;
				ss0 = milli1;

				if (time_s >= r_frame_end) {
					r_frame_end = r_frame_end + r_frame;
					time_sd = (double) time_s;
					// write data
					if (write_singles) {
						outfile_singles.write(
								reinterpret_cast<const char*>(&time_sd),
								sizeof(double));
						outfile_singles.write((char*) block_rate.data(),
								block_rate.size() * sizeof(double));
						outfile_singles_module.write(
								reinterpret_cast<const char*>(&time_sd),
								sizeof(double));

						outfile_singles_module.write((char*) module_rate.data(),
								module_rate.size() * sizeof(double));
					}
					if (write_coincidences) {
						outfile_prompts_rate.write(
								reinterpret_cast<const char*>(&time_sd),
								sizeof(double));
						outfile_randoms_rate.write(
								reinterpret_cast<const char*>(&time_sd),
								sizeof(double));
						outfile_prompts_rate.write((char*) prompt_rate.data(),
								prompt_rate.size() * sizeof(double));
						outfile_randoms_rate.write((char*) random_rate.data(),
								random_rate.size() * sizeof(double));
					}
					if (write_module_sinos) {
						outfile_pmod_sino.write(
								reinterpret_cast<const char*>(&time_sd),
								sizeof(double));
						outfile_rmod_sino.write(
								reinterpret_cast<const char*>(&time_sd),
								sizeof(double));
						outfile_pmod_sino.write((char*) sino_module_avg.data(),
								sino_module_avg.size() * sizeof(double));
						outfile_rmod_sino.write(
								(char*) sino_module_r_avg.data(),
								sino_module_r_avg.size() * sizeof(double));
					}
					// add assorted data
					for (int rk = 0; rk < num_bins_sino_module * 8 * 8; rk++) {
						sino_module_r_avg[rk] = sino_module_r_new[rk];
						sino_module_r_new[rk] = 0.0;

						sino_module_avg[rk] = sino_module_new[rk];
						sino_module_new[rk] = 0.0;
					}
					for (int bk = 0; bk < num_blocks; bk++) {
						if (block_rate_counts[bk] > 1) {
							block_rate[bk] = block_rate_new[bk]
									/ block_rate_counts[bk];
						} else {
							block_rate[bk] = 1000.0;
							block_rate[bk] = 0.0;
						}
						block_rate_new[bk] = 0.0;
						block_rate_counts[bk] = 0.0;
					}
					for (int mmk = 0; mmk < num_mod; mmk++) {
						//if (prompt_rate_new[mmk] > 10.0) {
						prompt_rate[mmk] = prompt_rate_new[mmk];
						//} else {
						//	prompt_rate[mmk] = 500.0 * ((double)r_frame / 1000.0);
						//	cout << "Low count data\n";
						//}
						prompt_rate_new[mmk] = 0.0;

						//if (random_rate_new[mmk] > 10.0) {
						random_rate[mmk] = random_rate_new[mmk];
						//} else {
						//	random_rate[mmk] = 475.0 * ((double)r_frame / 1000.0);
						//cout << "Low count data\n";
						//}
						random_rate_new[mmk] = 0.0;

						if (module_rate_counts[mmk] > 1.0) {
							module_rate[mmk] = module_rate_new[mmk]
									/ module_rate_counts[mmk];
						} else {
							module_rate[mmk] = 0.0;
						}
						module_rate_new[mmk] = 0.0;
						module_rate_counts[mmk] = 0.0;
					}
					// make dead-time block pair array
					for (int bbi = 0; bbi < (24 * 8); bbi++) {
						for (int bbj = 0; bbj < (24 * 8); bbj++) {
							DTtemp1 = 1.0;
							DTtemp2 = 1.0;
							// 1
							floodtemp = module_rate[bbi] - singles_bkg;
							if (floodtemp < 10.0) {
								floodtemp = 10.0;
							}
							xx = floodtemp;
							ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
							mm = (tau_s * xx - 1.0) * exp(-1.0 * tau_s * xx);
							xxnew = xx - (ff / mm);
							// Newton root finder
							for (int iii = 0; iii < 5; iii++) {
								xx = xxnew;
								xxnew = 0.0;
								ff = 0.0;
								ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
								mm = 0.0;
								mm = (tau_s * xx - 1.0)
										* exp(-1.0 * tau_s * xx);
								xxnew = xx - (ff / mm);
							}
							xx = 0.0;
							mm = 0.0;
							ff = 0.0;
							DTtemp1 = xxnew / floodtemp;
							xxnew = 0.0;
							floodtemp = 0.0;
							// 2
							floodtemp = module_rate[bbj] - singles_bkg;
							if (floodtemp < 10.0) {
								floodtemp = 10.0;
							}
							xx = floodtemp;
							ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
							mm = (tau_s * xx - 1.0) * exp(-1.0 * tau_s * xx);
							xxnew = xx - (ff / mm);
							// Newton root finder
							for (int iii = 0; iii < 5; iii++) {
								xx = xxnew;
								xxnew = 0.0;
								ff = 0.0;
								ff = floodtemp - xx * exp(-1.0 * tau_s * xx);
								mm = 0.0;
								mm = (tau_s * xx - 1.0)
										* exp(-1.0 * tau_s * xx);
								xxnew = xx - (ff / mm);
							}
							xx = 0.0;
							mm = 0.0;
							ff = 0.0;

							DTtemp2 = xxnew / floodtemp;
							xxnew = 0.0;
							//DTtemp = (DTtemp1 + DTtemp2) / 2.0;
							DTtemp = sqrt(DTtemp1) * sqrt(DTtemp2);
							//if (DTtemp < 0.99) {
							//	DTtemp = 1.0;
							//}
							//if (isnan(DTtemp) || isinf(DTtemp)) {
							//	DTtemp = 1.0;
							//}
							//if (DTtemp > 3.0) {
							//	DTtemp = 3.0;
							//}
							//DT_fac[i][j] = DT_fac[i][j] + floodtemp * DTtemp;
							//DT_fac[bbi][bbj] = 1.0 / DTtemp;
							DT_fac[bbi][bbj] = DTtemp;
						}
					}
					// make dead-time block pair array
				} // end if statement

				if (time_s >= frame_end || next_frame) { // near feof?
					//fnum = fnum + 1;

					ss0 = milli00;
					//time_s = time_s0;
					next_frame = false;
					cout << "Cur time: " << hour1 << ":" << minute1 << ":"
							<< second1 << "." << milli1 << "\n";
					cout << byte_location + 8 << endl; // used to be ftell

					milli00 = milli1;
					ss0 = milli00;
					time_s0 = frame_end;
					time_s = time_s0;
					write_lm = true;

					//time_tag = false;
					//cout << "file num: " << fnum << "\n";
				} //else {
				  //time_s = time_s0;
				  //cout << "file num: " << fnum << "\n";
				  //}
			} else if (COINC::IsCoinc(pRawBuffer[i])) {

				if (num_coinc > cou) { // counter for keeping track of number of events
					cout << "num events = " << num_coinc << "\n";
					cou = cou + 100000000.0;
				}

				num_coinc += 1.0;

				unitA = COINC::GetUiA(pRawBuffer[i]);
				unitB = COINC::GetUiB(pRawBuffer[i]);

				crys1 = COINC::GetCrysA(pRawBuffer[i]);
				crys2 = COINC::GetCrysB(pRawBuffer[i]);

				bankk = COINC::GetBankPair(pRawBuffer[i]) - 1;
				//bankk = bankk - 1;

				modA = bank_lut[bankk][0];
				modB = bank_lut[bankk][1];

				transA = (crys1 % 70) + (modA * 70);
				transB = (crys2 % 70) + (modB * 70); 

				axA = floor(crys1 / 70) + (unitA * 84);
				axB = floor(crys2 / 70) + (unitB * 84);
/*
				keep_event = SUBS.KeepEvent(axA, axB, transA, transB);
				if (!keep_event){
				//	cout << "coincidences A,B (trans/ax):\t(" << transA << "/" << axA << "),\t(" << transB << "/" << axB << ")" << endl;
				//	cout << "keep event " << num_coinc << " (1=yes):\t" << keep_event << endl;
					num_coinc -= 1.0;
					continue;
				}		
*/
				
				blkXa = floor(transA / 7);
				blkXb = floor(transB / 7);
				blkYa = floor(axA / 6);
				blkYb = floor(axB / 6);
				blk_absA = blkXa + (120 * blkYa);
				blk_absB = blkXb + (120 * blkYb);

				transcA = (signed short) transA;
				transcB = (signed short) transB;
				crysaxA = (signed short) axA;
				crysaxB = (signed short) axB;

				modA = floor(transA / 35);
				modB = floor(transB / 35);

				// add timing offset to dt;

				crys1 = transcA + (840 * crysaxA);
				crys2 = transcB + (840 * crysaxB);

				//crys1 = crysaxA + (transcA*672);
				//crys2 = crysaxB + (transcB*672);


				crysaxA = crysaxA + (short) unitA; // add unit gap
				crysaxB = crysaxB + (short) unitB; // add unit gap

				short dout[5]; // 10-byte list-mode data format
				dout[0] = COINC::GetTxID1(pRawBuffer[i]);
				dout[1] = COINC::GetAxID1(pRawBuffer[i]) + (short) unitA; // add unit gap
				dout[2] = COINC::GetTxID2(pRawBuffer[i]);
				dout[3] = COINC::GetAxID2(pRawBuffer[i]) + (short) unitB; // add unit gap
				dout[4] = COINC::GetTATB(pRawBuffer[i]);

				//unit_diff = abs(unitA - unitB);
				t_window = coinc_window[abs(unitA - unitB)];

				/*
				 if (!prompt) {
				 sec1 = floor(transcA / 35);
				 sec2 = floor(transcB / 35);

				 sector_counts[sec1][sec2] = sector_counts[sec1][sec2] + 1.0;
				 sector_counts[sec2][sec1] = sector_counts[sec2][sec1] + 1.0;

				 ind = transcA + (840 * crysaxA) + (sec2 * 570360);
				 sector_flood[ind] = sector_flood[ind] + 1.0;
				 ind = transcB + (840 * crysaxB) + (sec1 * 570360);
				 sector_flood[ind] = sector_flood[ind] + 1.0;
				 }
				 */

				ind = noindex_crystalpairs_transaxial_int16[transcA][transcB];

				ind_block = noindex_blockpairs_transaxial_int16[blkXa][blkXb];

				ind_module_trans =
						noindex_modulepairs_transaxial_int16[modA][modB];

				ind_module1 = ind_module_trans + (num_bins_sino_module * unitA)
						+ (num_bins_sino_module * 8 * unitB);
				ind_module2 = ind_module_trans + (num_bins_sino_module * unitB)
						+ (num_bins_sino_module * 8 * unitA);
				if (ind_module_trans >= 0) {
					if (COINC::IsDelayFlag(pRawBuffer[i])) {
						random_rate_new[modA + 24 * unitA] =
								random_rate_new[modA + 24 * unitA] + 1.0;
						random_rate_new[modB + 24 * unitB] =
								random_rate_new[modB + 24 * unitB] + 1.0;
						sino_module_r_new[ind_module1] =
								sino_module_r_new[ind_module1] + 1.0;
						//if (ind_module1 != ind_module2) {
						sino_module_r_new[ind_module2] =
								sino_module_r_new[ind_module2] + 1.0;
						//}
					} else {
						prompt_rate_new[modA + 24 * unitA] =
								prompt_rate_new[modA + 24 * unitA] + 1.0;
						prompt_rate_new[modB + 24 * unitB] =
								prompt_rate_new[modB + 24 * unitB] + 1.0;
						sino_module_new[ind_module1] =
								sino_module_new[ind_module1] + 1.0;
						//if (ind_module1 != ind_module2) {
						sino_module_new[ind_module2] =
								sino_module_new[ind_module2] + 1.0;
						//}
					}
				}

				if (ind >= 0) {
					if (!COINC::IsDelayFlag(pRawBuffer[i])) { // prompt event
						if ((ind_block > 0)
								&& (abs(blkYa - blkYb) <= block_ax_span)) {
							sino_block[ind_block2]++;
						}
						if (write_lmfile) {
							pids_p[p_index].txID1 = dout[0];
							pids_p[p_index].axID1 = dout[1];
							pids_p[p_index].txID2 = dout[2];
							pids_p[p_index].axID2 = dout[3];
							pids_p[p_index].tof = dout[4];
							p_index++;
							if (p_index == BUFFER_SIZE) {
								fwrite(pids_p, sizeof(ids), BUFFER_SIZE,
										pPromptFile);
								p_index = 0;
							}
							
							mtemp = 1.0;
							mtemp = (float) DT_fac[modA + 24 * unitA][modB
									+ 24 * unitB];
							

							if (r_singles) {
								singles_c1 = block_rate[blk_absA] / 42.0;
								singles_c2 = block_rate[blk_absB] / 42.0;
								rstemp = singles_c1 * singles_c2
										* frame_length_s[frame_num];
								if (isnan(rstemp) || isinf(rstemp)) {
									rstemp = 0.0;
								}
								rtemp = (float) rstemp;
								rtemp = rtemp * (39.0625E-12);
								//rtemp = rtemp * t_window * 1E-12;
							} else {

								rtemp = (float) sino_module_r_avg[ind_module1]
										* ((float) frame_length[frame_num]
												/ (float) r_frame);

								//rtemp = rtemp / (2.0 * (float)num_lor_modpair);
								if (unit_diff == 0) {
									rtemp = rtemp
											/ (2.0 * (float) num_lor_modpair);
								} else {
									rtemp = rtemp / ((float) num_lor_modpair);
								}

								rtemp = rtemp * (39.0625 / t_window);
								rtemp = rtemp * mtemp;

								//rtemp = rtemp * (nc_crys[crys1] / nc_mod[modA]) * (nc_crys[crys2] / nc_mod[modB]);

								//rtemp = rtemp * plane_efficiency[abs(unit_diff)][crys_ring_diff];
							}

							// normalization crystal efficiency
							//mtemp = (1.0 / nc_crys[crys1]) * (1.0 / nc_crys[crys2]);
							//mtemp = (nc_crys[crys1]) * (nc_crys[crys2]);

							//michel_ind = axA + (672*axB);
							//mtemp = mtemp * plaeff[michel_ind];

							

							outfile_s.write(
									reinterpret_cast<const char*>(&rtemp),
									sizeof(float));

							outfile_m.write(
									reinterpret_cast<const char*>(&mtemp),
									sizeof(float));
						}
						num_prompts += 1.0;
					} else { // delay event

						if (write_lmfile) {
							for (int yy = 0; yy < 5; yy++) {
								//outfile_r.write(reinterpret_cast<const char *>(&dout[yy]), sizeof(dout[yy]));
							}
						}
						num_randoms += 1.0;
					}
				}
				//} else if (COINC::IsBedPos(pRawBuffer[i])) {
				//} else if (COINC::IsPdRate(pRawBuffer[i])) {
				//} else {
				//	cout << "Unknown event detected at " << byte_location << endl;
			}
			// end check event type
			byte_location += 8; // move to next event (8 bytes)

			// =============== spit out files ===============

			if (write_lm == true || run == false || feof(pInputFile)) { // feof makes no sense here due to the hard-coded file_size1 - 100
			// run == false also is impossible since run is only changed down below

				frameinfo_fullpath = "";
				fname_out = "";
				stringstream ss;
				ss << "lm_info_f" << (frame_num);
				fname_out = ss.str();
				frameinfo_fullpath = outfolder;
				cout << outfolder << "\n";
				frameinfo_fullpath.append(fname_out);
				ofstream frameinfo_out;
				frameinfo_out.open(frameinfo_fullpath.c_str());
				cout << "info out :\t" << frameinfo_fullpath << "\n";
				if (!frameinfo_out) {
					cout << "Could not create frame info file\n";
				}
				ss << "";
				ss.clear();
				fname_out = "";

				ss << "startDateTime=" << year00 << "," << month00 << ","
						<< day00 << "," << hour00 << "," << minute00 << ","
						<< second00 << "\n" << "endDateTime=" << year1 << ","
						<< month1 << "," << day1 << "," << hour1 << ","
						<< minute1 << "," << second1 << "\n" << "frame_start="
						<< (frame_start) << "\n" << "frame_length="
						<< (frame_end - frame_start) << "\n" << byte_location
						<< "\n"; // not byte_location + 8 since it used to be ftell

				str_temp = ss.str();
				frameinfo_out << str_temp;
				ss << "";
				ss.clear();
				str_temp = "";
				frameinfo_out.close();

				// close and flush (if applicable) output lm files
				fwrite(pids_p, sizeof(ids), p_index, pPromptFile);
				fclose(pPromptFile);
				p_index = 0; // reset buffer index
				//outfile_r.close();
				outfile_s.close();
				outfile_m.close();

				//for (int i = 0; i < 24; i++) {
				//	for (int j = 0; j < 24; j++) {
				//		sector_counts[i][j] = 0.0;
				//	}
				//}
				//for (int i = 0; i < (570360 * 24); i++) {
				//	sector_flood[i] = 0.0;
				//}

				// Create files for next frame, or quit if end frame is reached
				frame_num++;

				if (frame_num == tot_frames || feof(pInputFile)) {

					run = false;
					cout << "Scan time: " << (time_s / 1000) << " seconds\n";
					cout
							<< "\nFinish listmode processing and sinogram binning!\n";
					break; // break out of for loop
				} else {
					cout << "\n\n*** Processing Frame " << frame_num
							<< "... ***\n\n";
					frame_start = frame_end;
					frame_end = frame_start + frame_length[frame_num];
					if (write_lmfile) {
						stringstream ss;
						ss << "lm_reorder_f" << frame_num;
						fname_out = ss.str();
						outfile_fullpath_p = outfolder + fname_out + "_prompts."
								+ raw_num + ".lm";
						outfile_fullpath_r = outfolder + fname_out + "_randoms."
								+ raw_num + ".lm";
						outfile_fullpath_s = outfolder + fname_out + "_prompts."
								+ raw_num + ".add_fac";
						outfile_fullpath_m = outfolder + fname_out + "_prompts."
								+ raw_num + ".mul_fac";

						ss << "";
						ss.clear();
						str_temp = "";
						fname_out = "";

						pPromptFile = fopen(outfile_fullpath_p.c_str(), "wb");
						//outfile_r.open(outfile_fullpath_r.c_str(), ios::out | ios::binary);
						outfile_s.open(outfile_fullpath_s.c_str(),
								ios::out | ios::binary);
						outfile_m.open(outfile_fullpath_m.c_str(),
								ios::out | ios::binary);

					}

					num_prompts = 0;
					num_randoms = 0;
					write_lm = false;
				}
			} // end spit out files - this is the biggest curly brace where the break out of for loop happens
		} // end for loop
	} // end while loop

	// ==================== cleanup ====================
	cout << "cleaning up... " << endl;
	fclose(pInputFile);

	if (pPromptFile) { // why bother?
		fwrite(pids_p, sizeof(ids), p_index, pPromptFile); // flush
		//fclose(pPromptFile);	// causes core dump for whatever the reason might be...
		//outfile_r.close();
		outfile_s.close();
		outfile_m.close();
	}
	outfile_singles.close();
	outfile_singles_module.close();
	outfile_randoms_rate.close();
	outfile_prompts_rate.close();

	outfile_fullpath_p = "";

	fname_out = "";

	stringstream sss;

	sss << "lm_reorder_f" << frame_num;
	fname_out = sss.str();
	outfile_fullpath_p = outfolder;

	outfile_fullpath_p.append(fname_out);

	outfile_fullpath_p.append("_prompts.");
	outfile_fullpath_p.append(raw_num);
	outfile_fullpath_p.append(".lm");

	sss << "";
	sss.clear();
	str_temp = "";
	fname_out = "";

	pPromptFile = fopen(outfile_fullpath_p.c_str(), "wb");
	fwrite(&rtemp, sizeof(float), 1, pPromptFile); // dummy file for recon scheduler
	fclose(pPromptFile);

	delete[] pRawBuffer;
	delete[] pids_p;

	cout << " done" << endl;
	return 0;

}
