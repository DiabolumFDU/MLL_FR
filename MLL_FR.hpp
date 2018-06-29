#ifndef MLL_FR_H
#define MLL_FR_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h> //for strerror
#include <stdint.h> //for uint32_t
#include <assert.h>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <ctgmath>
#include <cmath>


#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define LOG_DEBUG
#define LOG_TBT
#define LOG_IBP
#define LOG_VLC

#ifdef LOG_DEBUG
#define ylog(x,...) printf(x, __VA_ARGS__)
#else
#define ylog(x,...) 
#endif

using namespace cv;
using namespace std;

class IBP_mode_candidate
{
public:
	int mode1;
	int mode2;
	int mode3;
};

class MLL_FR
{
public:
	MLL_FR();
	~MLL_FR() {};
	void set_bitfile(char* filename) { bitfile = filename; }
	void set_input8x8(Mat in);
	Mat& get_input8x8() { return input8x8; }
	Mat& get_output8x8() { return output8x8; }
	void set_intraMode(int intraMode) { Intra_mode = intraMode; confirm_mode_candidate(); }
	void confirm_mode_candidate();
	int TBT_enc(int b);
	int TBT_dec(int b);
	int IBP_enc();
	int IBP_dec();
	void compute_BP_res();
	uchar NP_predict_one_pixel(int mode, uchar in1, uchar in2, uchar in3, uchar in4);
	Mat compute_NP_res(int mode);
	uchar compute_NP_res_abs_sum(Mat m);
	int VLC_enc();
	string VLC_enc_4x4(Mat m);
	uchar max_in_Mat(Mat m);
	int VLC_dec();
	void VLC_dec_4x4(int& index, Mat m);
	string expGolomb_enc(uchar in);
	uchar expGolomb_dec(string in);
	string dec2bin(uchar in);
	uchar bin2dec(string in);
	void run_all();
	void encoding();

private:
	Mat input8x8;
	Mat output8x8;
	Mat tr;
	Mat pr_enc;
	Mat pr_dec;
	Mat IBP_enc_res;
	Mat IBP_dec_res;
	Mat IBP_enc_NP_res[3];
	char* bitfile;
	string buf4bitfile;
	IBP_mode_candidate mode_candidate;
	int IBP_mode;
	int Intra_mode;
};







#endif

