#include "sc.h"
#include <iostream>
#include <chrono>
using namespace cv;
using namespace std;

bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image) {

	// some sanity checks
	// Check 1 -> new_width <= in_image.cols
	if (new_width>in_image.cols) {
		cout << "Invalid request!!! new_width has to be smaller than the current size!" << endl;
		return false;
	}
	if (new_height>in_image.rows) {
		cout << "Invalid request!!! ne_height has to be smaller than the current size!" << endl;
		return false;
	}

	if (new_width <= 0) {
		cout << "Invalid request!!! new_width has to be positive!" << endl;
		return false;

	}

	if (new_height <= 0) {
		cout << "Invalid request!!! new_height has to be positive!" << endl;
		return false;

	}


	return seam_carving_trivial(in_image, new_width, new_height, out_image);
}

void calculateGradient(Mat& input, Mat& gradient) {

	Mat input_gray;
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;

	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;

	// to reduce the noise(kernel size = 3)
	GaussianBlur(input, input, Size(3, 3), 0, 0, BORDER_DEFAULT);

	// Convert the input image to gray
	cvtColor(input, input_gray, CV_BGR2GRAY);
	//gradient = input_gray;
	///// Create window
	//namedWindow("gd", CV_WINDOW_AUTOSIZE);

	/// Horizontal changes derivatives
	Sobel(input_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);

	/// Vertical changes derivatives(depth)
	Sobel(input_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);

	/// Total Gradient (approximate)
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, gradient);

	//imshow("imput", input);
	//std::cout << "abs_grad_x = " << std::endl << " " << gradient << std::endl << std::endl;
}

void gradientToseam(Mat& Image, vector<int>& seamMat)
{
	int nRows = Image.rows;
	int nCols = Image.cols;
	//image store in a continue memory
	//if (Image.isContinuous())
	//{
	//	nCols *= nRows;
	//	nRows = 1;
	//}
	int i, j;
	//using ptr can access mat much more faster
	uchar* ptr;
	for (i = 0; i < nRows; ++i)
	{
		ptr = Image.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			seamMat[i*Image.cols+j]=(ptr[j]);
		}
	}
}

// seam carves by removing trivial seams
bool seam_carving_trivial(Mat& in_image, int new_width, int new_height, Mat& out_image) {

	Mat iimage = in_image.clone();
	Mat oimage = in_image.clone();

	while (iimage.rows != new_height || iimage.cols != new_width) {
		// horizontal seam if needed
		if (iimage.rows>new_height) {
			reduce_horizontal_seam_trivial(iimage, oimage);
			iimage = oimage.clone();
		}

		if (iimage.cols>new_width) {
			reduce_vertical_seam_trivial(iimage, oimage);
			iimage = oimage.clone();
		}
	}

	out_image = oimage.clone();
	return true;
}


// horizontl trivial seam is a seam through the center of the image
bool reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image) {

	// retrieve the dimensions of the new image
	int rows = in_image.rows - 1;
	int cols = in_image.cols;
	vector<int> seamMat(in_image.rows*in_image.cols);
	vector<int> visitedPath(in_image.cols);

	Mat gradientMat;
	Mat input;
	
	input = in_image.clone();
	calculateGradient(input, gradientMat);
	gradientToseam(gradientMat, seamMat);
	//std::cout << "M = " << endl << " " << gradientMat << endl << endl;
	for (int col = 1; col<in_image.cols; col++) {
		for (int row = 0; row<in_image.rows; row++) {
			if (row == 0) {
				seamMat[(row)*in_image.cols + col] += min(seamMat[(row)*in_image.cols+col - 1], seamMat[(row + 1)*in_image.cols+col - 1]);
			}
			else if (row == in_image.rows - 1) {
				seamMat[(row)*in_image.cols + col] += min(seamMat[(row)*in_image.cols + col - 1], seamMat[(row - 1)*in_image.cols + col - 1]);
			}
			else {
				seamMat[(row)*in_image.cols + col] += min(seamMat[(row-1)*in_image.cols + col - 1], min(seamMat[(row)*in_image.cols + col - 1], seamMat[(row + 1)*in_image.cols + col - 1]));
			}
		}
	}

	// go through the last col to find the minimum seam val
	int miniVal = INT_MAX;
	int minRow = -1;
	for (int row = 0; row < in_image.rows; row++) {
		if (miniVal > seamMat[row*in_image.cols+in_image.cols - 1]) {
			miniVal = seamMat[row*in_image.cols + in_image.cols - 1];
			minRow = row;
		}
	}

	//get the visitedPath,from the min Seam
	for (int i = in_image.cols-1; i >0; i--) {
		visitedPath[i] = minRow;
		if (minRow == 0) {
			if (seamMat[minRow*in_image.cols + i-1]> seamMat[(minRow + 1)*in_image.cols + i-1]) {
				minRow = minRow + 1;
			}
			else {
				minRow = minRow ;
			}
		}
		else if (minRow == in_image.rows - 1) {
			if (seamMat[minRow*in_image.cols + i - 1]> seamMat[(minRow - 1)*in_image.cols + i - 1]) {
				minRow = minRow - 1;
			}
			else {
				minRow = minRow;
			}
		}
		else {
			int temp = min(seamMat[(minRow - 1)*in_image.cols + i - 1], min(seamMat[minRow*in_image.cols + i - 1], seamMat[(minRow + 1)*in_image.cols + i - 1]));
			if (temp == seamMat[(minRow - 1)*in_image.cols + i - 1]) {
				minRow = minRow - 1;
			}
			else if (temp == seamMat[(minRow + 1)*in_image.cols + i - 1]) {
				minRow = minRow + 1;
			}
			else {
				minRow = minRow;
			}

		}

	}


	// create an image slighly smaller
	out_image = Mat(rows, cols, CV_8UC3);

	//populate the image
	for (int j = 0; j < rows; j++) {
		Vec3b pixel = in_image.at<Vec3b>(j, 0);
		out_image.at<Vec3b>(j, 0) = pixel;
	}

	for (int j = cols-1; j>0; j--) {
		int cutRow = visitedPath[j];
	    for (int i = 0; i < cutRow; i++) {
			Vec3b pixel = in_image.at<Vec3b>(i, j);
			out_image.at<Vec3b>(i, j) = pixel;
		}

		for (int i = cutRow; i < rows; i++) {
			Vec3b pixel = in_image.at<Vec3b>(i+1, j);
			out_image.at<Vec3b>(i, j) = pixel;
		}

	}

	return true;
}

//vertical trivial seam is a seam through the center of the image
bool reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image) {
	// retrieve the dimensions of the new image
	int rows = in_image.rows;
	int cols = in_image.cols - 1;

	// create an image slighly smaller
	out_image = Mat(rows, cols, CV_8UC3);

	vector<int> seamMat(in_image.rows*in_image.cols);
	vector<int> visitedPath(in_image.rows*in_image.cols,-1);
	Mat gradientMat;
	Mat input;
	input = in_image.clone();
	calculateGradient(input, gradientMat);

	//chrono::steady_clock sc;
	//auto start = sc.now();     // start timer

	gradientToseam(gradientMat, seamMat);

	//auto end = sc.now();       // end timer (starting & ending is done by measuring the time at the moment the process started & ended respectively)
	//auto time_span = static_cast<chrono::duration<double>>(end - start);   // measure time span between start & end
	//cout << "Operation gradientToseam took: " << time_span.count() << " seconds !!!";


	//chrono::steady_clock sc1;
	//auto start1 = sc1.now();     // start timer

    //generate the dp mat
	for (int row = 1; row <in_image.rows; row++) {
		if (seamMat[(row - 1)*in_image.cols + 1]> seamMat[(row - 1)*in_image.cols]) {
			seamMat[row*in_image.cols] += seamMat[(row - 1)*in_image.cols];
			visitedPath[row*in_image.cols] = (row - 1)*in_image.cols;
		}
		else {
			seamMat[row*in_image.cols] += seamMat[(row - 1)*in_image.cols + 1];
			visitedPath[row*in_image.cols] = (row - 1)*in_image.cols + 1;
		}
		if (seamMat[(row - 1)*in_image.cols + in_image.cols - 1 - 1]> seamMat[(row - 1)*in_image.cols + in_image.cols - 1]) {
			seamMat[row*in_image.cols+in_image.cols - 1] += seamMat[(row - 1)*in_image.cols + in_image.cols - 1];
			visitedPath[row*in_image.cols+in_image.cols - 1] = (row - 1)*in_image.cols + in_image.cols - 1;
		}
		else {
			seamMat[row*in_image.cols+in_image.cols - 1] += seamMat[(row - 1)*in_image.cols + in_image.cols - 1 - 1];
			visitedPath[row*in_image.cols+in_image.cols - 1] = (row - 1)*in_image.cols + in_image.cols - 1 - 1;
		}
		for (int col = 1; col<in_image.cols-1; col++) {
			int temp = min(seamMat[(row - 1)*in_image.cols + col - 1], min(seamMat[(row - 1)*in_image.cols + col], seamMat[(row - 1)*in_image.cols + col + 1]));
			//int temp = 1;
			seamMat[row*in_image.cols + col] += temp;
			if (temp == seamMat[(row - 1)*in_image.cols + col + 1]) {
				visitedPath[row*in_image.cols + col] = (row - 1)*in_image.cols + col + 1;
			}
			else if (temp == seamMat[(row - 1)*in_image.cols + col - 1]) {
				visitedPath[row*in_image.cols + col] = (row - 1)*in_image.cols + col - 1;
			}
			else {
				visitedPath[row*in_image.cols + col] = (row - 1)*in_image.cols + col;
			}
			//int res=INT_MAX;
			//for (int i = -1; i < 2; i++) {
			//	if (res > seamMat[row - 1][col + i]) {
			//		res = seamMat[row - 1][col + i];
			//		visitedPath[row][col] = col+i;
			//	}
			//}
		}
	}

	//auto end1 = sc1.now();       // end timer (starting & ending is done by measuring the time at the moment the process started & ended respectively)
	//auto time_span1 = static_cast<chrono::duration<double>>(end1 - start1);   // measure time span between start & end
	//cout << "Operation generate the dp mat took: " << time_span1.count() << " seconds !!!";
	//
	// go through the last col to find the minimum seam val
	int miniVal = INT_MAX;
	int minCol = -1;
	for (int col = 0; col < in_image.cols; col++) {
		if (miniVal > seamMat[(in_image.rows-1) * in_image.cols + col]) {
			miniVal = seamMat[(in_image.rows-1) * in_image.cols + col];
			minCol = (in_image.rows - 1) * in_image.cols + col;
		}
	}

	//get the min path from the visitedPath
	//for (int i = 0; i < in_image.row; ++i) {
	//}
	// create an image slighly smaller
	out_image = Mat(rows, cols, CV_8UC3);

	//populate the image
	int cutCol = minCol- (rows - 1)*in_image.cols;

	for (int j = 0; j < cols; j++) {
		Vec3b pixel = in_image.at<Vec3b>(0, j);
		out_image.at<Vec3b>(0, j) = pixel;
	}

	for (int i = rows - 1; i>0; i--) {

		for (int j = 0; j < cutCol; j++) {
			Vec3b pixel = in_image.at<Vec3b>(i, j);
			out_image.at<Vec3b>(i, j) = pixel;
		}

		for (int j = cutCol; j < cols; j++) {
			Vec3b pixel = in_image.at<Vec3b>(i, j+1);
			out_image.at<Vec3b>(i, j) = pixel;
		}
		cutCol = visitedPath[i*in_image.cols+cutCol]- (i-1)*in_image.cols;
	}

	return true;
}
//单独水平左边有花点