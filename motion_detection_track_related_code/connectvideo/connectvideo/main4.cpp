#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
using namespace std;
using namespace cv;

#define CUTVIDEO
/*
int main(int argc, char *argv[])  
{  

	VideoCapture cap("../20.avi");
	if (!cap.isOpened()){
		printf("video open failed!"); 
		cv::waitKey(0);
		return -1;
	}
	Mat frameorigin;
	cap>>frameorigin;

	cv::namedWindow("original image");
	cv::imshow("original image", frameorigin);
	waitKey(0);   //不加这句，无法显示出图像，因为图像还未加载就进行了下一句

	float scale=1;
	Mat frame(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
	cv::resize(frameorigin,frame,frame.size());


	double fps=cap.get(CV_CAP_PROP_FPS);
	cv::Size size=frame.size();
	int count=cap.get(CV_CAP_PROP_FRAME_COUNT);
	cv::VideoWriter writevideo("../20.1.avi",CV_FOURCC('M','J','P','G'), fps, size);

	int i=1;

	//读取和显示  
	while(1)  
	{  
		cap>>frameorigin;
		i++;
		if (frameorigin.empty()) break;
		// stop the program if no more images
		if (frameorigin.rows == 0 || frameorigin.cols == 0)
			break;

		Mat frame(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
		cv::resize(frameorigin,frame,frame.size());

		imshow("tracker", frame);
		printf("changing frame %d!\n",i);

		if(i>300) writevideo<<frame;


		if (waitKey(1) == 27)   //不加这句，无法显示出图像，因为图像还未加载就进行了下一句
			break;

	} 



	return 0;  
}
*/