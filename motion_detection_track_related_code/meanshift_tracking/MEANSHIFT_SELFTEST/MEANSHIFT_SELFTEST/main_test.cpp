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


//����IplImage��ʽͼ��
#define B(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col*image->nChannels+0]
#define G(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col*image->nChannels+1]
#define R(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col*image->nChannels+2]
#define S(image,col,row) ((uchar*)(image->imageData+row*image->widthStep))[col]

IplImage *curframe=NULL;     //��ǰ֡ͼ���ָ��

#define MAX_OBJECTS 5
typedef struct params{           //��껭���Ŀ�������Ϣ�����԰������Ŀ����Ϣ
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

//����Mat��ʽͼ��
int dive=64;
#define MB(image,col,row) image.at<Vec3b>(row,col)[0]/dive*dive+dive/2
#define MG(image,col,row) image.at<Vec3b>(row,col)[1]/dive*dive+dive/2
#define MR(image,col,row) image.at<Vec3b>(row,col)[2]/dive*dive+dive/2

Mat curframe;

#define MAX_OBJECTS 5
typedef struct params{           //��껭���Ŀ�������Ϣ�����԰������Ŀ����Ϣ
	Point loc1[MAX_OBJECTS];
	Point loc2[MAX_OBJECTS];
	int n;
}params;

int get_regions(Rect**);
void mouse(int,int,int,int,void*);
void MatToImg(Mat src,int w,int h);

#endif


#define Num 10               //֡��ļ��
#define ai 0.08              //ѧϰ��
#define SKIP_FRAME_COUNT 10  //������֡��
#define FRAME_RATE 25        //֡��

bool pause=false;            //�Ƿ���ͣ
bool track=false;            //�Ƿ���׷��
bool selectRegion=true;      //�Ƿ�ѡ��Ŀ������

unsigned char* img;          //��ipl/matͼ��ת��Ϊchar *������ֲ����
int xin,yin;                 //����ʱ�����ͼ�����ĵ�
int WidIn,HeiIn;             //����ʱ�����ͼ��İ��Ͱ��
int xout,yout;               //����ʽ�����ͼ�����ĵ�
int WidOut,HeiOut;           //����ʽ�����ͼ��İ��Ͱ��
int Wid,Hei;                 //����ͼ��Ŀ�͸�



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

	uchar key=false;         //���������������
	float rho_v;             //��ʾ���ƶ�
	int distx,disty;         //������¼���������Ŀ��ľ���
	int count=0;               //������¼���������Ŀ���������Ĵ���������������ĳ����ֵʱ���ж�ΪĿ�궪ʧ
	
	MeanshiftTracker T;

#ifdef VIDEO 
	while(capture)
	{		
		curframe=cvQueryFrame(capture); //ץȡһ֡
		FrameNum ++;
		if(!curframe)
			break;

		if(selectRegion)//��һ֡ʱ��ʼ������   ���������ѡ��Ŀ�������ܣ������ֶ�ѡ��
		{ 
			CvRect* regions;

			Wid = curframe->width;                    //ͼ���С
			Hei = curframe->height; 
			img = new unsigned char [Wid * Hei * 3];  //��������洢 ͼ������ͨ������Ϣ


			int valide=get_regions(curframe, &regions);         //���ú�����ͨ������ȡĿ���������
			if(!valide) break;

			CvRect r = regions[0];			                   //ֻ��ȡ��һ��Ŀ����Ϊ���ٶ���
			int centerX = r.x+r.width/2;//�õ�Ŀ�����ĵ�
			int centerY = r.y+r.height/2;

			WidIn = r.width/2;          //�õ�Ŀ�������ߣ�����x��y���꣬��Ϊmeanshift�ĸ��ٳ�ʼλ��
			HeiIn = r.height/2;
			xin = centerX;
			yin = centerY;

			IplToImg(curframe,Wid,Hei);    //��ͼ��curframe��Ϣ��Ipl��ʽ��Ϊunsigned char * img��ʽ������opencv��

			T.Initial_MeanShift_tracker(centerX,centerY,WidIn,HeiIn,img,Wid,Hei,1.0/FRAME_RATE);  //����img�ͳ�ʼĿ����Ϣ����ʼ������Ŀ��

			track = true;                  //��������
			selectRegion = false;          //�ر�ѡ��������
			totalDist = 0;                 //�ܾ���Ϊ��

			free(regions);
			//continue;
		}
		else
			IplToImg(curframe,Wid,Hei);	 //��ͼ��curframe��Ϣ��Ipl��ʽ��Ϊunsigned char * img��ʽ������opencv��	

		if(track)
		{
			double start,end;
			start=cvGetTickCount();
			/* ����һ֡������ǰһ֡��λ�ã����ٵõ���һ֡��λ��*/
			rho_v = T.MeanShift_tracker( xin, yin, WidIn, HeiIn, img, Wid, Hei, xout, yout, WidOut, HeiOut );
			end=cvGetTickCount();
			printf("use time:%.2fms\n",(end-start)/((double)cvGetTickFrequency()*1000));

			/* ����: ��λ��Ϊ���� */
			cvRectangle(curframe,cvPoint(xout - WidOut, yout - HeiOut),cvPoint(xout+WidOut,yout+HeiOut),cvScalar(255,0,0),2,8,0);
			xin = xout; yin = yout;
			WidIn = WidOut; HeiIn = HeiOut;

			if ( rho_v < 0.8 )  /* �ж��Ƿ�Ŀ�궪ʧ */
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

	    while(pause)                                                 //�����ͣ�Ժ󣬿���ͨ��c����ѡ������
	    {
		  key = cvWaitKey(10);
		  if(key == 'c')  {  selectRegion = true;  pause = false; } //c��ͣ������������ѡ��������
		  else if(key == ' ') pause = false;                         //�ո���ͣ
	    }

	      key = cvWaitKey(10);
	      if(key == 27) break;                                       //ESC �Ƴ�����
	      else if(key == ' ')  pause = true;                         //�ո���ͣ
	      else if(key == 'c')                                        //c��ͣ������������ѡ��������
	      {  selectRegion = true; pause = false;}

    }

    cout<<"average dist:"<<totalDist/(FrameNum-SKIP_FRAME_COUNT);
	cvReleaseCapture(&capture);                 //ʹ��CvQueryFrameȡ��CvCapture* ÿ֡ͼ�����ֻ���ͷ�CvCapture*������Ҫ�ͷ�IplImage
	cvDestroyAllWindows();                      //ʹ��cvCreateImage()�½�һ��IplImage*���Լ�ʹ��cvCreateMat()�½�һ��CvMat*������ҪcvReleaseImage()  cvReleaseMat()��ʾ���ͷ�
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
	char* win_name="select region";  //������ʾ��ǵĴ���
	params p;                        //�����洢�����Ϣ�����ݽṹ
	CvRect* r;                       //�����������������ָ��
	int x1,y1,x2,y2,w,h;

	p.win_name=win_name;                    //��Ǵ��ڵ�����
	p.orig_img=(IplImage*)cvClone(frame);   //��Ǵ��ڵ�ԭʼͼ�񸱱� ������ĵ�ǰͼ��
	p.n=0;                                  //��Ǵ��ڵı������ ��ʼΪ0
	cvNamedWindow(win_name,1);              //�򿪱�Ǵ���
	cvShowImage(win_name,frame);
	cvSetMouseCallback(win_name,&mouse,&p);     //�����������Ļص�����mouse������������Ϣ����p��
	while(cvWaitKey(0) != ' ');                 //���ո����������
	cvDestroyWindow(win_name);                  //���ٱ�Ǵ���
	cvReleaseImage(&(p.orig_img));              //�ͷ�ԭʼͼ�񸱱�


	if(p.n==0){                                //����ǵ�Ŀ����������
		*regions=NULL;
		return 0;
	}
	r=(CvRect*)malloc(p.n*sizeof(CvRect));     //����洢Ŀ������������ݵ��ڴ�ռ�=����*��С

	for(int i=0;i<p.n;i++)               //�������е�Ҫ�������ľ��ο�
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

	*regions=r;                                //��������Ϣ����regions
	return p.n;                                //������������

}

void mouse(int event,int x,int y, int flags, void* param)
{
	params* p = (params*)param;    //������Ĳ�����Ϣ��������ָ��
	CvPoint* loc;                  //����Ŀ��λ�õ����ʱָ��
	int n;
	IplImage* tmp;                 //ͼ����ʱָ��
	static int pressed = 0;        //��ѹλ״̬��0����û�������ѹ��1���������ѹ

	int height=p->orig_img->height;    //ͼ��ĸ߶�

	/* on left button press, remember first corner of rectangle around object */
	if( event == CV_EVENT_LBUTTONDOWN )          //��������ѹ����סĿ������ĵ�һ�����λ��
	{
		n = p->n;                            //����Ŀ����������
		if( n == MAX_OBJECTS )               //�Ƿ�ﵽ������������򷵻�
			return;
		loc = p->loc1;                       //loc��ֵΪ��������Ŀ�����ʼ���ָ��loc1
		loc[n].x = x;                        //���µ�ǰ�������x��y
		loc[n].y = y;
		pressed = 1;                         //��ѹλ��Ϊ1
	}

	/* on mouse move with left button down, draw rectangle as defined in white */
	else if( event == CV_EVENT_MOUSEMOVE  && (flags & CV_EVENT_FLAG_LBUTTON ))  //����ƶ��Լ� flags������갴�µ���ק�¼��Լ������ק
	{
		n = p->n;                            //ͬ�����Ŀ�����
		if( n == MAX_OBJECTS )
			return;
		tmp = (IplImage *)cvClone( p->orig_img );    //��ԭʼͼ�񸱱�����ʱͼ��
		loc = p->loc1;                               //������Ŀ�����ʼ���ָ�븳����ʱ����
		cvRectangle( tmp, loc[n], cvPoint(x, y), CV_RGB(255,255,255), 1, 8, 0 );  //����ʱ����ͼ���ϻ�����Ŀ��ľ���
		cvShowImage( p->win_name, tmp );             //��ʾ����ʱ����
	}  

	/* on left button up, finalize the rectangle and draw it in black */
	else if( event == CV_EVENT_LBUTTONUP )    //������̧��
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

	uchar key = false;      //����������ͣ
	float rho_v;//��ʾ���ƶ�
	int distx,disty;//������¼���������Ŀ��ľ���
	int count = 0; //������¼���������Ŀ���������Ĵ���������������ĳ����ֵʱ��˵��Ŀ����ٶ�ʧ��
	//PP ret;

	MeanshiftTracker T;
#ifdef VIDEO
	while(true)
	{	
		capture>>curframe; //ץȡһ֡
		FrameNum ++;
		if(curframe.empty())break;

		if(selectRegion)//��һ֡ʱ��ʼ������   ���������ѡ��Ŀ�������ܣ������ֶ�ѡ��
		{ 
			Rect* regions;

			Wid = curframe.cols;                    //ͼ���С
			Hei = curframe.rows; 
			img = new unsigned char [Wid * Hei * 3];  //��������洢 ͼ������ͨ������Ϣ

			int valide=get_regions(&regions);         //���ú�����ͨ������ȡĿ���������
			if(!valide) {printf("select no regions\n");break;}

			Rect r = regions[0];			                   //ֻ��ȡ��һ��Ŀ����Ϊ���ٶ���
			int centerX = r.x+r.width/2;//�õ�Ŀ�����ĵ�
			int centerY = r.y+r.height/2;

			WidIn = r.width/2;          //�õ�Ŀ�������ߣ�����x��y���꣬��Ϊmeanshift�ĸ��ٳ�ʼλ��
			HeiIn = r.height/2;
			xin = centerX;
			yin = centerY;

			MatToImg(curframe,Wid,Hei);    //��ͼ��curframe��Ϣ��Ipl��ʽ��Ϊunsigned char * img��ʽ������opencv��

			T.Initial_MeanShift_tracker(centerX,centerY,WidIn,HeiIn,img,Wid,Hei,1.0/FRAME_RATE);  //����img�ͳ�ʼĿ����Ϣ����ʼ������Ŀ��

			track = true;                  //��������
			selectRegion = false;          //�ر�ѡ��������
			totalDist = 0;                 //�ܾ���Ϊ��

			free(regions);
			//continue;
		}
		else
			MatToImg(curframe,Wid,Hei);	 //��ͼ��curframe��Ϣ��Ipl��ʽ��Ϊunsigned char * img��ʽ������opencv��	

		if(track)
		{
			double start,end;
			start=cvGetTickCount();
			/* ����һ֡������ǰһ֡��λ�ã����ٵõ���һ֡��λ��*/
			rho_v = T.MeanShift_tracker( xin, yin, WidIn, HeiIn, img, Wid, Hei, xout, yout, WidOut, HeiOut );
			end=cvGetTickCount();
			printf("use time:%.2fms\n",(end-start)/((double)cvGetTickFrequency()*1000));

			/* ����: ��λ��Ϊ���� */
			rectangle(curframe,Point(xout - WidOut, yout - HeiOut),Point(xout+WidOut,yout+HeiOut),Scalar(255,0,0),2,8,0);
			xin = xout; yin = yout;
			WidIn = WidOut; HeiIn = HeiOut;

			if ( rho_v < 0.8 )  /* �ж��Ƿ�Ŀ�궪ʧ */
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
	    if(key == 27) break;                                       //ESC �Ƴ�����
	    else if(key == ' ')                                       //��ͣ������������ѡ��������
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
	for (int j = 0; j < h; j++ )      // ת������ͼ��
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