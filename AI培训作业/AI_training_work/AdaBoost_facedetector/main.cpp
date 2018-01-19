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


//String cascadeName = "haarcascades/haarcascade_frontalface_alt2.xml";//������ѵ������
String cascadeName = "haarcascades/haarcascade_frontalface_alt_tree.xml";//������ѵ������

//String nestedCascadeName = "haarcascades/haarcascade_eye_tree_eyeglasses.xml";//���۵�ѵ������
String nestedCascadeName = "haarcascades/haarcascade_eye.xml";//���۵�ѵ������

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
	CascadeClassifier cascade, nestedCascade;//������������������
	double scale = 1.3;

	image = imread( "yale/s2.bmp", 1 );
	//image = imread( "lena.png", 1 );//����lenaͼƬ
	//image = imread( "people.jpg", 1 );
	//image = imread("people_with_hands.png",1);
	namedWindow( "result", 1 );//opencv2.0�Ժ���namedWindow�������Զ����ٴ���




	if( !cascade.load(mycascde /*cascadeName*/ ) )//��ָ�����ļ�Ŀ¼�м��ؼ���������
	{
		cerr << "ERROR: Could not load classifier cascade" << endl;
		return 0;
	}

	if( !nestedCascade.load( nestedCascadeName ) )
	{
		cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
		return 0;
	}

	if( !image.empty() )//��ȡͼƬ���ݲ���Ϊ��
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
		CV_RGB(255,0,255)} ;//�ò�ͬ����ɫ��ʾ��ͬ������

	Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );//��ͼƬ��С���ӿ����ٶ�

	cvtColor( img, gray, CV_BGR2GRAY );//��Ϊ�õ�����haar���������Զ��ǻ��ڻҶ�ͼ��ģ�����Ҫת���ɻҶ�ͼ��
	resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );//���ߴ���С��1/scale,�����Բ�ֵ
	equalizeHist( smallImg, smallImg );//ֱ��ͼ����

	t = (double)cvGetTickCount();//���������㷨ִ��ʱ��
	//�������
	//detectMultiScale������smallImg��ʾ����Ҫ��������ͼ��ΪsmallImg��faces��ʾ��⵽������Ŀ�����У�1.1��ʾ
	//ÿ��ͼ��ߴ��С�ı���Ϊ1.1��2��ʾÿһ��Ŀ������Ҫ����⵽3�β��������Ŀ��(��Ϊ��Χ�����غͲ�ͬ�Ĵ��ڴ�
	//С�����Լ�⵽����),CV_HAAR_SCALE_IMAGE��ʾ�������ŷ���������⣬��������ͼ��Size(30, 30)ΪĿ���
	//��С���ߴ�
	cascade.detectMultiScale( smallImg, faces,1.1, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		|CV_HAAR_SCALE_IMAGE,Size(30, 30) ); 
	t = (double)cvGetTickCount() - t;//���Ϊ�㷨ִ�е�ʱ��
	printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
	for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
	{
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i%8];
		int radius;
		center.x = cvRound((r->x + r->width*0.5)*scale);//��ԭ��ԭ���Ĵ�С
		center.y = cvRound((r->y + r->height*0.5)*scale);
		radius = cvRound((r->width + r->height)*0.25*scale);
		circle( img, center, radius, color, 3, 8, 0 );
		/*
		//������ۣ���ÿ������ͼ�ϻ�������
		if( nestedCascade.empty() )
			continue;
		smallImgROI = smallImg(*r);
		//������ĺ�������һ��
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
			circle( img, center, radius, color, 3, 8, 0 );//���۾�Ҳ���������Ͷ�Ӧ������ͼ����һ����
		}
		*/
	}
	
	cv::imshow( "result", img );
}



int cascade()
{
	char*haartraining_ouput_dir ="xml"; //����ʵ������޸�
	char*ouput_file ="xml/haar_adaboost.xml";//����ʵ������޸�

	const char* size_opt = "--size=";
	char comment[1024];
	CvHaarClassifierCascade* cascade = 0;
	CvSize size;

	size.width= 24; //����ʵ������޸�
	size.height= 24; //����ʵ������޸�

	cascade= cvLoadHaarClassifierCascade(haartraining_ouput_dir,size);
	if(!cascade ){
		fprintf(stderr,"Inputcascade could not be found/opened\n");
		return-1;
	}
	sprintf(comment, "Automaticallyconverted from %s, window size = %dx%d",ouput_file, size.width, size.height );
	cvSave(ouput_file, cascade, 0, comment, cvAttrList(0,0) );
	return 0;
}
