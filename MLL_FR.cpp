#include "MLL_FR.hpp"

MLL_FR::MLL_FR()
{
	Intra_mode = 0;
	confirm_mode_candidate();
	bitfile = "EncodingResult.txt";
	output8x8.create(8, 8, CV_8UC1);
	tr.create(1, 64, CV_8UC1);
	pr_enc.create(8, 8, CV_8UC1);
	pr_dec.create(8, 8, CV_8UC1);
	IBP_enc_res.create(8, 8, CV_8UC1);
	IBP_dec_res.create(8, 8, CV_8UC1);
	//IBP_enc_BP_res[0].create(1, 7, CV_8UC1);
	//IBP_enc_BP_res[1].create(1, 7, CV_8UC1);
	//IBP_enc_NP_res[0].create(7, 7, CV_8UC1);
	//IBP_enc_NP_res[1].create(7, 7, CV_8UC1);
	//IBP_enc_NP_res[2].create(7, 7, CV_8UC1);
}

void MLL_FR::set_input8x8(Mat in)
{
	input8x8 = in;
	assert((input8x8.rows == 8) && (input8x8.cols == 8));
}

void MLL_FR::confirm_mode_candidate()
{
	assert((Intra_mode >= 0) && (Intra_mode <= 34));
	if(Intra_mode < 2)
	{
		mode_candidate.mode1 = 4;
		mode_candidate.mode2 = 5;
		mode_candidate.mode3 = 7;
	}
	else if(Intra_mode < 11)
	{
		mode_candidate.mode1 = 0;
		mode_candidate.mode2 = 4;
		mode_candidate.mode3 = 5;
	}
	else if (Intra_mode < 19)
	{
		mode_candidate.mode1 = 0;
		mode_candidate.mode2 = 2;
		mode_candidate.mode3 = 6;
	}
	else if (Intra_mode < 27)
	{
		mode_candidate.mode1 = 1;
		mode_candidate.mode2 = 5;
		mode_candidate.mode3 = 6;
	}
	else 
	{
		mode_candidate.mode1 = 1;
		mode_candidate.mode2 = 3;
		mode_candidate.mode3 = 4;
	}
}

int MLL_FR::TBT_enc(int b)
{
	assert((b > 0) && (b < 8));
	int mask = (1 << b) - 1;
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			tr.at<uchar>(i*8 + j) = input8x8.at<uchar>(i, j) & mask;
			pr_enc.at<uchar>(i, j) = input8x8.at<uchar>(i, j) >> b;
		}
	}
#ifdef LOG_TBT
	cout << "After TBT_enc" << endl;
	cout << "tr = " << endl << tr << endl;
	cout << "pr_enc = " << endl << pr_enc << endl;
#endif
	return 0;
}

int MLL_FR::TBT_dec(int b)
{
	assert((b > 0) && (b < 8));
	//pr_dec = pr_enc; //Only for debug!!!
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			output8x8.at<uchar>(i, j) = (pr_dec.at<uchar>(i, j) << b) + tr.at<uchar>(i * 8 + j);
		}
	}
#ifdef LOG_TBT
	cout << "After TBT_dec" << endl;
	cout << "output8x8 = " << endl << output8x8 << endl;
	cout << "diff_in_out = " << endl << output8x8 - input8x8 << endl;

#endif
	return 0;
}

int MLL_FR::IBP_enc()
{
	uchar sum1, sum2, sum3;
	int index;
	IBP_enc_res.at<uchar>(0, 0) = pr_enc.at<uchar>(0, 0);
	compute_BP_res();

	IBP_enc_NP_res[0] = compute_NP_res(mode_candidate.mode1);
	sum1 = compute_NP_res_abs_sum(IBP_enc_NP_res[0]);
	IBP_enc_NP_res[1] = compute_NP_res(mode_candidate.mode2);
	sum2 = compute_NP_res_abs_sum(IBP_enc_NP_res[1]);
	IBP_enc_NP_res[2] = compute_NP_res(mode_candidate.mode3);
	sum3 = compute_NP_res_abs_sum(IBP_enc_NP_res[2]);

	//Mode Decision
	if (sum1 < sum2)
	{
		if (sum1 < sum3)
		{
			IBP_mode = mode_candidate.mode1;
			index = 0;
		}
		else
		{
			IBP_mode = mode_candidate.mode3;
			index = 2;
		}
	}
	else
	{
		if (sum2 < sum3)
		{
			IBP_mode = mode_candidate.mode2;
			index = 1;
		}
		else
		{
			IBP_mode = mode_candidate.mode3;
			index = 2;
		}
	}
	Mat best = IBP_enc_res(Rect(1, 1, 7, 7));
	IBP_enc_NP_res[index].copyTo(best);

#ifdef LOG_IBP
	cout << "IBP_enc_NP_res[0]" << endl << IBP_enc_NP_res[0] << endl;
	ylog("sum1 = %d\n", sum1);
	cout << "IBP_enc_NP_res[1]" << endl << IBP_enc_NP_res[1] << endl;
	ylog("sum2 = %d\n", sum2);
	cout << "IBP_enc_NP_res[2]" << endl << IBP_enc_NP_res[2] << endl;
	ylog("sum3 = %d\n", sum3);
	cout << "IBP_enc_res = " << endl << IBP_enc_res << endl;
#endif

	return 0;
}

int MLL_FR::IBP_dec()
{
	//IBP_dec_res = IBP_enc_res; // Only for debug!!!
	for(int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if ((i == 0) && (j == 0)) //IP
				pr_dec.at<uchar>(i, j) = IBP_dec_res.at<uchar>(0, 0);
			else if (i == 0) //BP_row
				pr_dec.at<uchar>(i, j) = pr_dec.at<uchar>(i, j - 1) + IBP_dec_res.at<uchar>(0, j);
			else if (j == 0) //BP_col
				pr_dec.at<uchar>(i, j) = pr_dec.at<uchar>(i - 1, j) + IBP_dec_res.at<uchar>(i, 0);
			else if ((j + 1) < 8) //NP
				pr_dec.at<uchar>(i, j) = NP_predict_one_pixel(IBP_mode, pr_dec.at<uchar>(i, j - 1), pr_dec.at<uchar>(i - 1, j - 1),
					pr_dec.at<uchar>(i - 1, j), pr_dec.at<uchar>(i - 1, j + 1)) + IBP_dec_res.at<uchar>(i, j);
			else //NP_padding
				pr_dec.at<uchar>(i, j) = NP_predict_one_pixel(IBP_mode, pr_dec.at<uchar>(i, j - 1), pr_dec.at<uchar>(i - 1, j - 1),
					pr_dec.at<uchar>(i - 1, j), pr_dec.at<uchar>(i - 1, j)) + IBP_dec_res.at<uchar>(i, j);
		}
	}

#ifdef LOG_IBP
	cout << "pr_dec = " << endl << pr_dec << endl;
	cout << "diff_of_pr = " << endl << pr_dec - pr_enc << endl;
#endif
	return 0;
}

void MLL_FR::compute_BP_res()
{
	Mat res;
	//row_BP
	res = IBP_enc_res(Range(0, 1), Range(1, 8));
	for (int j = 0; j < 7; j++)
	{
		res.at<uchar>(j) = pr_enc.at<uchar>(0, j + 1) - pr_enc.at<uchar>(0, j);
	}

	//col_BP
	res = IBP_enc_res(Range(1, 8), Range(0, 1));
	for (int i = 0; i < 7; i++)
	{
		res.at<uchar>(i) = pr_enc.at<uchar>(i + 1, 0) - pr_enc.at<uchar>(i, 0);
	}
	//cout << "IBP_enc_res = " << endl << IBP_enc_res << endl;
}

uchar MLL_FR::NP_predict_one_pixel(int mode, uchar in1, uchar in2, uchar in3, uchar in4)
{
	assert((mode >= 0) && (mode < 8));
	uchar res;
	switch(mode)
	{
	case 0:{
		res = in1;
		break;
	}
	case 1: {
		res = in3;
		break;
	}
	case 2: {
		res = (in1 + in2) >> 1;
		break;
	}
	case 3: {
		res = (in3 + in4) >> 1;
		break;
	}
	case 4: {
		res = (in1 + in4) >> 1;
		break;
	}
	case 5: {
		res = (in1 + in3) >> 1;
		break;
	}
	case 6: {
		res = ( ((in1 + in2) >> 1) + in3 ) >> 1;
		break;
	}
	case 7: {
		res = ( ((in1 + in2) >> 1) + ((in3 + in4) >> 1) ) >> 1;
		break;
	}
	}
	return res;
}

Mat MLL_FR::compute_NP_res(int mode)
{
	Mat res(7, 7, CV_8UC1);
	for( int i = 0; i < 7; i++ )
	{
		for (int j = 0; j < 7; j++ )
		{
			if (j + 2 < 8)
			{
				res.at<uchar>(i, j) = pr_enc.at<uchar>(i + 1, j + 1) -
					NP_predict_one_pixel(mode, pr_enc.at<uchar>(i + 1, j), pr_enc.at<uchar>(i, j),
						pr_enc.at<uchar>(i, j + 1), pr_enc.at<uchar>(i, j + 2));
			}
			else
			{
				res.at<uchar>(i, j) = pr_enc.at<uchar>(i + 1, j + 1) -
					NP_predict_one_pixel(mode, pr_enc.at<uchar>(i + 1, j), pr_enc.at<uchar>(i, j),
						pr_enc.at<uchar>(i, j + 1), pr_enc.at<uchar>(i, j + 1)); //How padding?
			}
		}
	}
	return res;
}

uchar MLL_FR::compute_NP_res_abs_sum(Mat m)
{
	uchar sum = 0;
	uchar tmp;
	for(int i = 0; i < 7; i++)
	{
		for(int j = 0; j < 7; j++)
		{
			tmp = m.at<uchar>(i, j);
			if(tmp > 127)
			{
				tmp = 255 - tmp + 1;
			}
			sum += tmp;
		}
	}
	return sum;
}

int MLL_FR::VLC_enc()
{
	ofstream fout;
	Mat tmp;
	string mode = dec2bin(IBP_mode);
	mode = string(3 - mode.size(), '0') + mode;
	fout.open(bitfile);
	fout << mode;

	for (int i = 0; i < 4; i++)
	{
		tmp = IBP_enc_res(Rect((i % 2)*4, (i / 2)*4, 4, 4));

#ifdef LOG_VLC
		cout << "tmp" << i << " = " << endl << tmp << endl;
#endif
		fout << VLC_enc_4x4(tmp);
		//cout << VLC_enc_4x4(tmp);
	}
	fout.close();
	return 0;
}

string MLL_FR::VLC_enc_4x4(Mat m)
{
	string res = "";
	int flag;
	uchar buf;
	string tmp;

	uchar max = max_in_Mat(m);
	MatIterator_<uchar> pixelIter, pixelEnd;
	if (max == 0)
	{
		res = "01";
	}
	else if (max == 1)
	{
		res = "10";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			buf = (*pixelIter);
			if (buf == 0)
				res = res + "1";
			else if (buf == 1)
				res = res + "00";
			else if (buf == 255)
				res = res + "01";
		}
	}
	else if (max == 2)
	{
		res = "00";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			buf = (*pixelIter);
			if (buf == 0)
				res = res + "01";
			else if (buf == 1)
				res = res + "10";
			else if (buf == 255)
				res = res + "11";
			else if (buf == 2)
				res = res + "000";
			else if (buf == 254)
				res = res + "001";
		}
	}
	else if (max < 5)
	{
		res = "110";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			flag = 0;
			buf = (*pixelIter);
			if (buf == 0)
				res = res + "001";
			else if (buf == 4)
				res = res + "0000";
			else if (buf == 252)
				res = res + "0001";
			else
			{
				if (buf > 127)
				{
					flag = 1;
					buf = 255 - buf + 1;
				}
				tmp = dec2bin(buf);
				res = res + string(2 - tmp.size(), '0') + tmp + (flag ? '1' : '0');
			}
		}
	}
	else
	{
		res = "111";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			buf = (*pixelIter);
			res = res + expGolomb_enc(buf);
		}
	}
#if 0
	else if (max < 9)
	{
		res = "1110";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			flag = 0;
			buf = (*pixelIter);
			if (buf == 0)
				res = res + "0001";
			else if (buf == 8)
				res = res + "00000";
			else if (buf == 248)
				res = res + "00001";
			else
			{
				if (buf > 127)
				{
					flag = 1;
					buf = 255 - buf + 1;
				}
				tmp = dec2bin(buf);
				res = res + string(3 - tmp.size(), '0') + tmp + (flag ? '1' : '0');
			}
		}
	}
	else if (max < 17)
	{
		res = "11110";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			flag = 0;
			buf = (*pixelIter);
			if (buf == 0)
				res = res + "00001";
			else if (buf == 16)
				res = res + "000000";
			else if (buf == 240)
				res = res + "000001";
			else
			{
				if (buf > 127)
				{
					flag = 1;
					buf = 255 - buf + 1;
				}
				tmp = dec2bin(buf);
				res = res + string(4 - tmp.size(), '0') + tmp + (flag ? '1' : '0');
			}
		}
	}
	else if (max < 33)
	{
		res = "111110";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			flag = 0;
			buf = (*pixelIter);
			if (buf == 0)
				res = res + "00001";
			else if ((buf <= 11) || (buf >= 245))
			{
				if (buf > 127)
				{
					flag = 1;
					buf = 255 - buf + 1;
				}
				tmp = dec2bin(buf);
				res = res + string(4 - tmp.size(), '0') + tmp + (flag ? '1' : '0');
			}
			else if ((buf <= 15) || (buf >= 241))
			{
				if (buf > 127)
				{
					flag = 1;
					buf = 255 - buf + 1;
				}
				tmp = dec2bin(buf - 12);
				res = res + "110" + string(2 - tmp.size(), '0') + tmp + (flag ? '1' : '0');
			}
			else if ((buf <= 31) || (buf >= 225))
			{
				if (buf > 127)
				{
					flag = 1;
					buf = 255 - buf + 1;
				}
				tmp = dec2bin(buf - 16);
				res = res + "111" + string(4 - tmp.size(), '0') + tmp + (flag ? '1' : '0');
			}
			else if (buf == 32)
				res = res + "00000000";
			else if (buf == 224)
				res = res + "00000001";
		}
	}
	else 
	{
		res = "111111";
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			buf = (*pixelIter);
			res = res + expGolomb_enc(buf);
		}
	}
#endif //full table

#ifdef LOG_VLC
	cout << "res = " << res << endl;
#endif
	return res;
}

uchar MLL_FR::max_in_Mat(Mat m)
{
	double min, max;
	MatIterator_<uchar> pixelIter, pixelEnd;
	Mat mLocal;
	m.copyTo(mLocal);
	for (pixelIter = mLocal.begin<uchar>(), pixelEnd = mLocal.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
	{
		if ((*pixelIter) > 127)
			(*pixelIter) = 255 - (*pixelIter) + 1;
	}
	minMaxIdx(mLocal, &min, &max);
	//cout << "max = " << max << endl;
	return max;
}

int MLL_FR::VLC_dec()
{
	ifstream fin;
	fin.open(bitfile);
	Mat curMat;
	int bufSize;
	fin.seekg(0, ios::end);
	bufSize = fin.tellg();
	fin.seekg(0, ios::beg);
	ylog("bufSize = %d\n", bufSize);
	buf4bitfile = (char*)malloc(sizeof(char)*bufSize);
	fin >> buf4bitfile;
	//stdout
	cout << buf4bitfile << endl;
	fin.close();

	int index = 0;
	IBP_mode = bin2dec(buf4bitfile.substr(0, 3));
	ylog("IBP_mode = %d\n", IBP_mode);
	index += 3;

	for (int i = 0; i < 4; i++)
	{
		curMat = IBP_dec_res(Rect((i % 2) * 4, (i / 2) * 4, 4, 4));
		VLC_dec_4x4(index, curMat);
#ifdef LOG_VLC
		cout << "curMat" << i << " = " << endl << curMat << endl;
#endif
	}

#ifdef LOG_VLC
	cout << "IBP_dec_res = " << endl << IBP_dec_res << endl;
#endif

	return 0;
}

void MLL_FR::VLC_dec_4x4(int& index, Mat m)
{

	MatIterator_<uchar> pixelIter, pixelEnd;
	if (buf4bitfile[index] == '0')
	{
		if (buf4bitfile[index + 1] == '0') //00
		{
			index += 2;
			for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
			{
				if (buf4bitfile[index] == '1')
				{
					if (buf4bitfile[index + 1] == '0')
						(*pixelIter) = 1;
					else
						(*pixelIter) = 255;
					index += 2;
				}
				else if (buf4bitfile[index + 1] == '1')
				{
					(*pixelIter) = 0;
					index += 2;
				}
				else
				{
					if (buf4bitfile[index + 2] == '0')
						(*pixelIter) = 2;
					if (buf4bitfile[index + 2] == '1')
						(*pixelIter) = 254;
					index += 3;
				}
			}
		}
		else if (buf4bitfile[index + 1] == '1') //01
		{
			index += 2;
			for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
			{
				(*pixelIter) = 0;
			}
		}
	}
	else if (buf4bitfile[index + 1] == '0') //10
	{
		index += 2;
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			if (buf4bitfile[index] == '1')
			{
				(*pixelIter) = 0;
				index += 1;
			}
			else if (buf4bitfile[index] == '0')
			{
				if (buf4bitfile[index + 1] == '0')
					(*pixelIter) = 1;
				else
					(*pixelIter) = 255;
				index += 2;
			}
		}
	}
	else if (buf4bitfile[index + 2] == '0') //110
	{
		index += 3;
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			if (buf4bitfile.substr(index, 3) == "000")
			{
				if (buf4bitfile.substr(index, 4) == "0000")
					(*pixelIter) = 4;
				else
					(*pixelIter) = 252;
				index += 4;
			}
			else
			{
				(*pixelIter) = bin2dec(buf4bitfile.substr(index, 2));
				index += 2;
				if ((buf4bitfile[index] == '1') && ((*pixelIter) != 0))
					(*pixelIter) = 255 - (*pixelIter) + 1;
				index += 1;
			}
		}
	}
	else //Golomb
	{
		index += 3;
		for (pixelIter = m.begin<uchar>(), pixelEnd = m.end<uchar>(); pixelIter != pixelEnd; pixelIter++)
		{
			int group = 0;
			while (buf4bitfile[index] == '1')
			{
				group++;
				index++;
			}
			if (group == 0)
			{
				(*pixelIter) = 0;
				index++;
			}
			else
			{
				(*pixelIter) = expGolomb_dec(buf4bitfile.substr(index - group, 2 * group + 2));
				index += group + 2;
			}
		}
	}
}

string MLL_FR::expGolomb_enc(uchar in)
{
	//ylog("%d", in);
	if (!in) return "0";
	int flag = 0;
	if (in > 127) //below zero
	{
		flag = 1;
		in = 255 - in + 1; //abs
	}
	int group, offset;
	group = (int)floor(log2(in + 1));
	offset = in + 1 - pow(2, group);
	string res = "";
	string tmp = dec2bin(offset);
	
	res = res + string(group, '1') + '0' + string((group - tmp.size()), '0') + tmp + (flag?'1':'0');
	//cout << " => " << res << endl;
	return res;
}

uchar MLL_FR::expGolomb_dec(string in)
{
	if (in == "0") return 0;
	uchar res;
	int group, offset, flag;
	group = (in.size() - 2)/2;
	flag = in[in.size() - 1] == '1' ? 1 : 0;
	offset = bin2dec(in.substr(group + 1,group));
	res = (uchar)pow(2, group) - 1 + offset;
	if (flag) //below zero
		res = 255 - res + 1;
	//cout << in << "b => ";
	//ylog("%d\n", res);
	return res;
}

string MLL_FR::dec2bin(uchar in)
{
	//ylog("%d", in);
	string tmp = "";
	if (!in) return "0";
	while (in > 0)
	{
		if (in % 2) tmp = tmp + '1';
		else tmp = tmp + '0';
		in = in / 2;
	}
	reverse(tmp.begin(), tmp.end());
	//cout << " = " << tmp << 'b' << endl;
	return tmp;
}

uchar MLL_FR::bin2dec(string in)
{
	uchar res = 0;
	int size = in.size();
	for (int i = 0; i < size; i++)
	{
		res = res << 1;
		res = res + (in[i] == '1');
	}
	//cout << in << "b = ";
	//ylog("%d\n", res);
	return res;
}

void MLL_FR::run_all()
{
	TBT_enc(1);
	IBP_enc();
	VLC_enc();
	VLC_dec();
	IBP_dec();
	TBT_dec(1);
}

void MLL_FR::encoding()
{
	TBT_enc(1);
	IBP_enc();
	VLC_enc();
}
