#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  
#include <iomanip>
#include <iostream>  
using namespace cv;  
using namespace std;  

#define PI 3.1415926

int main()
{
	// Read input image
	Mat image= cv::imread("../lena.jpg",0);
	if (!image.data)
		return 0; 

	// Apply Canny algorithm
	Mat contours;
	Canny(image,contours,125,350);
	Mat contoursInv;
	threshold(contours,contoursInv,128,255,THRESH_BINARY_INV);
	namedWindow("Canny Contours");// Display the image of contours
	imshow("Canny Contours",contoursInv);

	// Hough tranform for line detection     »ô·ò±ä»»
	vector<cv::Vec2f> lines;
	HoughLines(contours,lines,1,PI/180,120);
	Mat result(contours.rows,contours.cols,CV_8U,cv::Scalar(255));// Draw the lines
	image.copyTo(result);
	cout << "Lines detected: " << lines.size() << endl;
	vector<cv::Vec2f>::const_iterator it= lines.begin();
	while (it!=lines.end()) {
		float rho= (*it)[0];   // first element is distance rho
		float theta= (*it)[1]; // second element is angle theta

		if (theta < PI/4. || theta > 3.*PI/4.) { // ~vertical line
			// point of intersection of the line with first row
			Point pt1(rho/cos(theta),0);        
			// point of intersection of the line with last row
			Point pt2((rho-result.rows*sin(theta))/cos(theta),result.rows);
			// draw a white line
			line( result, pt1, pt2, cv::Scalar(255), 1); 
		} else { // ~horizontal line
			// point of intersection of the line with first column
			Point pt1(0,rho/sin(theta));        
			// point of intersection of the line with last column
			Point pt2(result.cols,(rho-result.cols*cos(theta))/sin(theta));
			// draw a white line
			line( result, pt1, pt2, cv::Scalar(255), 1); 
		}
		//std::cout << "line: (" << rho << "," << theta << ")\n"; 
		++it;
	}
	cv::namedWindow("Detected Lines with Hough");  // Display the detected line image
	cv::imshow("Detected Lines with Hough",result);

	cv::waitKey();
	return 0;
}