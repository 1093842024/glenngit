#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  
#include <iomanip>
#include <iostream>  
#include "cv.h"    
#include "highgui.h"    
#include "math.h" 
#include <opencv2/video/background_segm.hpp>
using namespace cv;  
using namespace std;  


int main()
{

// Open the video file
cv::VideoCapture capture("../bike.avi");
// check if video successfully opened
if (!capture.isOpened())
	return 0;

// current video frame
cv::Mat frame; 
// foreground binary image   ǰ����ֵͼ��
cv::Mat foreground;

cv::namedWindow("Extracted Foreground");

// The Mixture of Gaussian object  ʹ��Ĭ�ϲ����Ļ�ϸ�˹����
// used with all default parameters
cv::BackgroundSubtractorMOG mog;

bool stop(false);
// for all frames in video
while (!stop) {

	// read next frame if any
	if (!capture.read(frame))
		break;

	// update the background      ���±���������ǰ��������Ĳ�����ѧϰ�ʣ�
	// and return the foreground
	mog(frame,foreground,0.01);

	// Complement the image       ��ǰ��ͼ��ȡ��
	cv::threshold(foreground,foreground,128,255,cv::THRESH_BINARY_INV);

	// show foreground           ��ʾǰ��
	cv::imshow("Extracted Foreground",foreground);

	// introduce a delay
	// or press key to stop    �����ӳ٣�Ҳ����ͨ������ֹͣ
	if (cv::waitKey(10)>=0)
		stop= true;
}

cv::waitKey();

return 0;
}




