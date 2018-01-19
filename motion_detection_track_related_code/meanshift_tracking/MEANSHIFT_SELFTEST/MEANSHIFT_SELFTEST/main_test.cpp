#include <iostream>
#include <cmath>
#include <math.h>
#include <vector>
#include <fstream> 
#include "MeanShift.h"

using namespace std;

#define DECLARE_TIMING(s)  int64 timeStart_##s; double timeDiff_##s; double timeTally_##s = 0; int countTally_##s = 0
#define START_TIMING(s)    timeStart_##s = cvGetTickCount()
#define STOP_TIMING(s) 	   timeDiff_##s = (double)(cvGetTickCount() - timeStart_##s); timeTally_##s += timeDiff_##s; countTally_##s++
#define GET_TIMING(s) 	   (double)(timeDiff_##s / (cvGetTickFrequency()*1000.0))
#define GET_AVERAGE_TIMING(s)   (double)(countTally_##s ? timeTally_##s/ ((double)countTally_##s * cvGetTickFrequency()*1000.0) : 0)
#define CLEAR_AVERAGE_TIMING(s) timeTally_##s = 0; countTally_##s = 0

//#define IPL
#define VIDEO

#ifdef IPL
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>


//用于IplImage格式图像
#define B(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col*image->nChannels+0]
#define G(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col*image->nChannels+1]
#define R(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col*image->nChannels+2]
#define S(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col]

IplImage *curframe=NULL;     //当前帧图像的指针

#define MAX_OBJECTS 5
typedef struct params{           //鼠标画框的目标矩形信息，可以包含多个目标信息
	CvPoint loc1[MAX_OBJECTS];
	CvPoint loc2[MAX_OBJECTS];
	IplImage* objects[MAX_OBJECTS];
	char* win_name;
	IplImage* orig_img;
	int n;
}params;

int get_regions(IplImage*,CvRect**);
void mouse(int,int,int,int,void*);
void IplToImg(IplImage* src,int w,int h);


#else
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

//用于Mat格式图像
int dive=64;
#define MB(image,col,row) image.at<Vec3b>(row,col)[0]/dive*dive+dive/2
#define MG(image,col,row) image.at<Vec3b>(row,col)[1]/dive*dive+dive/2
#define MR(image,col,row) image.at<Vec3b>(row,col)[2]/dive*dive+dive/2

Mat curframe;

#define MAX_OBJECTS 5
typedef struct params{           //鼠标画框的目标矩形信息，可以包含多个目标信息
	Point loc1[MAX_OBJECTS];
	Point loc2[MAX_OBJECTS];
	int n;
}params;

int get_regions(Rect**);
void mouse(int,int,int,int,void*);
void MatToImg(Mat src,int w,int h);

#endif


#define Num 10               //帧差的间隔
#define ai 0.08              //学习率
#define SKIP_FRAME_COUNT 10  //跳过的帧数
#define FRAME_RATE 25        //帧率

bool pause=false;            //是否暂停
bool track=false;            //是否开启追踪
bool selectRegion=true;      //是否选择目标区域

unsigned char* img;          //把ipl/mat图像转化为char *便于移植计算
int xin,yin;                 //跟踪时输入的图像中心点
int WidIn,HeiIn;             //跟踪时输入的图像的半宽和半高
int xout,yout;               //跟踪式输出的图像中心点
int WidOut,HeiOut;           //跟踪式输出的图像的半宽和半高
int Wid,Hei;                 //整个图像的宽和高



#ifdef IPL

int main()
{
	float totalDist=0;
	int FrameNum=0;
	char* winname="MeanShift";
#ifdef VIDEO
	CvCapture *capture = cvCreateFileCapture("../car.avi");
	curframe=cvQueryFrame(capture);
	if(!curframe) return -1;
#endif
	cvNamedWindow(winname,1);
	cvShowImage(winname,curframe);

	uchar key=false;         //用来读入键盘输入
	float rho_v;             //表示相似度
	int distx,disty;         //用来记录输入与输出目标的距离
	int count=0;               //用来记录输入与输出目标跟踪相近的次数，当次数大于某个阈值时，判断为目标丢失
	
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
			CvRect* regions;

			Wid = curframe->width;                    //图像大小
			Hei = curframe->height; 
			img = new unsigned char [Wid * Hei * 3];  //定义变量存储 图像三个通道的信息


			int valide=get_regions(curframe, &regions);         //调用函数，通过鼠标获取目标区域矩形
			if(!valide) break;

			CvRect r = regions[0];			                   //只提取第一个目标作为跟踪对象
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

			free(regions);
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
					count =0;
					printf("****************missing target!\n");
				}
				else
				{
					distx=abs(xin-xout);
					disty =abs(yin-yout);
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

#endif

	    if(track) {cout<<FrameNum<<":\t"<<rho_v<<endl;  totalDist += rho_v;}
	    else cvWaitKey(10);

	    cvShowImage(winname,curframe);

	    while(pause)                                                 //如果暂停以后，可以通过c重新选择区域
	    {
		  key = cvWaitKey(10);
		  if(key == 'c')  {  selectRegion = true;  pause = false; } //c暂停，并开启重新选择区域功能
		  else if(key == ' ') pause = false;                         //空格暂停
	    }

	      key = cvWaitKey(10);
	      if(key == 27) break;                                       //ESC 推出程序
	      else if(key == ' ')  pause = true;                         //空格暂停
	      else if(key == 'c')                                        //c暂停，并开启重新选择区域功能
	      {  selectRegion = true; pause = false;}

    }

    cout<<"average dist:"<<totalDist/(FrameNum-SKIP_FRAME_COUNT);
	cvReleaseCapture(&capture);                 //使用CvQueryFrame取出CvCapture* 每帧图像，最后只需释放CvCapture*，不需要释放IplImage
	cvDestroyAllWindows();                      //使用cvCreateImage()新建一个IplImage*，以及使用cvCreateMat()新建一个CvMat*，都需要cvReleaseImage()  cvReleaseMat()显示的释放
    while(cvWaitKey(0) != ' ');

	return 0;
}

void IplToImg(IplImage* src,int w,int h)
{
	for(int j=0;j<h;j++)
		for(int i=0;i<w;i++)
		{
		    img[(j*w+i)*3]=R(src,i,j);
			img[(j*w+i)*3+1]=G(src,i,j);
			img[(j*w+i)*3+2]=B(src,i,j);
		}
}

int get_regions(IplImage* frame, CvRect** regions)
{
	char* win_name="select region";  //单独显示标记的窗口
	params p;                        //创建存储标记信息的数据结构
	CvRect* r;                       //创建画框的区域数据指针
	int x1,y1,x2,y2,w,h;

	p.win_name=win_name;                    //标记窗口的名字
	p.orig_img=(IplImage*)cvClone(frame);   //标记窗口的原始图像副本 即传入的当前图像
	p.n=0;                                  //标记窗口的标记数量 初始为0
	cvNamedWindow(win_name,1);              //打开标记窗口
	cvShowImage(win_name,frame);
	cvSetMouseCallback(win_name,&mouse,&p);     //调用鼠标输入的回调函数mouse，将鼠标操作信息存在p中
	while(cvWaitKey(0) != ' ');                 //按空格键继续运行
	cvDestroyWindow(win_name);                  //销毁标记窗口
	cvReleaseImage(&(p.orig_img));              //释放原始图像副本


	if(p.n==0){                                //检查标记的目标区域数量
		*regions=NULL;
		return 0;
	}
	r=(CvRect*)malloc(p.n*sizeof(CvRect));     //分配存储目标区域矩形数据的内存空间=个数*大小

	for(int i=0;i<p.n;i++)               //计算所有的要标记区域的矩形框
	{
		x1=MIN(p.loc1[i].x, p.loc2[i].x);    
		x2=MAX(p.loc1[i].x, p.loc2[i].x);
		y1=MIN(p.loc1[i].y, p.loc2[i].y);
		y2=MAX(p.loc1[i].y, p.loc2[i].y);
		w=x2-x1;
		h=y2-y1;

		w=(w%2)?w:w+1;
		h=(h%2)?h:h+1;
		r[i]=cvRect(x1,y1,w,h);
	}

	*regions=r;                                //将区域信息给到regions
	return p.n;                                //返回区域数量

}

void mouse(int event,int x,int y, int flags, void* param)
{
	params* p = (params*)param;    //将传入的操作信息创建副本指针
	CvPoint* loc;                  //创建目标位置点的临时指针
	int n;
	IplImage* tmp;                 //图像临时指针
	static int pressed = 0;        //按压位状态，0代表没有左键按压，1代表左键按压

	int height=p->orig_img->height;    //图像的高度

	/* on left button press, remember first corner of rectangle around object */
	if( event == CV_EVENT_LBUTTONDOWN )          //鼠标左键下压，记住目标物体的第一个点的位置
	{
		n = p->n;                            //记下目标数量多少
		if( n == MAX_OBJECTS )               //是否达到最大数量，是则返回
			return;
		loc = p->loc1;                       //loc赋值为保存所有目标的起始点的指针loc1
		loc[n].x = x;                        //记下当前鼠标坐标x，y
		loc[n].y = y;
		pressed = 1;                         //按压位记为1
	}

	/* on mouse move with left button down, draw rectangle as defined in white */
	else if( event == CV_EVENT_MOUSEMOVE  && (flags & CV_EVENT_FLAG_LBUTTON ))  //鼠标移动以及 flags代表鼠标按下的拖拽事件以及左键拖拽
	{
		n = p->n;                            //同样检测目标个数
		if( n == MAX_OBJECTS )
			return;
		tmp = (IplImage *)cvClone( p->orig_img );    //将原始图像副本给临时图像
		loc = p->loc1;                               //将所有目标的起始点的指针赋给临时变量
		cvRectangle( tmp, loc[n], cvPoint(x, y), CV_RGB(255,255,255), 1, 8, 0 );  //在临时副本图像上画出该目标的矩形
		cvShowImage( p->win_name, tmp );             //显示该临时副本
	}  

	/* on left button up, finalize the rectangle and draw it in black */
	else if( event == CV_EVENT_LBUTTONUP )    //鼠标左键抬起，
	{
		n = p->n;
		if( n == MAX_OBJECTS )
			return;
		loc = p->loc2;
		loc[n].x = x;
		loc[n].y = y;
		cvRectangle( p->orig_img, p->loc1[n], loc[n], CV_RGB(255,0,0), 1, 8, 0 );
		cvShowImage( p->win_name, p->orig_img );
		pressed = 0;
		p->n++;
	}
}


#else

int main()
{
	float totalDist = 0;
	int FrameNum=1;
#ifdef VIDEO
	VideoCapture capture("../car.avi");
	capture>>curframe;
	namedWindow("meanshift",0);
	imshow("meanshift",curframe);
	if(curframe.empty()) return -1;
#endif

	uchar key = false;      //用来设置暂停
	float rho_v;//表示相似度
	int distx,disty;//用来记录输入与输出目标的距离
	int count = 0; //用来记录输入与输出目标跟踪相近的次数，当次数大于某个阈值时，说明目标跟踪丢失。
	//PP ret;

	MeanshiftTracker T;
#ifdef VIDEO
	while(true)
	{	
		capture>>curframe; //抓取一帧
		FrameNum ++;
		if(curframe.empty())break;

		if(selectRegion)//第一帧时初始化过程   如果开启了选择目标区域功能，进行手动选择
		{ 
			Rect* regions;

			Wid = curframe.cols;                    //图像大小
			Hei = curframe.rows; 
			img = new unsigned char [Wid * Hei * 3];  //定义变量存储 图像三个通道的信息

			int valide=get_regions(&regions);         //调用函数，通过鼠标获取目标区域矩形
			if(!valide) {printf("select no regions\n");break;}

			Rect r = regions[0];			                   //只提取第一个目标作为跟踪对象
			int centerX = r.x+r.width/2;//得到目标中心点
			int centerY = r.y+r.height/2;

			WidIn = r.width/2;          //得到目标半宽与半高，中心x，y坐标，作为meanshift的跟踪初始位置
			HeiIn = r.height/2;
			xin = centerX;
			yin = centerY;

			MatToImg(curframe,Wid,Hei);    //将图像curframe信息由Ipl格式存为unsigned char * img格式，脱离opencv库

			T.Initial_MeanShift_tracker(centerX,centerY,WidIn,HeiIn,img,Wid,Hei,1.0/FRAME_RATE);  //利用img和初始目标信息，初始化跟踪目标

			track = true;                  //开启跟踪
			selectRegion = false;          //关闭选择区域功能
			totalDist = 0;                 //总距离为零

			free(regions);
			//continue;
		}
		else
			MatToImg(curframe,Wid,Hei);	 //将图像curframe信息由Ipl格式存为unsigned char * img格式，脱离opencv库	

		if(track)
		{
			double start,end;
			start=cvGetTickCount();
			/* 跟踪一帧，输入前一帧的位置，跟踪得到后一帧的位置*/
			rho_v = T.MeanShift_tracker( xin, yin, WidIn, HeiIn, img, Wid, Hei, xout, yout, WidOut, HeiOut );
			end=cvGetTickCount();
			printf("use time:%.2fms\n",(end-start)/((double)cvGetTickFrequency()*1000));

			/* 画框: 新位置为蓝框 */
			rectangle(curframe,Point(xout - WidOut, yout - HeiOut),Point(xout+WidOut,yout+HeiOut),Scalar(255,0,0),2,8,0);
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
					count =0;
					printf("****************missing target!\n");
				}
				else
				{
					distx=abs(xin-xout);
					disty =abs(yin-yout);
					if(distx<1&&disty<1)
					{
						count = count+1;
					}
					xin = xout; yin = yout;
					WidIn = WidOut; HeiIn = HeiOut;
				}				
			}
            cout<<FrameNum<<":\t"<<rho_v<<endl;  
			totalDist += rho_v;
		}else waitKey(10);

#else

#endif

        
	    imshow("meanshift",curframe);

	    key = waitKey(10);
	    if(key == 27) break;                                       //ESC 推出程序
	    else if(key == ' ')                                       //暂停，并开启重新选择区域功能
	    {  selectRegion = false; pause = true;}
		else if(key=='c')
		{  selectRegion = true; pause = false;}

		while(pause){
			key=waitKey(10);
			if(key==' ') {  selectRegion = false; pause = false;}
			else if(key=='c') {pause=false;selectRegion=true;}
		}

		
  }

    cout<<"average dist:"<<totalDist/(FrameNum-SKIP_FRAME_COUNT);
    system("pause");
	return 0;
}
void MatToImg(Mat src,int w,int h,)
{
	for (int j = 0; j < h; j++ )      // 转成正向图像
		for (int i = 0; i < w; i++ )
		{
			img[ ( j*w+i )*3 ] = MR(src,i,j);
			img[ ( j*w+i )*3+1 ] = MG(src,i,j);
			img[ ( j*w+i )*3+2 ] = MB(src,i,j);
		}
}
int get_regions(Rect** regions)
{
	params p;
	Rect* r;
	int i,x1,y1,x2,y2,w,h;
	p.n=0;
	//imshow("meanshift",curframe);
	setMouseCallback("meanshift",mouse,&p);
	while(cvWaitKey(0) != ' ');

	/* extract regions defined by user; store as an array of rectangles */
	if( p.n == 0 )
	{
		*regions = NULL;
		return 0;
	}
	r = (Rect *)malloc( p.n * sizeof( Rect ) );
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
		r[i] = Rect( x1, y1, w, h );    //define one of the rects
	}
	*regions = r;
	return p.n;
}
void mouse(int event,int x,int y,int flags,void *param)
{
  if(selectRegion==true)
 {
	params* p = (params*)param;
	Point* loc;
	Point* loc2;
	int n;
    Mat tmp;
	static int pressed = 0;

	/* on left button press, remember first corner of rectangle around object */
	if( event == CV_EVENT_LBUTTONDOWN )
	{
		n = p->n;
		if( n == MAX_OBJECTS )
		{printf("exceed max select_object!\n");return;}
		loc = p->loc1;
		loc[n].x = x;
		loc[n].y = y;
		pressed = 1;
	}
/* on mouse move with left button down, draw rectangle as defined in white */
	else if( event == CV_EVENT_MOUSEMOVE  &&  flags & CV_EVENT_FLAG_LBUTTON )
	{
		n = p->n;
		if( n == MAX_OBJECTS )
			return;
		tmp = curframe.clone();
		loc = p->loc1;
		loc2= p->loc2;
		if(loc[n].x!=x&&loc[n].y!=y)
		{
			for(int i=0;i<n;i++) rectangle( tmp, loc[i], loc2[i],Scalar(255,0,0),2);
           rectangle( tmp, loc[n], Point(x, y),Scalar(255,0,0),2);
		}
		imshow("meanshift",tmp);
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
		pressed = 0;
		p->n++;
	}
  }
}


#endif