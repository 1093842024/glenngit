#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  

#include <iostream>  
using namespace cv;  
using namespace std;  

int main(int argc, char **argv)
{
	const int kWidth = 512, kHeight = 512;
	Vec3b red(0, 0, 255), green(0, 255, 0), blue(255, 0, 0);
	Mat image = Mat::zeros(kHeight, kWidth, CV_8UC3);  

	// Ϊ3��ѵ�������ı�ǩ��ֵ
	float labels[150][3];
	for (int i  = 0 ; i < 50; i++)
	{
		labels[i][0] =  1.0f;
		labels[i][1] = -1.0f;
		labels[i][2] = -1.0f;
	}
	for (int i = 50; i < 100; i++)
	{
		labels[i][0] = -1.0f;
		labels[i][1] =  1.0f;
		labels[i][2] = -1.0f;
	}
	for (int i = 100; i < 150; i++)
	{
		labels[i][0] = -1.0f;
		labels[i][1] = -1.0f;
		labels[i][2] =  1.0f;
	}
	Mat trainResponse(150, 3, CV_32FC1, labels);

	// ����ѵ������������������
	float trainDataArray[150][2];
	RNG rng;
	for (int i = 0; i < 50; i++)
	{
		trainDataArray[i][0] = 250 + static_cast<float>(rng.gaussian(30));
		trainDataArray[i][1] = 250 + static_cast<float>(rng.gaussian(30));
	}

	for (int i = 50; i < 100; i++)
	{
		trainDataArray[i][0] = 150 + static_cast<float>(rng.gaussian(30));
		trainDataArray[i][1] = 150 + static_cast<float>(rng.gaussian(30));
	}

	for (int i = 100; i < 150; i++)
	{
		trainDataArray[i][0] = 320 + static_cast<float>(rng.gaussian(30));
		trainDataArray[i][1] = 150 + static_cast<float>(rng.gaussian(30));
	}

	Mat trainData(150, 2, CV_32FC1, trainDataArray);

	CvANN_MLP mlp;
	// ��������3�㣬��һ��2����Ԫ����Ӧ�ڶ�ά�������������ڶ�����
	// 6����Ԫ����������3����Ԫ����Ӧ���������е�3�����
	Mat layerSizes=(Mat_<int>(1,3) << 2, 10, 3);
	CvANN_MLP_TrainParams params;
	// �������ѵ���������ں������ϸ����
	params.term_crit = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 
		1000, 0.001 );
	params.train_method = CvANN_MLP_TrainParams::BACKPROP;  
	params.bp_dw_scale = 0.1;  
	params.bp_moment_scale = 0.1;  

	// ����������
	mlp.create(layerSizes, CvANN_MLP::SIGMOID_SYM);

	// ѵ��������
	mlp.train(trainData, trainResponse,  Mat(), Mat(), params);

	// ��ͼ�������е�(i,j )����Ԥ�⣬���ݷ�������ʾ��ͬ����ɫ
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			Mat sampleMat = (Mat_<float>(1, 2) << j, i); 	
			Mat predictResult(1, 3, CV_32FC1);
			// ��������Ԥ�⣬predictResult��Ԥ���������
			mlp.predict(sampleMat, predictResult); 

			Point maxLoc;
			double maxVal;
			// ѵ����������������maxVal�Ƿ��ص�������ֵ��maxLoc�����ֵ
			// ��Ӧ�ķ����ţ���������
			minMaxLoc(predictResult, 0, &maxVal, 0, &maxLoc);
			// ���ݷ�������ʾ��ͬ����ɫ
			if (maxLoc.x == 0)
				image.at<Vec3b>(i, j) = red;
			else if (maxLoc.x == 1)
				image.at<Vec3b>(i, j) = blue;
			else
				image.at<Vec3b>(i, j) = green;
		}
	}

	// ��ʾѵ������
	for (int i = 0; i < trainData.rows; i++)
	{
		const float* v = trainData.ptr<float>(i);
		Point pt = Point((int)v[0], (int)v[1]);
		if (labels[i][0] == 1)
			circle(image, pt, 5, Scalar::all(0), -1, 8); 
		else if (labels[i][1] == 1)
			circle(image, pt, 5, Scalar::all(128), -1, 8);
		else
			circle(image, pt, 5, Scalar::all(255), -1, 8);
	}

	imshow("MLP classifier demo", image);
	waitKey(0);
	return 0;
}