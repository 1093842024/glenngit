#include "mnist.h"

#include "opencv2/core/core.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/ml/ml.hpp"  

#include <string>
#include <iostream>

#include "NeuralNetworksFunctions.h"  

using namespace std;
using namespace cv;

string trainImage = "train-images.idx3-ubyte";
string trainLabel = "train-labels.idx1-ubyte";
string testImage = "t10k-images.idx3-ubyte";
string testLabel = "t10k-labels.idx1-ubyte";


//计时器
double cost_time_;
clock_t start_time_;
clock_t end_time_;

void EncodeOutputVector(Mat& src, Mat& out)
{
	for(int i=0;i<src.rows;i++)
	{
        int k=src.data[i];
		for(int j=0;j<10;j++)
		{
		    if(j==k) out.data[j]=1.0;
			else out.data[j]=0.0;
		}
	}
}

#define  TRAIN


int main()
{
    CvANN_MLP mlp;
	string model="mlp-ite=10-0.25-100-100hidden.xml";
#ifdef TRAIN

	//--------------------- 1. Set up training data ---------------------------------------
	Mat trainData;
	Mat labels;
	trainData = read_mnist_image(trainImage);
	labels = read_mnist_label(trainLabel);
	//cout <<"rows:"<< trainData.rows << "cols" << trainData.cols << endl;
	//cout <<"rows:"<< labels.rows << "cols" << labels.cols << endl;

	cv::Mat TrainingLabelVector(60000,10,CV_32FC1);  
	EncodeOutputVector(labels, TrainingLabelVector);  //将标签转化为10维的0-1编码

	// 神经网络有3层，第一层2个神经元，对应于二维的特征向量。第二层有
	// 6个神经元，第三层有3个神经元，对应分类问题中的3个类别
	Mat layerSizes=(Mat_<int>(1,4) << 784, 100,100, 10);
	CvANN_MLP_TrainParams params;
	// 神经网络的训练参数，在后面会详细解释
	params.term_crit = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 
		10, 0.001 );
	params.train_method = CvANN_MLP_TrainParams::BACKPROP;  
	params.bp_dw_scale = 0.25;  
	params.bp_moment_scale = 0.01;  

	// 创建神经网络
	mlp.create(layerSizes, CvANN_MLP::SIGMOID_SYM);

	// 训练神经网络
	mlp.train(trainData, TrainingLabelVector,  Mat(), Mat(), params);

	mlp.save(model.c_str());  

//#else

	mlp.load(model.c_str());  
	//------------------------ 6. read the test dataset -------------------------------------------
	cout << "开始导入测试数据...\n";
	Mat testData;
	Mat tLabel;
	testData = read_mnist_image(testImage);
	tLabel = read_mnist_label(testLabel);
	cout << "成功导入测试数据！！！\n";
	//cout <<"rows:"<< testData.rows << "cols" << testData.cols << endl;
	//cout <<"rows:"<< tLabel.rows << "cols" << tLabel.cols << endl;


	cv::Mat TestingLabelVector(10000,10,CV_32FC1);  
	EncodeOutputVector(tLabel, TestingLabelVector);  //将标签转化为10维的0-1编码

	Mat predictResult(10000, 10, CV_32FC1);
	mlp.predict(testData, predictResult); 

	int count=0;
	for(int i=0;i<predictResult.rows;i++)
	{
       float max1=predictResult.data[i*10+0];
	   float max2=TestingLabelVector.data[i*10+0];
	   int pos1=0;
	   int pos2=0;
	    for(int j=1;j<predictResult.cols;j++)
		{
			if(predictResult.data[i*10+j]>max1)
			{
				max1=predictResult.data[i*10+j];
				pos1=j;
			}
			if(TestingLabelVector.data[i*10+j]>max2)
			{
				max2=TestingLabelVector.data[i*10+j];
				pos2=j;
			}
		}
		if(pos1==pos2) count++;
	}
	
	cout << "正确的识别个数 count = " << count << endl;
	cout << "错误率为..." << (10000 - count + 0.0) / 10000 * 100.0 << "%....\n";

#endif
	system("pause");
	return 0;
}



/*
int main()
{
	CvANN_MLP Networks; 
#ifdef TRAIN

	//--------------------- 1. Set up training data ---------------------------------------
	Mat trainData;
	Mat labels;
	trainData = read_mnist_image(trainImage);
	labels = read_mnist_label(trainLabel);
	cout <<"rows:"<< trainData.rows << "cols" << trainData.cols << endl;
	cout <<"rows:"<< labels.rows << "cols" << labels.cols << endl;
	
	cv::Mat TrainingLabelVector(60000,10,CV_32FC1);  
	EncodeOutputVector(labels, TrainingLabelVector);  //将标签转化为10维的0-1编码


	// defining the network  
	 


	// The number of iteration  
	int MaxIte = 2;  

	NeuralNetworksTraing(Networks, trainData, TrainingLabelVector,  
		MaxIte);  
	// save the networks  
	Networks.save("NerualNetworks-ite=2-1000hidden.xml");  



#endif

#ifdef TEST

	//Networks.load("NerualNetworks-ite=1-30hidden.xml");
	//------------------------ 6. read the test dataset -------------------------------------------
	cout << "开始导入测试数据...\n";
	Mat testData;
	Mat tLabel;
	testData = read_mnist_image(testImage);
	tLabel = read_mnist_label(testLabel);
	cout << "成功导入测试数据！！！\n";
	cout <<"rows:"<< testData.rows << "cols" << testData.cols << endl;
	cout <<"rows:"<< tLabel.rows << "cols" << tLabel.cols << endl;
    int count=0;

	// predicting  
	cv::Mat Output(10000,1,CV_8UC1);  
	NeuralNetworksPredict(Networks, testData, Output);  //将预测的结果（10维的0-1编码）转化为一维的数字结果
	
	for(int i=0;i<10000;i++)
	{
	    if(Output.data[i]==tLabel.data[i]) count++;
		//printf("label:%d,output:%d\n",tLabel.data[i],Output.data[i]);
	}

	cout << "正确的识别个数 count = " << count << endl;
	cout << "错误率为..." << (10000 - count + 0.0) / 10000 * 100.0 << "%....\n";

#endif
	system("pause");
	return 0;
}
*/