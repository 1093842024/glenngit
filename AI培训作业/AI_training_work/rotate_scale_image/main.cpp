#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  
#include <iomanip>
#include <iostream>  
#include "cv.h"    
#include "highgui.h"    
#include "math.h" 
using namespace cv;  
using namespace std;  


// clockwise 为true则顺时针旋转，否则为逆时针旋转    
IplImage* rotateImage(IplImage* src, int angle, bool clockwise)    
{    
	angle = abs(angle) % 180;    
	if (angle > 90)    
	{    
		angle = 90 - (angle % 90);    
	}    
	IplImage* dst = NULL;    
	int width =    
		(double)(src->height * sin(angle * CV_PI / 180.0)) +    
		(double)(src->width * cos(angle * CV_PI / 180.0 )) + 1;    
	int height =    
		(double)(src->height * cos(angle * CV_PI / 180.0)) +    
		(double)(src->width * sin(angle * CV_PI / 180.0 )) + 1;    
	int tempLength = sqrt((double)src->width * src->width + src->height * src->height) + 10;    
	int tempX = (tempLength + 1) / 2 - src->width / 2;    
	int tempY = (tempLength + 1) / 2 - src->height / 2;    
	int flag = -1;    

	dst = cvCreateImage(cvSize(width, height), src->depth, src->nChannels);    
	cvZero(dst);    
	IplImage* temp = cvCreateImage(cvSize(tempLength, tempLength), src->depth, src->nChannels);    
	cvZero(temp);    

	cvSetImageROI(temp, cvRect(tempX, tempY, src->width, src->height));    
	cvCopy(src, temp, NULL);    
	cvResetImageROI(temp);    

	if (clockwise)    
		flag = 1;    

	float m[6];    
	int w = temp->width;    
	int h = temp->height;    
	m[0] = (float) cos(flag * angle * CV_PI / 180.);    
	m[1] = (float) sin(flag * angle * CV_PI / 180.);    
	m[3] = -m[1];    
	m[4] = m[0];    
	// 将旋转中心移至图像中间    
	m[2] = w * 0.5f;    
	m[5] = h * 0.5f;    
	//    
	CvMat M = cvMat(2, 3, CV_32F, m);    
	cvGetQuadrangleSubPix(temp, dst, &M);    
	cvReleaseImage(&temp);    
	return dst;    
}    



int main(int argc, char **argv)    
{   
	double scale=0.5; 
	Mat image =imread("../lena.jpg");
	imshow("origin",image);
	Size dsize = Size(image.cols*scale,image.rows*scale);
	Mat image2 = Mat(dsize,CV_32S);
	resize(image, image2,dsize);
	imshow("scaled lena",image2);

	IplImage *src = 0;    
	IplImage *dst = 0;    
	// 旋转角度    
	int angle = 45;    

	src = cvLoadImage("../lena.jpg",CV_LOAD_IMAGE_COLOR);    
	cvNamedWindow("src", 1);    
	cvShowImage("src", src);    

	dst = rotateImage(src, angle, false);  
	Mat dstmat=Mat(dst);
	imshow("dst", dstmat);    
	cvWaitKey(0);    

	cvReleaseImage(&src);    
	cvReleaseImage(&dst);  

    cv::waitKey();
	return 0;    
}    










