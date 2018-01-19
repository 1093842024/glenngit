#include "core/core.hpp"      
#include "highgui/highgui.hpp"      
#include "imgproc/imgproc.hpp"  
#include "video/tracking.hpp"  
#include<iostream>      

using namespace cv;      
using namespace std;      

Mat image;    
Mat rectImage;  
Mat imageCopy; //���ƾ��ο�ʱ��������ԭͼ��ͼ��    
bool leftButtonDownFlag=false; //�����������Ƶ��ͣ���ŵı�־λ    
Point originalPoint; //���ο����    
Point processPoint; //���ο��յ�    

Mat targetImageHSV;  
int histSize=200;    
float histR[]={0,255};    
const float *histRange=histR;    
int channels[]={0,1};   
Mat dstHist;  
Rect rect;  
vector<Point> pt; //����Ŀ��켣  
void onMouse(int event,int x,int y,int flags ,void* ustc); //���ص�����    

int main(int argc,char*argv[])      
{      
	VideoCapture video("car.avi");    
	double fps=video.get(CV_CAP_PROP_FPS); //��ȡ��Ƶ֡��    
	double pauseTime=1000/fps; //���������м���    
	namedWindow("����ľͷ��",0);      
	setMouseCallback("����ľͷ��",onMouse);    
	while(true)    
	{    
		if(!leftButtonDownFlag) //�ж�������û�а��£���ȡ������Ƶ��������ͣ    
		{    
			video>>image;    
			if(image.empty()) break;
		}    
		if(!image.data||waitKey(pauseTime)==27)  //ͼ��Ϊ�ջ�Esc�������˳�����    
		{    
			break;    
		}   

		double start,end;
		start=cvGetTickCount();
		if(originalPoint!=processPoint&&!leftButtonDownFlag)    
		{   
			Mat imageHSV;  
			Mat calcBackImage;  
			cvtColor(image,imageHSV,CV_RGB2HSV);  
			calcBackProject(&imageHSV,2,channels,dstHist,calcBackImage,&histRange);  //����ͶӰ  
			TermCriteria criteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 0.001);    
			meanShift(calcBackImage, rect, criteria);     
			Mat imageROI=imageHSV(rect);   //����ģ��             
			targetImageHSV=imageHSV(rect);  
			calcHist(&imageROI, 2, channels, Mat(), dstHist, 1, &histSize, &histRange);    
			normalize(dstHist, dstHist, 0.0, 1.0, NORM_MINMAX);   //��һ��  
			rectangle(image, rect, Scalar(255, 0, 0),3);    //Ŀ�����    
			pt.push_back(Point(rect.x+rect.width/2,rect.y+rect.height/2));  
			/*for(int i=0;i<pt.size()-1;i++)  
			{  
				line(image,pt[i],pt[i+1],Scalar(0,255,0),2.5);  
			}*/  
		} 
		end=cvGetTickCount();
		printf("use time:%.2fms\n",(end-start)/((double)cvGetTickFrequency()*1000));

		imshow("����ľͷ��",image);   
		//waitKey(100);  
	}  
	system("pause");
	return 0;    
}      

//*******************************************************************//      
//���ص�����      
void onMouse(int event,int x,int y,int flags,void *ustc)      
{     
	if(event==CV_EVENT_LBUTTONDOWN)      
	{      
		leftButtonDownFlag=true; //��־λ    
		originalPoint=Point(x,y);  //����������µ�ľ������    
		processPoint=originalPoint;    
	}      
	if(event==CV_EVENT_MOUSEMOVE&&leftButtonDownFlag)      
	{      
		imageCopy=image.clone();    
		processPoint=Point(x,y);    
		if(originalPoint!=processPoint)    
		{    
			//�ڸ��Ƶ�ͼ���ϻ��ƾ���    
			rectangle(imageCopy,originalPoint,processPoint,Scalar(255,0,0),2);    
		}    
		imshow("����ľͷ��",imageCopy);    
	}      
	if(event==CV_EVENT_LBUTTONUP)      
	{      
		leftButtonDownFlag=false;    
		rect=Rect(originalPoint,processPoint);        
		rectImage=image(rect); //��ͼ����ʾ    
		imshow("Sub Image",rectImage);        
		cvtColor(rectImage,targetImageHSV,CV_RGB2HSV);  
		imshow("targetImageHSV",targetImageHSV);  
		calcHist(&targetImageHSV,2,channels,Mat(),dstHist,1,&histSize,&histRange,true,false);         
		normalize(dstHist,dstHist,0,255,CV_MINMAX);  
		imshow("dstHist",dstHist);  
	}        
}     