#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "highgui.h"
#include "colotracker.h"
#include "region.h"
#include <string>

using namespace cv;

cv::Point g_topLeft(0,0);
cv::Point g_botRight(0,0);
cv::Point g_botRight_tmp(0,0);
bool plot = false;
bool g_trackerInitialized = false;
ColorTracker * g_tracker = NULL;

bool drawing_box = false;
cv::Rect box;
bool selected = false;


static void onMouse( int event, int x, int y, int, void* param)
{
	cv::Mat *image = (cv::Mat*) param;
    if( event == cv::EVENT_LBUTTONDOWN && !g_trackerInitialized){
        std::cout << "DOWN " << std::endl;
        g_topLeft = Point(x,y);
        plot = true;

        drawing_box = true;
		box = cv::Rect(x, y, 0, 0);

    }else if (event == cv::EVENT_LBUTTONUP && !g_trackerInitialized){
        std::cout << "UP " << std::endl;
        g_botRight = Point(x,y);
        plot = false;
        if (g_tracker != NULL)
            delete g_tracker;

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
    }else if (event == cv::EVENT_MOUSEMOVE && !g_trackerInitialized){
        //plot bbox
		if (drawing_box){
			box.width = x - box.x;
			box.height = y - box.y;
		}
        g_botRight_tmp = Point(x,y);
        // if (plot){
        //     cv::rectangle(img, g_topLeft, current, cv::Scalar(0,255,0), 2);
        //     imshow("output", img);
        // }
    }
	
}

/*
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
*/


int main(int argc, char **argv) 
{
    BBox * bb = NULL;
    //cv::Mat img;
    int captureDevice = 0;
    if (argc > 1)
        captureDevice = atoi(argv[1]);

    /*
	cv::VideoCapture webcam = cv::VideoCapture(captureDevice);
    webcam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    webcam.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	*/

	cv::VideoCapture webcam("../1.mp4");

    if (!webcam.isOpened()){
        webcam.release();
        std::cerr << "Error during opening capture device!" << std::endl;
        return 1;
    }
	Mat frameorigin;
	webcam>>frameorigin;
	float scale=0.8;
	Mat img(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
	cv::resize(frameorigin,img,img.size());


    cv::namedWindow( "original image", 0 );
	Mat temp_img = img.clone();
    cv::setMouseCallback( "original image", onMouse, &temp_img);
	cv::imshow("original image", img);
/*
	cv::namedWindow("original image");
	Mat temp_img = img.clone();
	cv::setMouseCallback("original image", onMouse, (void*)&temp_img);*/
	
	
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

	
	g_tracker = new ColorTracker();
	g_tracker->init(*(cv::Mat *)&img, g_topLeft.x, g_topLeft.y, g_botRight.x, g_botRight.y);
	g_trackerInitialized = true;



    for(;;){

        webcam >> frameorigin; 

		if(frameorigin.empty()) break;

		Mat img(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
		cv::resize(frameorigin,img,img.size());

        int c = waitKey(10);
        if( (c & 255) == 27 ) {
            std::cout << "Exiting ..." << std::endl;
            break;
        }

        //some control
        switch( (char)c ){
        case 'i':
            g_trackerInitialized = false;
            g_topLeft = cv::Point(0,0);
            g_botRight_tmp = cv::Point(0,0);
            break;
        default:;
        }

        if (g_trackerInitialized && g_tracker != NULL){
			double time_profile_counter = (double)cvGetTickCount();
            bb = g_tracker->track(img);
			time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
			std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
        }

        if (!g_trackerInitialized && plot && g_botRight_tmp.x > 0){
            cv::rectangle(img, g_topLeft, g_botRight_tmp, cv::Scalar(0,255,0), 2);
        }

        if (bb != NULL){
            cv::rectangle(img, Point2i(bb->x, bb->y), Point2i(bb->x + bb->width, bb->y + bb->height), Scalar(255, 0, 0), 3);
            //cv::rectangle(img, Point2i(bb->x-5, bb->y-5), Point2i(bb->x + bb->width+5, bb->y + bb->height+5), Scalar(0, 0, 255), 1);
            delete bb;
            bb = NULL;
        }

        cv::imshow("output", img);

    }

    if (g_tracker != NULL)
        delete g_tracker;
    return 0;

	cv::waitKey(0);
}
