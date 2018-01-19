#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"
#include <stdlib.h>
#include <iostream>
#include <string>

//#include "dirent.h"
//#include "basetype.h"

using namespace std;
using namespace cv;

//#define RECORDVIDEO
//#define PIC

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


int main(int argc, char* argv[]){

	if (argc > 5) return -1;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;
	//bool LAB = true;
	bool LAB = false;

	for(int i = 0; i < argc; i++){
		if ( strcmp (argv[i], "hog") == 0 )
			HOG = true;
		if ( strcmp (argv[i], "fixed_window") == 0 )
			FIXEDWINDOW = true;
		if ( strcmp (argv[i], "singlescale") == 0 )
			MULTISCALE = false;
		if ( strcmp (argv[i], "show") == 0 )
			SILENT = false;
		if ( strcmp (argv[i], "lab") == 0 ){
			LAB = true;
			HOG = true;}
		if ( strcmp (argv[i], "gray") == 0 )
			HOG = false;
	}
	
	// Create KCFTracker object
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
	// Frame readed
	Mat frameorigin;
	// Tracker results
	//Rect result;

	

#ifdef PIC
	string img_root = "C:/Users/glennge/Desktop/移动跟踪/算法作者源码/vot2016";
	string img_path = img_root + "/leaves/";  
	string first_frame_name = img_path + "00000001.jpg";
	frameorigin = imread(first_frame_name, 1);
	char img_name[128];
	int img_idx = 2;

#else
	VideoCapture cap("../5.mp4");
	if (!cap.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}
	cap >> frameorigin;
#endif	


	float scale=1;
	Mat frame(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
	cv::resize(frameorigin,frame,frame.size());



#ifdef RECORDVIDEO
	double fps=cap.get(CV_CAP_PROP_FPS);
	int w=static_cast<int>(frame.rows);
	int h= static_cast<int>(frame.cols);
	cv::Size size=Size(w,h);
	int count=cap.get(CV_CAP_PROP_FRAME_COUNT);
	cv::VideoWriter writevideo("05_1.avi",CV_FOURCC('M','J','P','G'), fps, size);
#endif

	cv::namedWindow("original image");
	Mat temp_img = frame.clone();
	cv::setMouseCallback("original image", create_mouse_callback, (void*)&temp_img);
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
	//Mat frame_gray;
	//cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);

	//CMat cgray = CMat(CSize(frame_gray.cols, frame_gray.rows), MAT_8UC1, frame_gray.data);

	Rect cbox(box.x, box.y, box.width, box.height);
	//Rect cbox(311,78,203,139);
	tracker.init(cbox, frame);
	printf("%d %d %d %d\n",box.x,box.y,box.width, box.height);
	printf("Start the tracking process\n"); 



	for (;;) {
#ifdef PIC
		sprintf_s(img_name, "%08d.jpg", img_idx++);
		string img_full_name = img_path + img_name;
		frameorigin = imread(img_full_name, 1);
#else
		cap >> frameorigin;
#endif
		if (frameorigin.empty()) break;
		// stop the program if no more images
		if (frameorigin.rows == 0 || frameorigin.cols == 0)
			break;

        Mat frame(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
	    cv::resize(frameorigin,frame,frame.size());


		// update the tracking result
		//! [update]
		//tracker->update(frame, box);
		//Mat frame_gray;
		//cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
		//CMat cgray = CMat(CSize(frame_gray.cols, frame_gray.rows), MAT_8UC1, frame_gray.data);
		double time_profile_counter = (double)cvGetTickCount();
		Rect cbb = tracker.update(frame);
		Rect bb(cbb.x, cbb.y, cbb.width, cbb.height);
		time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
		std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << endl;
		//! [update]

		//! [visualization]// draw the tracked object
		rectangle(frame, bb, Scalar(255, 0, 0), 2, 1);

		// show image with the tracked object
		imshow("tracker", frame);

#ifdef RECORDVIDEO
		writevideo<<frame;
#endif

		
		int cut=0;
		if(cut%4==0) waitKey(0),cut++;
		

		//! [visualization]//quit on ESC button
		if (waitKey(1) == 27)
			break;
	}

	waitKey(0);
	//ReleaseTracker(&tracker);
	return EXIT_SUCCESS;


/*
	// Path to list.txt
	ifstream listFile;
	string fileName = "images.txt";
  	listFile.open(fileName);
  	// Read groundtruth for the 1st frame
  	ifstream groundtruthFile;
	string groundtruth = "region.txt";
  	groundtruthFile.open(groundtruth);
  	string firstLine;
  	getline(groundtruthFile, firstLine);
	groundtruthFile.close();
  	
  	istringstream ss(firstLine);

  	// Read groundtruth like a dumb
  	float x1, y1, x2, y2, x3, y3, x4, y4;
  	char ch;
	ss >> x1;
	ss >> ch;
	ss >> y1;
	ss >> ch;
	ss >> x2;
	ss >> ch;
	ss >> y2;
	ss >> ch;
	ss >> x3;
	ss >> ch;
	ss >> y3;
	ss >> ch;
	ss >> x4;
	ss >> ch;
	ss >> y4; 

	// Using min and max of X and Y for groundtruth rectangle
	float xMin =  min(x1, min(x2, min(x3, x4)));
	float yMin =  min(y1, min(y2, min(y3, y4)));
	float width = max(x1, max(x2, max(x3, x4))) - xMin;
	float height = max(y1, max(y2, max(y3, y4))) - yMin;

	// Read Images
	ifstream listFramesFile;
	string listFrames = "images.txt";
	listFramesFile.open(listFrames);
	string frameName;


	// Write Results
	ofstream resultsFile;
	string resultsPath = "output.txt";
	resultsFile.open(resultsPath);

	// Frame counter
	int nFrames = 0;


	while ( getline(listFramesFile, frameName) ){
		frameName = frameName;

		// Read each frame from the list
		frame = imread(frameName, CV_LOAD_IMAGE_COLOR);

		// First frame, give the groundtruth to the tracker
		if (nFrames == 0) {
			tracker.init( Rect(xMin, yMin, width, height), frame );
			rectangle( frame, Point( xMin, yMin ), Point( xMin+width, yMin+height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << xMin << "," << yMin << "," << width << "," << height << endl;
		}
		// Update
		else{
			result = tracker.update(frame);
			rectangle( frame, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << result.x << "," << result.y << "," << result.width << "," << result.height << endl;
		}

		nFrames++;

		if (!SILENT){
			imshow("Image", frame);
			waitKey(1);
		}
	}
	resultsFile.close();

	listFile.close();
	*/




}
