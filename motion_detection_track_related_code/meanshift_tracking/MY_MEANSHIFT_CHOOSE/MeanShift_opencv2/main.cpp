#include "core/core.hpp"      
#include "highgui/highgui.hpp"      
#include "imgproc/imgproc.hpp"  
#include "video/tracking.hpp"  
#include<iostream>      

using namespace cv;      
using namespace std;      

Mat image;    
Mat rectImage;  
Mat imageCopy; //绘制矩形框时用来拷贝原图的图像    
bool leftButtonDownFlag=false; //左键单击后视频暂停播放的标志位    
Point originalPoint; //矩形框起点    
Point processPoint; //矩形框终点    

Mat targetImageHSV;  
int histSize=200;    
float histR[]={0,255};    
const float *histRange=histR;    
int channels[]={0,1};   
Mat dstHist;  
Rect rect;  
vector<Point> pt; //保存目标轨迹  
void onMouse(int event,int x,int y,int flags ,void* ustc); //鼠标回调函数    

int main(int argc,char*argv[])      
{      
	VideoCapture video("car.avi");    
	double fps=video.get(CV_CAP_PROP_FPS); //获取视频帧率    
	double pauseTime=1000/fps; //两幅画面中间间隔    
	namedWindow("跟踪木头人",0);      
	setMouseCallback("跟踪木头人",onMouse);    
	while(true)    
	{    
		if(!leftButtonDownFlag) //判定鼠标左键没有按下，采取播放视频，否则暂停    
		{    
			video>>image;    
			if(image.empty()) break;
		}    
		if(!image.data||waitKey(pauseTime)==27)  //图像为空或Esc键按下退出播放    
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
			calcBackProject(&imageHSV,2,channels,dstHist,calcBackImage,&histRange);  //反向投影  
			TermCriteria criteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 0.001);    
			meanShift(calcBackImage, rect, criteria);     
			Mat imageROI=imageHSV(rect);   //更新模板             
			targetImageHSV=imageHSV(rect);  
			calcHist(&imageROI, 2, channels, Mat(), dstHist, 1, &histSize, &histRange);    
			normalize(dstHist, dstHist, 0.0, 1.0, NORM_MINMAX);   //归一化  
			rectangle(image, rect, Scalar(255, 0, 0),3);    //目标绘制    
			pt.push_back(Point(rect.x+rect.width/2,rect.y+rect.height/2));  
			/*for(int i=0;i<pt.size()-1;i++)  
			{  
				line(image,pt[i],pt[i+1],Scalar(0,255,0),2.5);  
			}*/  
		} 
		end=cvGetTickCount();
		printf("use time:%.2fms\n",(end-start)/((double)cvGetTickFrequency()*1000));

		imshow("跟踪木头人",image);   
		//waitKey(100);  
	}  
	system("pause");
	return 0;    
}      

//*******************************************************************//      
//鼠标回调函数      
void onMouse(int event,int x,int y,int flags,void *ustc)      
{     
	if(event==CV_EVENT_LBUTTONDOWN)      
	{      
		leftButtonDownFlag=true; //标志位    
		originalPoint=Point(x,y);  //设置左键按下点的矩形起点    
		processPoint=originalPoint;    
	}      
	if(event==CV_EVENT_MOUSEMOVE&&leftButtonDownFlag)      
	{      
		imageCopy=image.clone();    
		processPoint=Point(x,y);    
		if(originalPoint!=processPoint)    
		{    
			//在复制的图像上绘制矩形    
			rectangle(imageCopy,originalPoint,processPoint,Scalar(255,0,0),2);    
		}    
		imshow("跟踪木头人",imageCopy);    
	}      
	if(event==CV_EVENT_LBUTTONUP)      
	{      
		leftButtonDownFlag=false;    
		rect=Rect(originalPoint,processPoint);        
		rectImage=image(rect); //子图像显示    
		imshow("Sub Image",rectImage);        
		cvtColor(rectImage,targetImageHSV,CV_RGB2HSV);  
		imshow("targetImageHSV",targetImageHSV);  
		calcHist(&targetImageHSV,2,channels,Mat(),dstHist,1,&histSize,&histRange,true,false);         
		normalize(dstHist,dstHist,0,255,CV_MINMAX);  
		imshow("dstHist",dstHist);  
	}        
}     