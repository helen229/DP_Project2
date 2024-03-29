 //DP_Project2.cpp : Defines the entry point for the console application.

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream> 
#include <algorithm>
#include <map>
#include <iterator>
#include <opencv2/opencv.hpp>
#include "sc.h"
#include <iostream>
#include <chrono>
//#include "stdafx.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	if (argc != 5) {
		cout << "Usage: ../sc input_image new_width new_height output_image" << endl;
		return -1;
	}

	// Load the input image
	// the image should be a 3 channel image by default but we will double check that in teh seam_carving
	Mat in_image;
	in_image = imread(argv[1]/*, CV_LOAD_IMAGE_COLOR*/);

	if (!in_image.data)
	{
		cout << "Could not load input image!!!" << endl;
		return -1;
	}

	if (in_image.channels() != 3) {
		cout << "Image does not have 3 channels!!! " << in_image.depth() << endl;
		return -1;
	}

	// get the new dimensions from the argument list
	int new_width = atoi(argv[2]);
	int new_height = atoi(argv[3]);

	// the output image
	Mat out_image;

	if (!seam_carving(in_image, new_width, new_height, out_image)) {
		return -1;
	}

	// write it on disk
	imwrite(argv[4], out_image);

	// also display them both

	namedWindow("Original image", WINDOW_AUTOSIZE);
	namedWindow("Seam Carved Image", WINDOW_AUTOSIZE);
	imshow("Original image", in_image);
	imshow("Seam Carved Image", out_image);
	waitKey(0);
	/*for (int i = 0; i < 3000; ++i)
	{
		vector<int> pixelList;
		for (int j = 0; j < 2000; ++j)
		{
			pixelList.push_back(j);
			seamMatOne.push_back(j);
		}
		seamMat.push_back(pixelList);
		seamMatOne.push_back(i);
	}*/

	//chrono::steady_clock sc;
	//auto start = sc.now();     // start timer
	//for (int i = 0; i < 3000; ++i)
	//{
	//	for (int j = 0; j < 2000; ++j)
	//	{
	//		int temp = seamMatOne[i * 2000 + j];
	//	}
	//}

	//auto end = sc.now();       // end timer (starting & ending is done by measuring the time at the moment the process started & ended respectively)
	//auto time_span = static_cast<chrono::duration<double>>(end - start);   // measure time span between start & end
	//cout << "Operation gradientToseam took: " << time_span.count() << " seconds !!!";

	//chrono::steady_clock sc1;
	//auto start1 = sc1.now();     // start timer

	//for (int i = 0; i < 3000; ++i)
	//{
	//	for (int j = 0; j < 2000; ++j)
	//	{
	//		int temp = seamMat[i][j];
	//	}
	//}			
	//
	//auto end1 = sc1.now();       // end timer (starting & ending is done by measuring the time at the moment the process started & ended respectively)
	//auto time_span1 = static_cast<chrono::duration<double>>(end1 - start1);   // measure time span between start & end
	//cout << "Operation generate the dp mat took: " << time_span1.count() << " seconds !!!";

	//system("pause");

	return 0;
}

//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include <stdlib.h>
//#include <stdio.h>
//
//using namespace cv;
//using namespace std;
//
///** @function main */
//int main(int argc, char** argv)
//{
//
//	Mat src, src_gray;
//	Mat grad;
//	String window_name = "Sobel Demo - Simple Edge Detector";
//	int scale = 1;
//	int delta = 0;
//	int ddepth = CV_16S;
//
//	int c;
//
//	/// Load an image
//	src = imread(argv[1]);
//
//	if (!src.data)
//	{
//		return -1;
//	}
//	int count = 1000;
//	while (count) {
//		count--;
//		GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
//
//		/// Convert it to gray
//		cvtColor(src, src_gray, CV_BGR2GRAY);
//
//		/// Create window
//		namedWindow(window_name, CV_WINDOW_AUTOSIZE);
//
//		/// Generate grad_x and grad_y
//		Mat grad_x, grad_y;
//		Mat abs_grad_x, abs_grad_y;
//
//		/// Gradient X
//		//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
//		Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
//		convertScaleAbs(grad_x, abs_grad_x);
//
//		/// Gradient Y
//		//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
//		Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
//		convertScaleAbs(grad_y, abs_grad_y);
//
//		/// Total Gradient (approximate)
//		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
//
//	}
//
//	imshow(window_name, grad);
//	//std::cout << "M = " << endl << " " << grad << endl << endl;
//
//	waitKey(0);
//
//	return 0;
//}
//
