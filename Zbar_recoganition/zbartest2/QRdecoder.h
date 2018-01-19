#ifndef _QRDECODER_H
#define _QRDECODER_H       
#include <iostream>   
#include <string.h>
using namespace std;        
 
//�����Ķ�ά��ʶ���㷨�ӿڣ��ٶȿ죬��ʶ�𾫶�һ�㣬��������ʶ���ʵ�
string QRDecoder(unsigned char* img,int cols,int rows);           
//�Ż���ǿ�Ķ�ά��ʶ���㷨�ӿڣ������������ٶ���������ʶ�𾫶Ⱥܸ�           n���������ٴο�ɨ�跽���󣬽����Ż�����ɨ��
string QRDecoder_improve1(unsigned char* img,int cols,int rows,int n);
 //�������Ż���ǿ�Ķ�ά��ʶ���㷨�ӿڣ�����������ʶ���ٶ���������ʶ�𾫶���� 
string QRDecoder_improve2(unsigned char* img,int cols,int rows,int n);

string QRDecoder_improve(unsigned char* img,int cols,int rows,int n,int m);//�ýӿ�Ϊimprove1���޸İ棬�ٶȱ�ֱ�ӵĿ�ʶ����4��
string QRDecoder_improve_ROI(unsigned char* img,int cols,int rows,int n,int m);//�ýӿ�Ϊimprove���޸İ棬�Դ����ͼ����п�ͼ�Ժ���ʶ��

#endif