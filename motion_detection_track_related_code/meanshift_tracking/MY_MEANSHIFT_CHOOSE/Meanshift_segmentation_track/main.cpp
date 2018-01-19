//
//  main.cpp
//  Meanshift new
//
//  Created by Bran on 14-3-24.
//  Copyright (c) 2014年 Bran. All rights reserved.
//


#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <iostream>
#include "MeanShift.h"
#include <fstream>      
using namespace std;

#define DECLARE_TIMING(s)  int64 timeStart_##s; double timeDiff_##s; double timeTally_##s = 0; int countTally_##s = 0
#define START_TIMING(s)    timeStart_##s = cvGetTickCount()
#define STOP_TIMING(s) 	   timeDiff_##s = (double)(cvGetTickCount() - timeStart_##s); timeTally_##s += timeDiff_##s; countTally_##s++
#define GET_TIMING(s) 	   (double)(timeDiff_##s / (cvGetTickFrequency()*1000.0))
#define GET_AVERAGE_TIMING(s)   (double)(countTally_##s ? timeTally_##s/ ((double)countTally_##s * cvGetTickFrequency()*1000.0) : 0)
#define CLEAR_AVERAGE_TIMING(s) timeTally_##s = 0; countTally_##s = 0



#define B(image,x,y) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)*3]		//B
#define G(image,x,y) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)*3+1]	//G
#define R(image,x,y) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)*3+2]	//R
#define S(image,x,y) ((uchar*)(image->imageData + image->widthStep*(y)))[(x)]	

#define Num 10  //帧差的间隔
#define ai 0.08 //学习率
#define SKIP_FRAME_COUNT 10   //跳过的帧数
#define FRAME_RATE 29         //帧率

#define VIDEO


bool pause=false;            //是否暂停
bool track = false;          //是否开启跟踪
bool selectRegion = true;    //是否选择目标区域
IplImage *curframe=NULL;     //当前帧图像的变量

unsigned char * img;        //把iplimg改到char*  便于计算
int xin,yin;               //跟踪时输入的中心点
int xout,yout;             //跟踪时得到的输出中心点
int Wid,Hei;               //图像的大小
int WidIn,HeiIn;           //输入的半宽与半高
int WidOut,HeiOut;         //输出的半宽与半高


#define MAX_OBJECTS 5
typedef struct params {
	CvPoint loc1[MAX_OBJECTS];
	CvPoint loc2[MAX_OBJECTS];
	IplImage* objects[MAX_OBJECTS];
	char* win_name;
	IplImage* orig_img;
	IplImage* cur_img;
	int n;
} params;


int get_regions( IplImage*, CvRect** );
//鼠标函数
void mouse( int, int, int, int, void* );
void IplToImg(IplImage* src, int w,int h);


int main()
{
	float totalDist = 0;
	int FrameNum=0;
#ifdef VIDEO
           CvCapture *capture = cvCreateFileCapture("../car.avi");
		   curframe=cvQueryFrame(capture);
		   if(!curframe) return -1;
#endif
	
    
    
	cvNamedWindow("video",1);
    
	uchar key = false;      //用来设置暂停
	float rho_v;//表示相似度
	int distx,disty;//用来记录输入与输出目标的距离
	int count = 0; //用来记录输入与输出目标跟踪相近的次数，当次数大于某个阈值时，说明目标跟踪丢失。
    //PP ret;
    
	CvRect* regions;
	MeanshiftTracker T;

#ifdef VIDEO 
	while(capture)
	{		
		curframe=cvQueryFrame(capture); //抓取一帧
		FrameNum ++;
		if(!curframe)
			break;

		if(selectRegion)//第一帧时初始化过程   如果开启了选择目标区域功能，进行手动选择
		{ 
			Wid = curframe->width;                    //图像大小
			Hei = curframe->height; 
			img = new unsigned char [Wid * Hei * 3];  //定义变量存储 图像三个通道的信息

			get_regions(curframe, &regions);         //调用函数，通过鼠标获取目标区域矩形

			CvRect r = regions[0];			
			int centerX = r.x+r.width/2;//得到目标中心点
			int centerY = r.y+r.height/2;

			WidIn = r.width/2;          //得到目标半宽与半高，中心x，y坐标，作为meanshift的跟踪初始位置
			HeiIn = r.height/2;
			xin = centerX;
			yin = centerY;

			IplToImg(curframe,Wid,Hei);    //将图像curframe信息由Ipl格式存为unsigned char * img格式，脱离opencv库

			T.Initial_MeanShift_tracker(centerX,centerY,WidIn,HeiIn,img,Wid,Hei,1.0/FRAME_RATE);  //利用img和初始目标信息，初始化跟踪目标

			track = true;                  //开启跟踪
			selectRegion = false;          //关闭选择区域功能
			totalDist = 0;                 //总距离为零
			 
			//continue;
		}
		else
		IplToImg(curframe,Wid,Hei);	 //将图像curframe信息由Ipl格式存为unsigned char * img格式，脱离opencv库	

		if(track)
		{
			double start,end;
			start=cvGetTickCount();
			/* 跟踪一帧，输入前一帧的位置，跟踪得到后一帧的位置*/
			rho_v = T.MeanShift_tracker( xin, yin, WidIn, HeiIn, img, Wid, Hei, xout, yout, WidOut, HeiOut );
			end=cvGetTickCount();
			printf("use time:%.2fms\n",(end-start)/((double)cvGetTickFrequency()*1000));

			/* 画框: 新位置为蓝框 */
			cvRectangle(curframe,cvPoint(xout - WidOut, yout - HeiOut),cvPoint(xout+WidOut,yout+HeiOut),cvScalar(255,0,0),2,8,0);
			xin = xout; yin = yout;
			WidIn = WidOut; HeiIn = HeiOut;

			if ( rho_v < 0.8 )  /* 判断是否目标丢失 */
			{				
				if(count>20)
				{
					//MessageBox(TEXT("target loss, please relocate target"));
					xin = 0; yin = 0;
					xout =0; yout = 0;
					WidIn = 0; HeiIn = 0;
					WidOut = 0;HeiOut = 0;
					distx = 10; disty = 10;
					track = false;
				}
				else
				{
					distx=abs(xin-xout);
					disty = abs(yin-yout);
					if(distx<1&&disty<1)
					{
						count = count+1;
					}
					xin = xout; yin = yout;
					WidIn = WidOut; HeiIn = HeiOut;
				}				
			}
		}
#else
    string picture;
    char interval;
    string filename="/Users/Bran/Desktop/TestSet/Skating1/";
    ifstream fin(filename+"imagelist.txt");
    ifstream gin(filename+"groundtruth_rect.txt");
    
    int x1,x2,w,h;
    //CAMShift func;
    const int LINE_LENGTH=100;
    char str[LINE_LENGTH-1];
	//curframe=cvQueryFrame(capture);
    if (!fin.is_open()||!gin.is_open())
        return -1;
    char buf[100];
    int Buf_i=0;
    IplImage* frame = 0;
    //func.getPosition(446,181,29,26);//ground truth
    /*for(;;){
        Buf_i++;
        sprintf(buf, "/Users/Bran/Desktop/TestSet/Coke/img/%04d.jpg", Buf_i);
        cout<<buf<<endl;
        frame = cvLoadImage(buf);
        cvShowImage( "CamShiftDemo", frame );
        if(!frame)
            break;
        PP ret = func.Tracking(buf);// path
        CvPoint x1 = ret.first;	// rectangle	point
        CvPoint x2 = ret.second;// rectangle	point
        cvRectangle( frame, x1,x2, CV_RGB(73,228,81), 3, CV_AA, 0 );
        cvShowImage( "CamShiftDemo", frame );
        cvWaitKey(1);
    }*/
    //getchar();
	while(fin.getline(str,LINE_LENGTH))
	{
		//curframe=cvQueryFrame(capture); 用视频作为输入
        
        //cout<<str<<endl;
        //getchar();
        gin>>x1>>interval>>x2>>interval>>w>>interval>>h;
        picture=filename+str;
        curframe=cvLoadImage(picture.c_str());
		FrameNum ++;
		if(!curframe)
			break;
 		/*
		if(selectRegion)//鼠标选中区域
		{
			Wid = curframe->width;
			Hei = curframe->height;
			img = new unsigned char [Wid * Hei * 3]; //选中图片大小 RGB三通道
            
            
			get_regions(curframe, &regions);
            
			CvRect r = regions[0];
            //func.getPosition(r.x,r.y,r.width,r.height);//ground truth
            func.getPosition(x1,x2,w,h);
            //ret = func.Tracking(picture.c_str());
			int centerX = r.x+r.width/2;//中心
			int centerY = r.y+r.height/2;
			WidIn = r.width/2;//输入框大小
			HeiIn = r.height/2;
			
			xin = centerX;
			yin = centerY;
            cvRectangle(curframe,cvPoint(r.x-r.width/2, r.y-r.height/2),cvPoint(r.x+r.width/2,r.y+r.height/2),cvScalar(255,0,0),2,8,0);
			IplToImg(curframe,Wid,Hei);
			
			//T.Initial_MeanShift_tracker(centerX,centerY,WidIn,HeiIn,img,Wid,Hei,1.0/FRAME_RATE);
			track = true;
			selectRegion = false;
			totalDist = 0;
            //cvShowImage("video",curframe);
            //while(1);
            continue;
		}
         */
        //getchar();

        if (!track) {
            Wid = curframe->width;
            Hei = curframe->height;
            img = new unsigned char [Wid * Hei * 3]; //选中图片大小 RGB三通道
            IplToImg(curframe,Wid,Hei);
            cout<<x1<<x2<<w<<h;
            WidIn = w/2;//输入框大小
            HeiIn = h/2;
            int centerX = x1+w/2;//中心
            int centerY = x2+h/2;
            xin = centerX;
            yin = centerY;

            T.Initial_MeanShift_tracker(centerX,centerY,WidIn,HeiIn,img,Wid,Hei,1.0/FRAME_RATE);
            cvRectangle(curframe,cvPoint(xin - WidIn, yin - HeiIn),cvPoint(xin + WidIn,yin + HeiIn),cvScalar(255,0,0),2,8,0);
            //func.getPosition(x1,x2,w,h);
            track=true;
            cvShowImage("video",curframe);
        }

        IplToImg(curframe,Wid,Hei);
		if(track)
		{
            cout<<picture.c_str()<<endl;
            //ret = func.Tracking(picture.c_str());
            //cout<<ret.first.x<<endl;
			rho_v = T.MeanShift_tracker( xin, yin, WidIn, HeiIn, img, Wid, Hei, xout, yout, WidOut, HeiOut );
			
			cvRectangle(curframe,cvPoint(xout - WidOut, yout - HeiOut),cvPoint(xout+WidOut,yout+HeiOut),cvScalar(255,0,0),2,8,0);
            cvRectangle(curframe, cvPoint(x1,x2), cvPoint(x1+w,x2+h), CV_RGB(73,228,81), 3, CV_AA, 0);  

			xin = xout; yin = yout;
			WidIn = WidOut; HeiIn = HeiOut;
            
			/*if ( rho_v < 0.8 )
			{
				if(count>20)
				{
					//MessageBox(TEXT("target loss, please relocate target"));
					xin = 0; yin = 0;
					xout =0; yout = 0;
					WidIn = 0; HeiIn = 0;
					WidOut = 0;HeiOut = 0;
					distx = 10; disty = 10;
					track = false;
				}
				else
				{
					distx=abs(xin-xout);
					disty = abs(yin-yout);
					if(distx<1&&disty<1)
					{
						count = count+1;
					}
					xin = xout; yin = yout;
					WidIn = WidOut; HeiIn = HeiOut;
				}
			}*/
		}
#endif
		
		if(track) {cout<<FrameNum<<":\t"<<rho_v<<endl;  totalDist += rho_v;}
		else cvWaitKey(10);

 		cvShowImage("video",curframe);
        
		while(pause)          //如果暂停以后，可以通过c重新选择区域
		{
			key = cvWaitKey(10);
			if(key == 'c')   
			{
				selectRegion = true;
				pause = false;
			}
			else if(key == ' ')   //空格暂停
			{
				pause = false;
			}
		}

		key = cvWaitKey(10);
		if(key == 27)          //ESC 推出程序
		{
			break;
		}
 		else if(key == ' ')   //空格暂停
		{
			pause = true;
		}
		else if(key == 'c')   //c暂停，并开启重新选择区域功能
		{
			selectRegion = true;
			pause = false;
		}
		
	}
    
	cout<<"average dist:"<<totalDist/(FrameNum-SKIP_FRAME_COUNT);
    
	cvReleaseImage(&curframe);
	cvDestroyAllWindows();
    
	T.Clear_MeanShift_tracker();
    
	//getchar();
}

//int Wid,Hei;
//三通道整合
void IplToImg(IplImage* src, int w,int h)    //将图像由IPL格式转化为unsigned char *格式
{
	int i,j;
	for ( j = 0; j < h; j++ )      // 转成正向图像
		for ( i = 0; i < w; i++ )
		{
			img[ ( j*w+i )*3 ] = R(src,i,j);
			img[ ( j*w+i )*3+1 ] = G(src,i,j);
			img[ ( j*w+i )*3+2 ] = B(src,i,j);
		}
}

int get_regions( IplImage* frame, CvRect** regions ) 
{
	char* win_name = "Select Region";
	params p;
	CvRect* r;
	int i, x1, y1, x2, y2, w, h;
    
	/* use mouse callback to allow user to define object regions */
	p.win_name = win_name;
	p.orig_img = (IplImage *)cvClone( frame );
	p.cur_img = NULL;
	p.n = 0;
	cvNamedWindow( win_name, 1 );
	cvShowImage( win_name, frame );
	cvSetMouseCallback( win_name, &mouse, &p );
	while(cvWaitKey(0) != ' ');
	cvDestroyWindow( win_name );
	cvReleaseImage( &(p.orig_img) );
	if( p.cur_img )
		cvReleaseImage( &(p.cur_img) );
    /*refresh the struct p*/
    
	/* extract regions defined by user; store as an array of rectangles */
	if( p.n == 0 )
	{
		*regions = NULL;
		return 0;
	}
	r = (CvRect *)malloc( p.n * sizeof( CvRect ) );
	for( i = 0; i < p.n; i++ ) //for each rectangle round the object
	{
		x1 = MIN( p.loc1[i].x, p.loc2[i].x ); //determine the query rectangle
		x2 = MAX( p.loc1[i].x, p.loc2[i].x );
		y1 = MIN( p.loc1[i].y, p.loc2[i].y );
		y2 = MAX( p.loc1[i].y, p.loc2[i].y );
		w = x2 - x1;
		h = y2 - y1;
        
		/* ensure odd width and height */
		w = ( w % 2 )? w : w+1;
		h = ( h % 2 )? h : h+1;
		r[i] = cvRect( x1, y1, w, h );    //define one of the rects
	}
	*regions = r;
	return p.n;
}

/*
 Mouse callback function that allows user to specify the initial object
 regions.  Parameters are as specified in OpenCV documentation.
 */
void mouse( int event, int x, int y, int flags, void* param )
{
	params* p = (params*)param;
	CvPoint* loc;
	int n;
	IplImage* tmp;
	static int pressed = 0;
    
	int height=p->orig_img->height;
    
	/* on left button press, remember first corner of rectangle around object */
	if( event == CV_EVENT_LBUTTONDOWN )
	{
		n = p->n;
		if( n == MAX_OBJECTS )
			return;
		loc = p->loc1;
		loc[n].x = x;
		loc[n].y = y;
		pressed = 1;
	}
    
	/* on left button up, finalize the rectangle and draw it in black */
	else if( event == CV_EVENT_LBUTTONUP )
	{
		n = p->n;
		if( n == MAX_OBJECTS )
			return;
		loc = p->loc2;
		loc[n].x = x;
		loc[n].y = y;
		cvReleaseImage( &(p->cur_img) );
		p->cur_img = NULL;
		cvRectangle( p->orig_img, p->loc1[n], loc[n], CV_RGB(255,0,0), 1, 8, 0 );
		cvShowImage( p->win_name, p->orig_img );
		pressed = 0;
		p->n++;
	}
    
	/* on mouse move with left button down, draw rectangle as defined in white */
	else if( event == CV_EVENT_MOUSEMOVE  &&  flags & CV_EVENT_FLAG_LBUTTON )
	{
		n = p->n;
		if( n == MAX_OBJECTS )
			return;
		tmp = (IplImage *)cvClone( p->orig_img );
		loc = p->loc1;
		cvRectangle( tmp, loc[n], cvPoint(x, y), CV_RGB(255,255,255), 1, 8, 0 );
		cvShowImage( p->win_name, tmp );  
		if( p->cur_img )  
			cvReleaseImage( &(p->cur_img) );  
		p->cur_img = tmp;  
	}  
} 

