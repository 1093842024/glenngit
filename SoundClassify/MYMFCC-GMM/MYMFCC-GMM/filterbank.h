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
#define NUMCHANS 40     //�˲������ά��
#define FRAMESIZE 512   //֡��
#define FRAMESHIFT 256  //֡��
#define MFCCCOEF 13     //MFCCsά��
#define PREEMPHA 0.97  //Ԥ����ϵ��
#define SAMPLEPERIOD 625   //1024?  /   625?

#define LOWSHORTENERGY 2.0
#define HIGHSHORTENERGY 2.9
#define LOWZERORATE 3.5
#define HIGHZERORATE 7.5

#define PROCESSBYTEORIGIN

#define RESERVESAMPLEDOT 10000000  //������һ�δ���3���ӵ���Ƶ
//#define RDATA


typedef struct Wave  //����wav���ݵĻ�����Ϣ����ȡ�����������Ϣ
{
	int nSample;//wav���������������
	int frSize;//һ֡�е�������
    int nRow;        //wav������֡�ĸ���
	int frIdx;//������֡��������е�ǰ֡����ʼλ��
	int frRate;//֡��
	float *ShortEnergy;   //���ж�ʱ��������
	float *Zerorate;      //���ж�ʱƽ������������
    float *MFCCs;   //����13άMFCC����
	float *DevMFCCs;  //����13άһ�ײ��MFCC����  ����ÿһά��ǰ��֡�ͺ���֡����Ϊ0
    float *wavdata;  //wav�����в����������
	float *Rdata;   //fbank�������
	int *valideframe; //ͳ����Ч����֡��1��Ч��0��Ч
};
typedef struct FBankInfo    //fft�任�Լ�mel�˲�����������Ϣ
{
	int frameSize;       /* speech frameSize */ //֡�ߴ�
	int numChans;        /* number of channels */ //ͨ����
	long sampPeriod;     /* sample period */      //��������
	int fftN;            /* fft size */           //fft�ߴ�
	int klo, khi;         /* lopass to hipass cut-off fft indices */    //��ͨ���ͨ�׶�fft����  kloΪ2 khiΪfftN/2
	int usePower;    /* use power rather than magnitude *///boolen      //ʹ�����������Ƿ���   ������InitFBank�иò���Ϊ0 ��ʹ��
	int takeLogs;    /* log filterbank channels *///boolen              //log  �˲���ͨ��
	float fres;          /* scaled fft resolution */                   //��һ��fft�ֱ���
	float *cf;           /* array[1..pOrder+1] of centre freqs */      //����Ƶ�ʵ�������1....porder+1��
	float *loChan;     /* array[1..fftN/2] of loChan index */          //lochanĿ¼��ָ�� ��1...fftN/2��
	float *loWt;         /* array[1..fftN/2] of loChan weighting */    //lochanȨ�ص�ָ�� ��1...fftN/2��
	float *x;            /* array[1..fftN] of fftchans */              //fftͨ����ָ��    ��1...fftN��
};
typedef struct IOConfig     //��ǰһ֡���ݵ���Ϣ
{
	float curVol;/* current volume dB (0.0-100.0) */  //��ǰ�������ֱ� ��֡��ƽ������
	float shortenergy;                                 //��֡�Ķ�ʱ����
	float zerorate;                                    //֡�Ķ�ʱƽ��������
	float preEmph;    //Ԥ����ϵ�� ������Ϊ0.97
	int frSize;//һ֡�е�������
	int frIdx;//��ǰ֡λ��
	int frRate;//֡��
	float *fbank;       //�洢mel�˲������˲����������Ϣ��fbank[0]�洢�˲�����ά��
	float *mfcc;        //�洢һ֡��13άmfcc
	struct FBankInfo fbInfo;
	float *s;//֡����    ֡�ľ�����ֵ���ڴ˲�����

};
/*��ȡPCM�ļ���wavdata��*/
void LoadFile(const char *s, struct Wave *w);    //��Ӧ���������w->wavedata
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
void ConvertFrame(struct IOConfig *cf, struct Wave *w,const vector<float> &ham);            //�ڴ˺����д���֡����
void linkdata(struct IOConfig *cf, struct Wave *w, int k);
void zeromean(struct Wave *w);
void calcu_mfcc_diff(struct Wave *w);
struct Wave* filter_bank(const char *s,const int soundtype=0,const int len=0);
void deletewav(struct Wave* w);

int AudioBeginDetect(unsigned char* wave,int len=1024, float lowshortenegy=2.0, float lowzerorate=3.5);   //PCʵʱ������0.1ms�������һ��

#endif 