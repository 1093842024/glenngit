#include <stdlib.h>

#include "kcftracker.hpp"
#include "basetype.h"
#include <iostream>
#include <string>
#include "trackerinterface.h"
#include "opencv2/opencv.hpp"

#define INTERFACE

using namespace std;
using namespace cv;
cv::Rect box;
bool drawing_box = false;
bool selected = false;
void create_mouse_callback(int event, int x, int y, int flag, void* param)     //新增部分，利用鼠标画框来进行初始标定
{
	cv::Mat *image = (cv::Mat*) param;
	switch (event){
	case CV_EVENT_MOUSEMOVE:
		if (drawing_box){
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;

	case CV_EVENT_LBUTTONDOWN:
		drawing_box = true;
		box = cv::Rect(x, y, 0, 0);
		break;

	case CV_EVENT_LBUTTONUP:
		drawing_box = false;
		if (box.width < 0){
			box.x += box.width;
			box.width *= -1;
		}
		if (box.height < 0){
			box.y += box.height;
			box.height *= -1;
		}
		cv::rectangle(*image, box, cv::Scalar(0), 2);
		selected = true;
		break;
	}

}

int main()
{
#ifdef INTERFACE
      TrackHandle tracker = CreateTracker(64,2.2,1.07);
#else
	  KCFTracker tracker(64,2.2,1.07);                        //创建KCF跟踪检测器 KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
#endif	
	
	cv::Mat frame;
	VideoCapture cap("../5.mp4");
	if (!cap.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}
	cap >> frame;

	cv::namedWindow("original image");
	Mat temp_img = frame.clone();
	
	cv::setMouseCallback("original image", create_mouse_callback, (void*)&temp_img);

	if(frame.empty())
	{
	 printf("can not load image");
	 cv::waitKey(0);
	 return -1;
	}

	cv::imshow("original image", frame);
	while (selected == false)
	{
		cv::Mat temp;
		temp_img.copyTo(temp);

		if (drawing_box)
			cv::rectangle(temp, box, cv::Scalar(0), 2);

		cv::imshow("original image", temp);
		if (cv::waitKey(15) == 27)
			break;
	}
	cv::setMouseCallback("original image", NULL, NULL);
	waitKey(0);
	if (box.width == 0 || box.height == 0)
		return 0;
	//tracker.init(frame, box);
	Mat frame_gray;
	cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
	CMat cgray = CMat(CSize(frame_gray.cols, frame_gray.rows), MAT_8UC1, frame_gray.data);

#ifdef INTERFACE
	BBox cbox;
	cbox.x=box.x;
	cbox.y=box.y;
	cbox.w=box.width;
	cbox.h=box.height;
	InitTracker(tracker,cbox,cgray.data,cgray.cols,cgray.rows,cgray.step);
#else
	CRect cbox(box.x, box.y, box.width, box.height);
	//CRect cbox(311,78,203,139);
	tracker.init(cbox, cgray);
#endif	

	printf("Start the tracking process\n");

	int img_idx = 2;
	//BBox_c bb;
	for (;;) {
		
		cap >> frame;
		if (frame.empty()) break;

		// stop the program if no more images
		if (frame.rows == 0 || frame.cols == 0)
			break;

		// update the tracking result
		//! [update]//tracker->update(frame, box);
		Mat frame_gray;
		cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
		CMat cgray = CMat(CSize(frame_gray.cols, frame_gray.rows), MAT_8UC1, frame_gray.data);
#ifdef INTERFACE	
		double time_profile_counter = (double)cvGetTickCount();
		BBox cbb=UpdateTracker(tracker,cgray.data,cgray.cols,cgray.rows,cgray.step);
		time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
		std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << endl;
        Rect bb(cbb.x, cbb.y, cbb.w, cbb.h);
#else
		double time_profile_counter = (double)cvGetTickCount();
		CRect cbb = tracker.update(cgray);
		time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
		std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << endl;
        Rect bb(cbb.x, cbb.y, cbb.width, cbb.height);
#endif

		//! [visualization]// draw the tracked object
		rectangle(frame, bb, Scalar(255, 0, 0), 2, 1);
		imshow("tracker", frame);
       //! [visualization]  //quit on ESC button
		if (waitKey(1) == 27)
			break;

		int cut=0;
		if(cut%4==0) waitKey(0),cut++;
	}
#ifdef INTERFACE
	ReleaseTracker(&tracker);
#endif
	waitKey(0);
	//ReleaseTracker(&tracker);
	return EXIT_SUCCESS;
}