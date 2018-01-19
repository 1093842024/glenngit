#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/objdetect/objdetect.hpp>
#include <fstream>
#include <stdio.h> 
#include <opencv2/ml/ml.hpp>
#include "opencv2/video/tracking.hpp"

#include <vector>    
using namespace cv;    
using namespace std;  


class MySVM : public CvSVM  
{  
public:  
	//获得SVM的决策函数中的alpha数组  
	double * get_alpha_vector()  
	{  
		return this->decision_func->alpha;  
	}  

	//获得SVM的决策函数中的rho参数,即偏移量  
	float get_rho()  
	{  
		return this->decision_func->rho;  
	}  
};  

int main(){
	
	MySVM svm;
	svm.load("SVM_DATA.xml");
	int DescriptorDim;//HOG描述子的维数，由图片大小、检测窗口大小、块大小、细胞单元中直方图bin个数决定 
	DescriptorDim = svm.get_var_count();//特征向量的维数，即HOG描述子的维数  
	int supportVectorNum = svm.get_support_vector_count();//支持向量的个数  
	//cout<<"支持向量个数："<<supportVectorNum<<endl;  

	Mat alphaMat = Mat::zeros(1, supportVectorNum, CV_32FC1);//alpha向量，长度等于支持向量个数  
	Mat supportVectorMat = Mat::zeros(supportVectorNum, DescriptorDim, CV_32FC1);//支持向量矩阵  
	Mat resultMat = Mat::zeros(1, DescriptorDim, CV_32FC1);//alpha向量乘以支持向量矩阵的结果  

	//将支持向量的数据复制到supportVectorMat矩阵中  
	for(int i=0; i<supportVectorNum; i++)  
	{  
		const float * pSVData = svm.get_support_vector(i);//返回第i个支持向量的数据指针  
		for(int j=0; j<DescriptorDim; j++)  
		{  
			//cout<<pData[j]<<" ";  
			supportVectorMat.at<float>(i,j) = pSVData[j];  
		}  
	}  

	//将alpha向量的数据复制到alphaMat中  

	double * pAlphaData = svm.get_alpha_vector();//返回SVM的决策函数中的alpha向量  
	for(int i=0; i<supportVectorNum; i++)  
	{  
		alphaMat.at<float>(0,i) = pAlphaData[i];  
	}  

	//计算-(alphaMat * supportVectorMat),结果放到resultMat中  
	//gemm(alphaMat, supportVectorMat, -1, 0, 1, resultMat);//不知道为什么加负号？  
	resultMat = -1 * alphaMat * supportVectorMat;  

	//得到最终的setSVMDetector(const vector<float>& detector)参数中可用的检测子  
	vector<float> myDetector;  
	//将resultMat中的数据复制到数组myDetector中  
	for(int i=0; i<DescriptorDim; i++)  
	{  
		myDetector.push_back(resultMat.at<float>(0,i));  
	}  
	//最后添加偏移量rho，得到检测子  
	myDetector.push_back(svm.get_rho());  
	//cout<<"检测子维数："<<myDetector.size()<<endl;  
	HOGDescriptor hog(Size(100,100),Size(10,10),Size(5,5),Size(5,5), 6);//需要固定
	hog.setSVMDetector(myDetector);
	
	cout<<"load complete"<<endl;

	/*
	std::vector<cv::Rect> regions;  
	Mat frame=imread("E:/handrec/4.jpg");
	resize(frame,frame,Size(640,480));
	hog.detectMultiScale(frame, regions, 0, cv::Size(8,8), cv::Size(32,32), 1.05); //可以修改
	for (size_t i = 0; i < regions.size(); i++)  
	{  
		cv::rectangle(frame, regions[i], cv::Scalar(0,0,255), 2);  
	}  
	cv::imshow("hog", frame);
	imwrite("E:/handrec/14.jpg",frame);
	waitKey();
	return 0;
*/
	
	std::vector<cv::Rect> regions;  
	
	VideoCapture cap("car.avi");
	int count=1;
	for (;;)
	{
		
		Mat frame;
		cap>>frame;
		resize(frame,frame,Size(320,240));
	
		if (count==1)
		{
			hog.detectMultiScale(frame, regions, 0, cv::Size(8,8), cv::Size(32,32), 1.05); //可以修改
			if(regions.size()==0) 
			{
				printf("no regions\n");
				break;
			}
			for (size_t i = 0; i < regions.size(); i++)  
			{  
				cv::rectangle(frame, regions[i], cv::Scalar(0,0,255), 2);  
			}  
			cv::imshow("hog", frame);
			imwrite("1.jpg",frame);

			count=0;
			waitKey(30);
		}	
		else{
		
			Mat image;
			bool backprojMode = false;
			bool selectObject = false;
			int trackObject = 0;
			bool showHist = true;
			Point origin;
			Rect selection,trackWindow;
			int vmin = 10, vmax = 256, smin = 30;

	 		Mat hsv, hue, mask, hist, histimg = Mat::zeros(480, 640, CV_8UC3), backproj;
	        bool paused = false;
			int hsize = 16;
			float hranges[] = {0,180};
			const float* phranges = hranges;
	         selection=regions[0];
		     frame.copyTo(image);

	
			    cvtColor(image, hsv, COLOR_BGR2HSV);
			
				inRange(hsv, Scalar(0, smin, vmin),
					Scalar(180, 256, vmax), mask);
				int ch[] = {0, 0};
				hue.create(hsv.size(), hsv.depth());
				mixChannels(&hsv, 1, &hue, 1, ch, 1);
				Mat roi(hue, selection), maskroi(mask, selection);
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
				normalize(hist, hist, 0, 255, CV_MINMAX);	
				 trackWindow = selection;

				calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
				backproj &= mask;
				RotatedRect trackBox = CamShift(backproj, trackWindow,
					TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
				if( trackWindow.area() <= 1 )
				{
					int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
					trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
						trackWindow.x + r, trackWindow.y + r) &
						Rect(0, 0, cols, rows);
				}

		ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
		imshow("hog",image);

		waitKey(30);
		//if(waitKey()=='s') continue;
			
		}

		}
	
	system("pause");
	return 0;
}