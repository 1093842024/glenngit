// HDalg.cpp : Defines the entry point for the console application.
//
#include "HDalg.h"
#include "CFG_OpenCV.h"


#include <stdlib.h>
#include <iostream>
#include <string>

using namespace cv;
using std::vector;
#define RECORDVIDEO
//#define VIDEO


int dive=64;
#define MB(image,col,row) image.at<Vec3b>(row,col)[0]/dive*dive+dive/2
#define MG(image,col,row) image.at<Vec3b>(row,col)[1]/dive*dive+dive/2
#define MR(image,col,row) image.at<Vec3b>(row,col)[2]/dive*dive+dive/2

void MatToImg(Mat src,int w,int h,unsigned char* img)
{
	for (int j = 0; j < h; j++ )      // 转成正向图像
	{
		uchar *cp=src.ptr<uchar>(j);
		for (int i = 0; i < w; i++ )
		{
			img[ ( j*w+i )*3 ] = cp[0];  //MR(src,i,j);
			img[ ( j*w+i )*3+1 ] =cp[1];  //MG(src,i,j);
			img[ ( j*w+i )*3+2 ] =cp[2];  //MB(src,i,j);
			cp+=3;
		}
	}
}

void ImgToGray(unsigned char* img,int w,int h,unsigned char* gray)
{
	for (int j = 0; j < h; j++ )     
	{
		for (int i = 0; i < w; i++ )
		{
			gray[j*w+i]=(15*img[ ( j*w+i )*3 ]+75*img[ ( j*w+i )*3+1 ]+38*img[ ( j*w+i )*3+2 ])>>7;   //grey=(R*38+G*75+B*15)>>7
			
		}
	}
}
/*void ImgToGray(Mat src,int w,int h,unsigned char* gray)
{
	for (int j = 0; j < h; j++ )     
	{
		uchar *cp=src.ptr<uchar>(j);
		for (int i = 0; i < w; i++ )
		{
			gray[j*w+i]=(15*cp[0]+75*cp[1]+38*cp[2])>>7;
			cp+=3;
		}
	}
}*/


#ifdef VIDEO

int main(int argc, char* argv[])
{
    HumanDetHandle phdHandle = CreateBackModel();
    VideoCapture cap( 
		"../20.2.avi"
		//"../5.mp4"
		//"../12.avi"
    );
	if (!cap.isOpened()){
		printf("video open failed!"); 
		cv::waitKey(0);
		return -1;
	}
    Mat frame;
    cap>>frame;

#ifdef RECORDVIDEO
	double fps=cap.get(CV_CAP_PROP_FPS);
	//int w=static_cast<int>(frame.rows);       //这个地方很可能由于类型转换导致size不准确，因此最好用下面的方式设置size。
	//int h= static_cast<int>(frame.cols);
	cv::Size size=frame.size();
	cv::VideoWriter writevideo("../1201.avi",CV_FOURCC('M','P','4','2'), fps, size);
#endif

    Mat gray;
    cvtColor( frame, gray, COLOR_BGR2GRAY );
    int frm_num = 0, half_num = 0, jmp_cnt = 0;
    double t0 = cvGetTickCount();
    //cv::Rect det_roi = cv::Rect(0,0,frame.cols/2,frame.rows);
    cv::Rect det_roi = cv::Rect(0,0,frame.cols,frame.rows);
    //SetROI(phdHandle,det_roi.x,det_roi.y,det_roi.width,det_roi.height,0);
    double totalTime = 0.0;

    while (1)
    {
        cap >>frame;
        if (frame.empty()) { break;}
		
	

        cvtColor( frame, gray, COLOR_BGR2GRAY );
        double t = getTickCount();
		double time_profile_counter = (double)cvGetTickCount();
        int ret = UpdateAndHD(phdHandle, (unsigned char*)gray.data, gray.cols, gray.rows, gray.step, 0);
		time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
		std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
		//printf("ret:%d\n",ret);
        t = getTickCount() - t;
        totalTime += t;
        if (3 == ret)
        {
            putText( frame, "MD", cv::Point( 100,100 ), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0,255,0), 2 );
        }
        ReturnObjs * out_objs = getObjs(phdHandle);
        int tl_x,tl_y,br_x,br_y;
        for (int kk=0; kk<out_objs->num; kk++)
        {
            float fwidth = sqrt(out_objs->obj[kk].area*det_roi.height/(out_objs->obj[kk].hvsw*det_roi.width));
            float fheight = fwidth*out_objs->obj[kk].hvsw*det_roi.width/det_roi.height;
            tl_x = det_roi.x + int(det_roi.width*(out_objs->obj[kk].cx - fwidth/2));
            tl_y = det_roi.y + int(det_roi.height*(out_objs->obj[kk].cy - fheight/2));
            br_x = det_roi.x + int(det_roi.width*(out_objs->obj[kk].cx + fwidth/2));
            br_y = det_roi.y + int(det_roi.height*(out_objs->obj[kk].cy + fheight/2));
            if (out_objs->obj[kk].bflag)
            {
                char percent[64];
                sprintf(percent,"%0.1f%% %d %0.2f",out_objs->obj[kk].area*100,out_objs->obj[kk].nof,out_objs->obj[kk].xspeed);
                putText( frame, percent, Point( tl_x,tl_y ), FONT_HERSHEY_SIMPLEX,1.0, Scalar(0,255,0),2 );
                rectangle(frame,Point(tl_x,tl_y),Point(br_x,br_y),Scalar(0,255,0),2);
				printf("motion detected!\n");

				if (out_objs->obj[0].show==true)  //蓝色和水蓝色
				{
                Rect bb(out_objs->obj[0].x,out_objs->obj[0].y,out_objs->obj[0].w,out_objs->obj[0].h);
			    rectangle(frame, bb, Scalar(255, 0, 0), 2, 1);
			    Rect cbb(out_objs->obj[0].xn,out_objs->obj[0].yn,out_objs->obj[0].wn,out_objs->obj[0].hn);
			    rectangle(frame, cbb, Scalar(255, 255, 0), 2, 1);
				}
				waitKey(0);

            }else{
                rectangle(frame,Point(tl_x,tl_y),Point(br_x,br_y),Scalar(0,0,255),2);
            }

			//printf("%d  %d  %d  %d",track->x,track->y,track->w,track->h);
			//rectangle(frame,Point(DrawTrack1.x,DrawTrack1.y),Point(DrawTrack1.x+DrawTrack1.w,DrawTrack1.y+DrawTrack1.h),Scalar(0,0,255),2);
			
        }


        //Mat showImg;
        //resize(frame,showImg,Size(640,360));
        imshow( "original", frame);

#ifdef RECORDVIDEO
		writevideo<<frame;
#endif

        char c = waitKey(1);
        if ( c == 27 )
            break;
    }
    printf( "Total time: %gms\n", totalTime/(cvGetTickFrequency()*1000.0) );
    ReleaseBackModel(&phdHandle);
    system("pause");
    return 0;
}

#else

int main()
{

	 HumanDetHandle phdHandle = CreateBackModel();
	
	cv::Mat frame;

	string img_root = "C:/Users/glennge/Desktop/移动跟踪/算法作者源码/pedestrian detection dataset";
	string img_path = img_root + "/highway/input/";
	string first_frame_name = img_path + "in000001.jpg";
	frame = imread(first_frame_name, 1);


#ifdef RECORDVIDEO
	double fps=25;
	//int w=static_cast<int>(frame.rows);       //这个地方很可能由于类型转换导致size不准确，因此最好用下面的方式设置size。
	//int h= static_cast<int>(frame.cols);
	cv::Size size=frame.size();
	cv::VideoWriter writevideo("../highway.avi",CV_FOURCC('M','P','4','2'), fps, size);
#endif
	int Wid,Hei;
	Wid = frame.cols;                    //图像大小
	Hei = frame.rows; 
	unsigned char* frame_color=new unsigned char [Wid * Hei * 3];
	unsigned char* frame_gray=new unsigned char [Wid * Hei ];
	

	Mat gray;
	cv::Rect det_roi = cv::Rect(0,0,frame.cols,frame.rows);

	char img_name[128];
	int img_idx = 2;

	 while (1)
    {
		sprintf_s(img_name, "%06d.jpg", img_idx++);
		string in="in";
		in+= img_name;
		string img_full_name = img_path + in/*+ img_name*/;
		
		frame = imread(img_full_name, 1);
        if (frame.empty()) {  break;}

        MatToImg(frame,Wid,Hei,frame_color);
		ImgToGray(frame_color,Wid,Hei,frame_gray);
		//cvtColor( frame, gray, COLOR_BGR2GRAY );


        double t = getTickCount();
		double time_profile_counter = (double)cvGetTickCount();
        //int ret = UpdateAndHD(phdHandle, (unsigned char*)gray.data, Wid, Hei, gray.step, 0,frame_color);
	    int ret = UpdateAndHD(phdHandle, (unsigned char*)frame_gray, Wid, Hei, Wid, 0,frame_color);
		time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
		std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
		//printf("ret:%d\n",ret);
       
		
        if (3 == ret)
        {
            putText( frame, "MD", cv::Point( 100,100 ), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0,255,0), 2 );
        }
        ReturnObjs * out_objs = getObjs(phdHandle);
        int tl_x,tl_y,br_x,br_y;
		
        for (int kk=0; kk<out_objs->num; kk++)
        {
            float fwidth = sqrt(out_objs->obj[kk].area*det_roi.height/(out_objs->obj[kk].hvsw*det_roi.width));
            float fheight = fwidth*out_objs->obj[kk].hvsw*det_roi.width/det_roi.height;
            tl_x = det_roi.x + int(det_roi.width*(out_objs->obj[kk].cx - fwidth/2));
            tl_y = det_roi.y + int(det_roi.height*(out_objs->obj[kk].cy - fheight/2));
            br_x = det_roi.x + int(det_roi.width*(out_objs->obj[kk].cx + fwidth/2));
            br_y = det_roi.y + int(det_roi.height*(out_objs->obj[kk].cy + fheight/2));
            if (out_objs->obj[kk].bflag)
            {
                char percent[64];
                sprintf(percent,"%0.1f%% %d %0.2f",out_objs->obj[kk].area*100,out_objs->obj[kk].nof,out_objs->obj[kk].xspeed);
                putText( frame, percent, Point( tl_x,tl_y ), FONT_HERSHEY_SIMPLEX,1.0, Scalar(0,255,0),2 );
                rectangle(frame,Point(tl_x,tl_y),Point(br_x,br_y),Scalar(0,255,0),2);
				//printf("motion detected!\n");

				if (out_objs->obj[0].show==true)
				{
                //Rect bb(out_objs->obj[0].x,out_objs->obj[0].y,out_objs->obj[0].w,out_objs->obj[0].h);
			    //rectangle(frame, bb, Scalar(255, 0, 0), 2, 1);
			    Rect cbb(out_objs->obj[0].xn,out_objs->obj[0].yn,out_objs->obj[0].wn,out_objs->obj[0].hn);
			    rectangle(frame, cbb, Scalar(255, 255, 0), 2, 1);
				}
				waitKey(0);

            }else{
                rectangle(frame,Point(tl_x,tl_y),Point(br_x,br_y),Scalar(0,0,255),2);
            }
				
        }
		
		
        //Mat showImg;
        //resize(frame,showImg,Size(640,360));
        imshow( "original", frame);

#ifdef RECORDVIDEO
		writevideo<<frame;
#endif

        char c = waitKey(1);
        if ( c == 27 )
            break;
    }

	delete frame_color;
	delete frame_gray;
    ReleaseBackModel(&phdHandle);
    system("pause");
	return EXIT_SUCCESS;
}

#endif
