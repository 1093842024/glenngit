/*
 *������ΪMeanshift�㷨���ٳ���
 *opencv3.1+vs14����opencv2411_vc12_x86��
 *�ο���Kernel-Based Object Tracking, Dorin Comaniciu
*/
#include "meanshift.h"
using cv::Mat;
using cv::Rect;
using cv::Point;
using std::cout;
using std::cin;
using std::endl;

Mat frame;					//����֡
Rect rec;					//�����ѡ���ľ���Ŀ������
bool selected = false;		//�����Ӧ������־λ����������������Ϊtrue����ʾѡ�����
bool draw_rec = false;		//�����Ӧ������־λ��������������ʼ��Ϊtrue����ʾ���������Կ�ʼ���ƾ�������
Point origin;				//��갴��ʱ�ĵ��λ�ã����ͼƬ���Ͻǣ�

void on_mouse1(int event, int x, int y, int, void*);	//���������Ӧ���������߹���һ��
void on_mouse2(int event, int x, int y, int, void*);
int main()
{
	//��������Ǵ���Ƶ���Ǵ�����ͷ��ȡͼƬ
	char src;
	cout << "The way get the frame(v:video/c:camera): ";
	cin >> src;

	cv::VideoCapture frame_cap;
	switch (src)
	{
	case 'v':
		//frame_cap = cv::VideoCapture("Homework_video.avi");
		//frame_cap = cv::VideoCapture("crowds.wmv");
		frame_cap = cv::VideoCapture("car.wmv");
		//frame_cap = cv::VideoCapture("dragonbaby.wmv");
		//frame_cap = cv::VideoCapture("tracking_video.avi");
		break;
	case 'c':
		//frame_cap = cv::VideoCapture(0);
		frame_cap.open(0);
		break;
	default:
		//cout << "Enter a error way!" << endl;
		frame_cap = cv::VideoCapture("Homework_video.avi");
		break;
	}
	
	if (!frame_cap.isOpened())
	{
		cout << "Failed in get the source of images!" << endl;
		return -1;
	}

	frame_cap >> frame;														//����Ƶ������֡
	Mat temp_img = frame.clone();

	cv::namedWindow("Happy-Meanshift", 1);
	cv::setMouseCallback("Happy-Meanshift", on_mouse1, (void*) &temp_img);	//���ûص�����
	//cv::setMouseCallback("Meanshift", on_mouse2, (void*)&temp_img);
	while (selected==false)
	{	
		Mat temp_img2 = temp_img.clone();
		//if (draw_rec && rec.width > 0 && rec.height > 0)
		if (draw_rec)
		{
			cv::rectangle(temp_img2, rec, cv::Scalar(0, 255, 0), 2);	//�������������̣߳��ص�������־λselected��draw_rec�������Ƿ���ƾ�������
		}		
		cv::imshow("Happy-Meanshift", temp_img2);
		//if (cv::waitKey(15) == 27)	
		//	break;
		cv::waitKey(15);		//�������������̱߳�־λ��ʱ���ƾ����������������º�������ƶ������ϻ��ƾ�������ֱ���������
	}

	//��ѡ���������������Ϊ�������У����ھ������ĵ�
	if (rec.height % 2 == 0)
		rec.height++;
	if (rec.width % 2 == 0)
		rec.width++;

	MeanShift ms;
	ms.Ini_target_frame(frame,rec);		//���ݵ�һ֡ͼ��ѡ��Ŀ��������򣬼���Ŀ������ģ�ͣ�ֻ����һ�Σ�����Ķ��Ǹ����Ŀ��ģ�����Ƚϣ�

	Rect next_rec;
	while (1)
	{
		if (!frame_cap.read(frame))
			break;
		next_rec=ms.track(frame);

		if (ms._flag_overflow) return -1;

		cv::rectangle(frame, next_rec, cv::Scalar(0, 255, 0), 2);
		cv::imshow("Happy-Meanshift", frame);
		cv::waitKey(25);
	}
}


//�����Ӧ����1
void on_mouse1(int event, int x, int y, int flag, void* param)
{
	Mat *image = (Mat*) param;

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin.x = x;
		origin.y = y;
		cout << origin.x << endl;
		rec = Rect(origin.x, origin.y, 0, 0);	//��������Ǳ���ģ���Ϊ����һ������draw_rec�����������оͻ�����ѡ�������Ҫ�Ծ��������ʼ������Ȼ������Ͻǣ�Ĭ��ԭ�㣩���ƾ���
		draw_rec = true;
		break;
	case CV_EVENT_MOUSEMOVE:
		if (draw_rec)
		{
			rec.x = MIN(origin.x, x);
			rec.y = MIN(origin.y, y);
			rec.width = std::abs(x-origin.x);
			rec.height = std::abs(y - origin.y);

			rec &= Rect(0, 0, (*image).cols, (*image).rows);		//���������������룬�õ�������������Ľ����������ɱ�֤��ѡ������ͼƬ��
		}
		break;
	case CV_EVENT_LBUTTONUP:
		//rec.x = MIN(origin.x, x);
		//rec.y = MIN(origin.y, y);		
		//cv::rectangle(*image, rec, cv::Scalar(0, 255, 0), 2);
		selected = true;
		draw_rec = false;		
		break;
	default:
		break;
	}
}

//�����Ӧ����2��ժ��20160222MeanShift_GitHub_Based_On_Paper��Ŀ����on_mouse1���Աȣ�
void on_mouse2(int event, int x, int y, int flag, void* param)
{
	cv::Mat *image = (cv::Mat*) param;
	switch (event) {
	case CV_EVENT_MOUSEMOVE:
		if (draw_rec) {
			rec.width = x - rec.x;
			rec.height = y - rec.y;
		}
		break;

	case CV_EVENT_LBUTTONDOWN:
		draw_rec = true;
		rec = cv::Rect(x, y, 0, 0);
		break;

	case CV_EVENT_LBUTTONUP:
		draw_rec = false;
		//��ʵ����ʹ��Rect��ȡROIʱ��Rect�Ŀ�͸߿����Ǹ��ģ����û�б�Ҫ����������֤
		//���⣬�����������֤�������ò�������Ϊ������߳�ִ����֮���ʹ��draw_rec=false,selected=true���������̶߳��Ѿ�ֹͣwhileѭ���ˣ�
		//����Ŀǰ����⣬���̱߳���ȫ��ִ����Ż�ص��������������CV_EVENT_LBUTTONUP�¼�ʱ������ִ�е�break�Ż�ָ�������
		//if (rec.width < 0) {
		//	rec.x += rec.width;
		//	rec.width *= -1;
		//}
		//if (rec.height < 0) {
		//	rec.y += rec.height;
		//	rec.height *= -1;
		//}
		//cv::rectangle(*image,rec,cv::Scalar(0),2);
		selected = true;
		break;
	}
}