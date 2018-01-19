#ifndef _QRDECODER_H
#define _QRDECODER_H       
#include <iostream>   
#include <string.h>
using namespace std;        
 
//基本的二维码识别算法接口，速度快，但识别精度一般，恶劣条件识别率低
string QRDecoder(unsigned char* img,int cols,int rows);           
//优化加强的二维码识别算法接口，恶劣条件下速度稍慢，但识别精度很高           n代表间隔多少次库扫描方法后，进行优化方法扫描
string QRDecoder_improve1(unsigned char* img,int cols,int rows,int n);
 //升级版优化加强的二维码识别算法接口，恶劣条件下识别速度最慢，但识别精度最高 
string QRDecoder_improve2(unsigned char* img,int cols,int rows,int n);

string QRDecoder_improve(unsigned char* img,int cols,int rows,int n,int m);//该接口为improve1的修改版，速度比直接的库识别慢4倍
string QRDecoder_improve_ROI(unsigned char* img,int cols,int rows,int n,int m);//该接口为improve的修改版，对传入的图像进行抠图以后再识别

#endif