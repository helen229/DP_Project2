#include "sc.h"
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

void gradientToseam(Mat& Image, vector<vector<int>>& seamMat, vector<vector<int>>& visitedPath)
{
	int channels = Image.channels();
	int nRows = Image.rows;
	int nCols = Image.cols* channels;
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
		vector<int> pixelList;
		ptr = Image.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			pixelList.push_back(ptr[j]);
		}
		seamMat.push_back(pixelList);
		visitedPath.push_back(pixelList);
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

		//if (iimage.cols>new_width) {
		//	reduce_vertical_seam_trivial(iimage, oimage);
		//	iimage = oimage.clone();
		//}
	}

	out_image = oimage.clone();
	return true;
}


// horizontl trivial seam is a seam through the center of the image
bool reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image) {

	// retrieve the dimensions of the new image
	int rows = in_image.rows - 1;
	int cols = in_image.cols;
	vector<vector<int>> seamMat;
	vector<vector<int>> visitedPath;
	Mat gradientMat;
	Mat input;
	input = in_image.clone();
	calculateGradient(input, gradientMat);
	gradientToseam(gradientMat, seamMat, visitedPath);
	//std::cout << "M = " << endl << " " << gradientMat << endl << endl;
	for (int col = 1; col<in_image.cols; col++) {
		for (int row = 0; row<in_image.rows; row++) {
			if (row == 0) {
				if (seamMat[row + 1][col - 1]> seamMat[row][col - 1]) {
					seamMat[row][col] += seamMat[row][col - 1]; 
					visitedPath[row][col] = row;
				}
				else {
					seamMat[row][col] += seamMat[row+1][col - 1];
					visitedPath[row][col] = row+1;
				}
			}
			else if (row == in_image.rows - 1) {
				if (seamMat[row - 1][col - 1]> seamMat[row][col - 1]) {
					seamMat[row][col] += seamMat[row][col - 1];
					visitedPath[row][col] = row;
				}
				else {
					seamMat[row][col] += seamMat[row - 1][col - 1];
					visitedPath[row][col] = row-1;
				}
			}
			else {
				int temp;
				temp = min(seamMat[row + 1][col - 1], min(seamMat[row][col - 1], seamMat[row - 1][col - 1]));
				seamMat[row][col] += temp;
				if (temp == seamMat[row + 1][col - 1]) {
					visitedPath[row][col] = row + 1;
				}
				else if (temp == seamMat[row][col - 1]) {
					visitedPath[row][col] = row;
				}
				else {
					visitedPath[row][col] = row - 1;
				}
			}
		}
	}

	// go through the last col to find the minimum seam val
	int miniVal = INT_MAX;
	int minRow = -1;
	for (int row = 0; row < in_image.rows; row++) {
		if (miniVal > seamMat[row][in_image.cols - 1]) {
			miniVal = seamMat[row][in_image.cols - 1];
			minRow = row;
		}
	}

	//get the min path from the visitedPath
	//for (int i = 0; i < in_image.row; ++i) {

	//}


	// create an image slighly smaller
	out_image = Mat(rows, cols, CV_8UC3);

	//populate the image
	int cutRow= minRow;
	for (int j = cols-1; j>1;j--) {

	    for (int i = 0; i < cutRow; i++) {
			Vec3b pixel = in_image.at<Vec3b>(i, j);
			out_image.at<Vec3b>(i, j) = pixel;
		}

		for (int i = cutRow; i < rows; i++) {
			Vec3b pixel = in_image.at<Vec3b>(i+1, j);
			out_image.at<Vec3b>(i, j) = pixel;
		}
		cutRow = visitedPath[cutRow][j];
	}

	return true;
}

// vertical trivial seam is a seam through the center of the image
//bool reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image) {
//	// retrieve the dimensions of the new image
//	int rows = in_image.rows;
//	int cols = in_image.cols - 1;
//
//	// create an image slighly smaller
//	out_image = Mat(rows, cols, CV_8UC3);
//
//	//populate the image
//	int middle = in_image.cols / 2;
//
//	for (int i = 0; i<rows; ++i)
//		for (int j = 0; j <= middle; ++j) {
//			Vec3b pixel = in_image.at<Vec3b>(i, j);
//
//			/* at operator is r/w
//			pixel[0] --> red
//			pixel[1] --> green
//			pixel[2] --> blue
//			*/
//
//
//			out_image.at<Vec3b>(i, j) = pixel;
//		}
//
//	for (int i = 0; i<rows; ++i)
//		for (int j = middle + 1; j<cols; ++j) {
//			Vec3b pixel = in_image.at<Vec3b>(i, j + 1);
//
//			/* at operator is r/w
//			pixel[0] --> red
//			pixel[1] --> green
//			pixel[2] --> blue
//			*/
//
//
//			out_image.at<Vec3b>(i, j) = pixel;
//		}
//
//	return true;
//}
