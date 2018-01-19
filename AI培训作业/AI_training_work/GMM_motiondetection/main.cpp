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
// foreground binary image   前景二值图像
cv::Mat foreground;

cv::namedWindow("Extracted Foreground");

// The Mixture of Gaussian object  使用默认参数的混合高斯对象
// used with all default parameters
cv::BackgroundSubtractorMOG mog;

bool stop(false);
// for all frames in video
while (!stop) {

	// read next frame if any
	if (!capture.read(frame))
		break;

	// update the background      更新背景并返回前景（额外的参数是学习率）
	// and return the foreground
	mog(frame,foreground,0.01);

	// Complement the image       对前景图像取反
	cv::threshold(foreground,foreground,128,255,cv::THRESH_BINARY_INV);

	// show foreground           显示前景
	cv::imshow("Extracted Foreground",foreground);

	// introduce a delay
	// or press key to stop    引入延迟，也可以通过按键停止
	if (cv::waitKey(10)>=0)
		stop= true;
}

cv::waitKey();

return 0;
}




