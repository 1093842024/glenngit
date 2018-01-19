#ifndef MNIST_H  
#define MNIST_H

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <opencv2/opencv.hpp>  

using namespace cv;
using namespace std;

//小端存储转换
int reverseInt(int i);

//读取image数据集信息
Mat read_mnist_image(const string fileName);

//读取label数据集信息
Mat read_mnist_label(const string fileName);

#endif

