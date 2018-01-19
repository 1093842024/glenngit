#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  
#include <iomanip>
#include <iostream>  
using namespace cv;  
using namespace std;  

int main()
{
	// Read input image
	Mat image= cv::imread("../lena.jpg",0);
	if (!image.data)
		return 0; 
	namedWindow("Original Image");// Display the image
	imshow("Original Image",image);

	// Compute Sobel X derivative
	Mat sobelX;
	Sobel(image,sobelX,CV_8U,1,0,3,0.4,128);
	namedWindow("Sobel X Image");// Display the image
	imshow("Sobel X Image",sobelX);

	// Compute Sobel Y derivative
	Mat sobelY;
	Sobel(image,sobelY,CV_8U,0,1,3,0.4,128);
	namedWindow("Sobel Y Image"); // Display the image
	imshow("Sobel Y Image",sobelY);

	// Compute norm of Sobel
	Sobel(image,sobelX,CV_16S,1,0);
	Sobel(image,sobelY,CV_16S,0,1);
	Mat sobel;
	sobel= abs(sobelX)+abs(sobelY);//compute the L1 norm

	double sobmin, sobmax;
	minMaxLoc(sobel,&sobmin,&sobmax);
	cout << "sobel value range: " << sobmin << "  " << sobmax << endl;
	for (int i=0; i<12; i++) {
		for (int j=0; j<12; j++)
			cout << setw(5) << static_cast<int>(sobel.at<short>(i+135,j+362)) << " ";// Print window pixel values
		cout << endl;
	}
	cout << endl;
	cout << endl;
	cout << endl;
	// Conversion to 8-bit image
	// sobelImage = -alpha*sobel + 255
	Mat sobelImage;
	sobel.convertTo(sobelImage,CV_8U,-255./sobmax,255);
	namedWindow("Sobel Image");// Display the image
	imshow("Sobel Image",sobelImage);

	// Apply threshold to Sobel norm (low threshold value)
	Mat sobelThresholded;
	threshold(sobelImage, sobelThresholded, 225, 255, THRESH_BINARY);
	namedWindow("Binary Sobel Image (low)"); // Display the image
	imshow("Binary Sobel Image (low)",sobelThresholded);
	// Apply threshold to Sobel norm (high threshold value)
	threshold(sobelImage, sobelThresholded, 190, 255, THRESH_BINARY);
	namedWindow("Binary Sobel Image (high)");  // Display the image
	imshow("Binary Sobel Image (high)",sobelThresholded);
/*
	// Compute Laplacian 3x3
	Mat laplace;
	Laplacian(image,laplace,CV_8U,1,1,128);
	namedWindow("Laplacian Image");// Display the image
	imshow("Laplacian Image",laplace);
	// Print window pixel values
	for (int i=0; i<12; i++) {
		for (int j=0; j<12; j++)
			std::cout << std::setw(5) << static_cast<int>(laplace.at<uchar>(i+135,j+362))-128 << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	// Compute Laplacian 7x7
	Laplacian(image,laplace,CV_8U,7,0.01,128);
	namedWindow("Laplacian Image2"); // Display the image 
	imshow("Laplacian Image2",laplace);
	*/


	// Apply Canny algorithm
	Mat contours;
	Canny(image,contours,125,350);
	Mat contoursInv;
	threshold(contours,contoursInv,128,255,THRESH_BINARY_INV);
	namedWindow("Canny Contours");// Display the image of contours
	imshow("Canny Contours",contoursInv);
	//imshow("Canny Contours origin",contours);

	cv::waitKey();
	return 0;
}





