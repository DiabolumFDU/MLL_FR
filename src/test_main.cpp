#include "MLL_FR.hpp"

int main(int argc, char** argv)
{


#if 1
	Mat image, result, input8x8;
	image = imread("a.jpg", 0); // Read the file
	if (image.empty()) // Check for invalid input
	{
		FATAL;
		return -1;
	}
	input8x8 = image(Rect(500, 500, 8, 8));
	//cout << "image.size = " << image.size << endl;
	//cout << "input8x8 = " << endl << input8x8 << endl;
	MLL_FR fr_flow;



	fr_flow.set_input8x8(input8x8);
	cout << "input8x8 = " << endl << fr_flow.get_input8x8() << endl;
	fr_flow.run_all();
	
#if 0
	for (int i = 0; i < 32400; i++)
	{
		input8x8 = image(Rect((i % 240) * 8, (i / 240) * 8, 8, 8));
		fr_flow.set_input8x8(input8x8);
		fr_flow.encoding();
	}
#endif
	
	//fr_flow.dec2bin(128);
	//fr_flow.bin2dec("1111");


	//fr_flow.expGolomb_enc(0);
	//fr_flow.expGolomb_dec(fr_flow.expGolomb_enc(233));


//	imshow("result", result);
//	waitKey(0);
	imwrite("b.png", image);
#endif

#if 0
	char* buf;
	int bufSize;
	ifstream fin;
	ofstream fout;
	fout.open("EncodingResult.txt");
	fout << "100100";
	fout << "100110";
	fout << "110110";
	fout.close();

	fin.open("EncodingResult.txt");
	fin.seekg(0, ios::end);
	bufSize = fin.tellg();
	fin.seekg(0, ios::beg);
	cout << "bufSize = " << bufSize << endl;
	buf = (char*)malloc(sizeof(char)*bufSize);
	fin >> buf;
	cout << buf << endl;
	free(buf);
#endif //test file operation

#if 0
	/*
	if (argc != 2)
	{
		cout << " Usage: display_image ImageToLoadAndDisplay" << endl;
		return -1;
	}
	*/
	Mat image, result;
	image = imread("a.jpg", 1); // Read the file
	if (image.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}
	Canny(image, result, 50, 150);

	namedWindow("Canny Result", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Canny Result", result); // Show our image inside it.
	waitKey(0); // Wait for a keystroke in the window
	imwrite("b.png", image);
#endif //first test


/*
	Mat myImg(400, 400, CV_8UC3);
#if 0
	for (int i = 0; i < myImg.rows; i++)
	{
		for (int j = 0; j < myImg.cols; j++)
		{
			Vec3b pixel;
			pixel[0] = i % 255;
			pixel[1] = j % 255;
			pixel[2] = 0;
			myImg.at<Vec3b>(i, j) = pixel;
		}
	}
#endif //test Mat.at()

#if 0
	MatIterator_<Vec3b> pixelIter, pixelEnd;
	for (pixelIter = myImg.begin<Vec3b>(), pixelEnd = myImg.end<Vec3b>(); pixelIter != pixelEnd; pixelIter++)
	{
		(*pixelIter)[0] = rand() % 255;
		(*pixelIter)[1] = rand() % 255;
		(*pixelIter)[2] = rand() % 255;
	}
#endif //test MatIterator

#if 0
	for (int i = 0; i < myImg.rows; i++)
	{
		Vec3b* pixelPtr = myImg.ptr<Vec3b>(i);
		for (int j = 0; j < myImg.cols; j++)
		{
			pixelPtr[j][0] = i % 255;
			pixelPtr[j][1] = j % 255;
			pixelPtr[j][2] = 0;
		}
	}
#endif //test Mat.ptr

#if 0
	Mat_<Vec3b> M = (Mat_<Vec3b> &) myImg;
	for (int i = 0; i < M.rows; i++)
	{
		for (int j = 0; j < M.cols; j++)
		{
			M(i,j)[0] = i % 255;
			M(i,j)[1] = j % 255;
			M(i,j)[2] = 0;
		}
	}
#endif //test Mat_<type>

	//cout << "M = " << endl << ' ' << M << endl;
	//namedWindow("test", WINDOW_AUTOSIZE);
	imshow("test", myImg);
	waitKey(0);
	*/



	return 0;
}