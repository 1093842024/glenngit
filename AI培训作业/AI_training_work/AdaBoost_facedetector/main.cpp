#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  
#include <iomanip>
#include <iostream>  
#include "change_img_size.h"
#include <vector>
using namespace cv;  
using namespace std;  


void detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade,double scale);
int cascade();


//String cascadeName = "haarcascades/haarcascade_frontalface_alt2.xml";//人脸的训练数据
String cascadeName = "haarcascades/haarcascade_frontalface_alt_tree.xml";//人脸的训练数据

//String nestedCascadeName = "haarcascades/haarcascade_eye_tree_eyeglasses.xml";//人眼的训练数据
String nestedCascadeName = "haarcascades/haarcascade_eye.xml";//人眼的训练数据

//string mycascde="xml/haar_adaboost.xml";
string mycascde="cascade2.xml";

//#define CREATE

int main()
{

	//changeimgsize_neg();
	//changeimgsize_neg();
#ifdef CREATE

	//createimg();
	cascade();
#else

	Mat image;
	CascadeClassifier cascade, nestedCascade;//创建级联分类器对象
	double scale = 1.3;

	image = imread( "yale/s2.bmp", 1 );
	//image = imread( "lena.png", 1 );//读入lena图片
	//image = imread( "people.jpg", 1 );
	//image = imread("people_with_hands.png",1);
	namedWindow( "result", 1 );//opencv2.0以后用namedWindow函数会自动销毁窗口




	if( !cascade.load(mycascde /*cascadeName*/ ) )//从指定的文件目录中加载级联分类器
	{
		cerr << "ERROR: Could not load classifier cascade" << endl;
		return 0;
	}

	if( !nestedCascade.load( nestedCascadeName ) )
	{
		cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
		return 0;
	}

	if( !image.empty() )//读取图片数据不能为空
	{
		detectAndDraw( image, cascade, nestedCascade, scale );
		waitKey(0);
	}

#endif
	system("pause");
	return 0;
}


void detectAndDraw( Mat& img,CascadeClassifier& cascade, CascadeClassifier& nestedCascade,double scale)
{
	int i = 0;
	double t = 0;
	vector<Rect> faces;
	const static Scalar colors[] =  { CV_RGB(0,0,255),
		CV_RGB(0,128,255),
		CV_RGB(0,255,255),
		CV_RGB(0,255,0),
		CV_RGB(255,128,0),
		CV_RGB(255,255,0),
		CV_RGB(255,0,0),
		CV_RGB(255,0,255)} ;//用不同的颜色表示不同的人脸

	Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );//将图片缩小，加快检测速度

	cvtColor( img, gray, CV_BGR2GRAY );//因为用的是类haar特征，所以都是基于灰度图像的，这里要转换成灰度图像
	resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );//将尺寸缩小到1/scale,用线性插值
	equalizeHist( smallImg, smallImg );//直方图均衡

	t = (double)cvGetTickCount();//用来计算算法执行时间
	//检测人脸
	//detectMultiScale函数中smallImg表示的是要检测的输入图像为smallImg，faces表示检测到的人脸目标序列，1.1表示
	//每次图像尺寸减小的比例为1.1，2表示每一个目标至少要被检测到3次才算是真的目标(因为周围的像素和不同的窗口大
	//小都可以检测到人脸),CV_HAAR_SCALE_IMAGE表示不是缩放分类器来检测，而是缩放图像，Size(30, 30)为目标的
	//最小最大尺寸
	cascade.detectMultiScale( smallImg, faces,1.1, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		|CV_HAAR_SCALE_IMAGE,Size(30, 30) ); 
	t = (double)cvGetTickCount() - t;//相减为算法执行的时间
	printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
	for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
	{
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i%8];
		int radius;
		center.x = cvRound((r->x + r->width*0.5)*scale);//还原成原来的大小
		center.y = cvRound((r->y + r->height*0.5)*scale);
		radius = cvRound((r->width + r->height)*0.25*scale);
		circle( img, center, radius, color, 3, 8, 0 );
		/*
		//检测人眼，在每幅人脸图上画出人眼
		if( nestedCascade.empty() )
			continue;
		smallImgROI = smallImg(*r);
		//和上面的函数功能一样
		nestedCascade.detectMultiScale( smallImgROI, nestedObjects,1.1, 2, 0
			//|CV_HAAR_FIND_BIGGEST_OBJECT
			//|CV_HAAR_DO_ROUGH_SEARCH
			//|CV_HAAR_DO_CANNY_PRUNING
			|CV_HAAR_SCALE_IMAGE ,Size(30, 30) );
		for( vector<Rect>::const_iterator nr = nestedObjects.begin(); nr != nestedObjects.end(); nr++ )
		{
			center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
			center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
			radius = cvRound((nr->width + nr->height)*0.25*scale);
			circle( img, center, radius, color, 3, 8, 0 );//将眼睛也画出来，和对应人脸的图形是一样的
		}
		*/
	}
	
	cv::imshow( "result", img );
}



int cascade()
{
	char*haartraining_ouput_dir ="xml"; //根据实际情况修改
	char*ouput_file ="xml/haar_adaboost.xml";//根据实际情况修改

	const char* size_opt = "--size=";
	char comment[1024];
	CvHaarClassifierCascade* cascade = 0;
	CvSize size;

	size.width= 24; //根据实际情况修改
	size.height= 24; //根据实际情况修改

	cascade= cvLoadHaarClassifierCascade(haartraining_ouput_dir,size);
	if(!cascade ){
		fprintf(stderr,"Inputcascade could not be found/opened\n");
		return-1;
	}
	sprintf(comment, "Automaticallyconverted from %s, window size = %dx%d",ouput_file, size.width, size.height );
	cvSave(ouput_file, cascade, 0, comment, cvAttrList(0,0) );
	return 0;
}
