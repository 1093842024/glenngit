#ifndef MNIST_H  
#define MNIST_H

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <opencv2/opencv.hpp>  

using namespace cv;
using namespace std;

//С�˴洢ת��
int reverseInt(int i);

//��ȡimage���ݼ���Ϣ
Mat read_mnist_image(const string fileName);

//��ȡlabel���ݼ���Ϣ
Mat read_mnist_label(const string fileName);

#endif

