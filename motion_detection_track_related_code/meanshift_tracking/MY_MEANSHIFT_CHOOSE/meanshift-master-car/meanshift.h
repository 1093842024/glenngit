#pragma once
#include "meanshift.h"
#include "opencv2/opencv.hpp"
using cv::Mat;
using cv::Rect;
using std::cout;
using std::endl;
#define PI 3.1415926

class MeanShift
{
private:
	Rect _target_region;	//���ѡ�еľ���Ŀ����������Ŀ���ƶ����㷨ѭ�����У��᲻�ϸ��£�ע�⣺ֻ�Ǿ���������Rect���ͣ�����Mat��
	Mat _rec_binValue;		//_target_region�е�ÿ�����������ĸ�ֱ��ͼ����
	Mat _target_model;		//Ŀ��ģ�ͣ�������ʽ��2����3������õ���q��ֻ����һ�Σ���ͬ��_target_region������£���ֵ������֮��һֱ���䣬����ͬ������������������candidate_model�������������ƶ����ϼ���
	float _bin_width;		//ֱ��ͼÿ������Ŀ��
	int _frame_rows;		//���ɼ���Ƭ������������������������������Ƿ񳬹����ɼ���Ƭ�Ĵ�С����ֹ���������
	int _frame_cols;		//�ɼ���Ƭ������
	//����������⣬�����ٲ��伸�䣺Ini_target_frame�����У������ܻ��������Ϊ����������ͨ�����ѡȡ�ģ����������Ӧ����1�У�������
	//rec &= Rect(0, 0, (*image).cols, (*image).rows);��䣬���Է�֮��ѡ���򳬹����ޣ���ˣ���������ľ����ڳ������й����У���������
	//������Ƭ����ƶ���ͬʱ�ַ�����Ƭ�е�����ֵ�����ʱ�򣬾ͻ���������ⷢ������ˣ����ܷ�������ĵط�ֻ����Щ�����������frame��
	//�ж�����з��ʵĵط���Ϊ��ֹ�������Щ�ط�Ӧ�ü��ϼ����ơ�����������ֻ����Calpdfmodel�����п��ܷ�������������Ȱ�����ͼƬ
	//˺����Ȼ���ж�����ʡ�
	struct config
	{
		int _num_bins;		//ֱ��ͼ���������
		int _piexl_range;	//����ֵ�ķ�Χ�����ֵ��
		int _Maxiter;		//����������
	}cfg;
public:
	bool _flag_overflow;	//�����־λ����Ϊpublic���ⲿ���Է���

	MeanShift();
	void Ini_target_frame(Mat& frame, Rect& rec);							//��ʼ�������ѡ���򣬲�����Calpdfmodel()��������Ŀ��ģ�ͣ�ֻ���������е���һ��
	float Epanechnikov_kernel(Mat& kernel);									//����ʽ��12������Epanechnikov�ˣ�kernel�Ĵ�С����ѡ��������һ��
	Mat Calpdfmodel(const Mat& frame, const Rect& rec);						//����ʽ��2����3������ģ�ͣ������Epanechnikov_kernel()����
	Mat Calweight(const Rect & rec, Mat target_model, Mat candidate_model);	//����ʽ��10������Ȩ��
	Rect MeanShift::track(Mat frame);										//���٣����ε��� Calpdfmodel()��Calweight()����
};