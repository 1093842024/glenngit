#ifndef __FILTERFBANK_H__
#define __FILTERFBANK_H__
#define INPUT_DIMEN 1640
#define LAYER_DIMEN 128
#define OUTPUT_DIMEN 3
#define LAYER 4

#include <vector>
#include<stdio.h>
#include<math.h>
#include <stdlib.h>
#include <time.h>
//#include "nNet.h"

#include <iostream>
#include <fstream>
//#include <bitset>
//#include <iomanip>
using namespace std;

#define PI   3.14159265358979
#define TPI  6.28318530717959     /* PI*2 */
#define NUMCHANS 40     //滤波器组的维数
#define FRAMESIZE 512   //帧长
#define FRAMESHIFT 256  //帧移
#define MFCCCOEF 13     //MFCCs维数
#define PREEMPHA 0.97  //预加重系数
#define SAMPLEPERIOD 625   //1024?  /   625?

#define LOWSHORTENERGY 2.0
#define HIGHSHORTENERGY 2.9
#define LOWZERORATE 3.5
#define HIGHZERORATE 7.5

#define PROCESSBYTEORIGIN

#define RESERVESAMPLEDOT 10000000  //最多可以一次处理3分钟的音频
//#define RDATA


typedef struct Wave  //整个wav数据的基本信息和提取储存的特征信息
{
	int nSample;//wav中样本采样点个数
	int frSize;//一帧中的样本数
    int nRow;        //wav样本的帧的个数
	int frIdx;//不断移帧处理过程中当前帧的起始位置
	int frRate;//帧移
	float *ShortEnergy;   //所有短时能量特征
	float *Zerorate;      //所有短时平均过零率特征
    float *MFCCs;   //所有13维MFCC数据
	float *DevMFCCs;  //所有13维一阶差分MFCC数据  其中每一维的前两帧和后两帧参数为0
    float *wavdata;  //wav中所有采样点的数据
	float *Rdata;   //fbank结果数据
	int *valideframe; //统计有效语音帧，1有效，0无效
};
typedef struct FBankInfo    //fft变换以及mel滤波器组的相关信息
{
	int frameSize;       /* speech frameSize */ //帧尺寸
	int numChans;        /* number of channels */ //通道数
	long sampPeriod;     /* sample period */      //采样周期
	int fftN;            /* fft size */           //fft尺寸
	int klo, khi;         /* lopass to hipass cut-off fft indices */    //低通与高通阶段fft因子  klo为2 khi为fftN/2
	int usePower;    /* use power rather than magnitude *///boolen      //使用能量而不是幅度   函数中InitFBank中该参数为0 不使用
	int takeLogs;    /* log filterbank channels *///boolen              //log  滤波组通道
	float fres;          /* scaled fft resolution */                   //归一化fft分辨率
	float *cf;           /* array[1..pOrder+1] of centre freqs */      //中心频率的向量【1....porder+1】
	float *loChan;     /* array[1..fftN/2] of loChan index */          //lochan目录的指针 【1...fftN/2】
	float *loWt;         /* array[1..fftN/2] of loChan weighting */    //lochan权重的指针 【1...fftN/2】
	float *x;            /* array[1..fftN] of fftchans */              //fft通道的指针    【1...fftN】
};
typedef struct IOConfig     //当前一帧数据的信息
{
	float curVol;/* current volume dB (0.0-100.0) */  //当前的音量分贝 即帧的平均能量
	float shortenergy;                                 //即帧的短时能量
	float zerorate;                                    //帧的短时平均过零率
	float preEmph;    //预加重系数 代码中为0.97
	int frSize;//一帧中的样本数
	int frIdx;//当前帧位置
	int frRate;//帧移
	float *fbank;       //存储mel滤波器组滤波后的特征信息，fbank[0]存储滤波器的维数
	float *mfcc;        //存储一帧的13维mfcc
	struct FBankInfo fbInfo;
	float *s;//帧数据    帧的具体数值存在此参数中

};
/*读取PCM文件到wavdata中*/
void LoadFile(const char *s, struct Wave *w);    //对应采样点读入w->wavedata
void LoadFile2(char *s, struct Wave *w);
void LoadFile3(char *s, struct Wave *w);
void LoadServerFile(const char *data, int len, struct Wave *w);
void GetWave(float *buf, struct Wave *w);
void ZeroMeanFrame(float *frame);
void PreEmphasise(float *frame, float k); 
void Ham(float *frame,const vector<float> &hamWin);
void initham(int frameSize,vector<float> &hamWin);
float Mel(int k, float fres);
float WarpFreq(float fcl, float fcu, float freq, float minFreq, float maxFreq, float alpha);
struct FBankInfo InitFBank(struct IOConfig *cf);
void FFT(float *s, int invert);
void Realft(float *s);
void Wave2FBank(float *s, float *fbank, struct FBankInfo info,float *mfcc);
void ConvertFrame(struct IOConfig *cf, struct Wave *w,const vector<float> &ham);            //在此函数中处理帧数据
void linkdata(struct IOConfig *cf, struct Wave *w, int k);
void zeromean(struct Wave *w);
void calcu_mfcc_diff(struct Wave *w);
struct Wave* filter_bank(const char *s,const int soundtype=0,const int len=0);
void deletewav(struct Wave* w);

int AudioBeginDetect(unsigned char* wave,int len=1024, float lowshortenegy=2.0, float lowzerorate=3.5);   //PC实时处理在0.1ms以内完成一次

#endif 