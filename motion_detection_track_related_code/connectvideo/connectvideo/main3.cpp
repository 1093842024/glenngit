#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
using namespace std;
using namespace cv;

#define UPSIDEDOWN
/*
int main(int argc, char *argv[])  
{  
 
 VideoCapture cap("20.avi");
	if (!cap.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}

	Mat frame;
	cap >> frame;

	double fps=cap.get(CV_CAP_PROP_FPS);
	cv::Size size=frame.size();
	cv::VideoWriter writevideo("20.1.avi",CV_FOURCC('M','J','P','G'), fps, size);

     Mat frame1;
     frame1.create( frame.size(), frame.type());
     Mat map_x;
     Mat map_y;
     map_x.create( frame.size(), CV_32FC1);
     map_y.create( frame.size(), CV_32FC1);
     for( int i = 0; i < frame.rows; ++i)
    {
	   for( int j = 0; j < frame.cols; ++j)
	   {
		   map_x.at<float>(i, j) = (float) (frame.cols - j) ;
		   map_y.at<float>(i, j) = (float) (frame.rows - i) ;
	   }
   }
   remap(frame, frame1, map_x, map_y, CV_INTER_LINEAR);
 
   cv::namedWindow("original image");
   cv::imshow("original image", frame1);
   

 //读取和显示  
 while(1)  
 {  
	 cap>>frame;
	 if (frame.empty()) break;
	 // stop the program if no more images
	
  for( int i = 0; i < frame.rows; ++i)
  {
	  for( int j = 0; j < frame.cols; ++j)
	  {
		  map_x.at<float>(i, j) = (float) (frame.cols - j) ;
		  map_y.at<float>(i, j) = (float) (frame.rows - i) ;
	  }
  }
  remap(frame, frame1, map_x, map_y, CV_INTER_LINEAR);

  imshow("tracker", frame1);
  printf("changing!\n");

  writevideo<<frame1;

  if (waitKey(1) == 27)   //不加这句，无法显示出图像，因为图像还未加载就进行了下一句
  break;

 } 

 return 0;  
} 
*/