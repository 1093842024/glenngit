/***************************************************************************
Module Name:
	KMeans

History:
	2003/10/16	Fei Wang
	2013 luxiaoxun
***************************************************************************/

#pragma once
#include <fstream>
#include<assert.h>
#include <string.h>
class KMeans
{
public:
	enum InitMode                       //��ʼ��ģʽ ���ģʽ���ֶ�ģʽ��ͳһģʽ
	{
		InitRandom,
		InitManual,
		InitUniform,
	};

	KMeans(int dimNum = 1, int clusterNum = 1);    //���캯�����������ݵ�ά�ȣ�������������
	~KMeans();

	void SetMean(int i, const double* u){ memcpy(m_means[i], u, sizeof(double) * m_dimNum); }   //���þ�ֵ
	void SetInitMode(int i)				{ m_initMode = i; }                                     //����k-means��ʼ��ģʽ
	void SetMaxIterNum(int i)			{ m_maxIterNum = i; }                                   //��������������
	void SetEndError(double f)			{ m_endError = f; }                                     //���ؽ�������

	double* GetMean(int i)	{ return m_means[i]; }            //��õ�i������ľ�ֵ
	int GetInitMode()		{ return m_initMode; }            //��ó�ʼ��ģʽ
	int GetMaxIterNum()		{ return m_maxIterNum; }          //�������������
	double GetEndError()	{ return m_endError; }            //��ý�������


	/*	SampleFile: <size><dim><data>...
		LabelFile:	<size><label>...
	*/
	void Cluster(const char* sampleFileName, const char* labelFileName);        //�������ļ����ߴ硢ά�ȡ����ݣ� ����� ��ǩ�ļ����ߴ硢��ǩ��
	void Init(std::ifstream& sampleFile);                                       //������������ļ����г�ʼ��
	void Init(double *data, int N);                                             //�������Լ�������������Ϣ ���г�ʼ��
	void Cluster(double *data, int N, int *Label);                              //�����ݡ������������� ���о��࣬����������Ϣ��¼��label��
	friend std::ostream& operator<<(std::ostream& out, KMeans& kmeans);         //�������������ѵ���õ�����Ϣ�����ĵ���

private:
	int m_dimNum;         //ÿ������������ά��
	int m_clusterNum;     //�������ĵ�����
	double** m_means;     //һ������ָ�룬�洢һ����ά���飬��һά����������ͣ��ڶ�ά��������������ά�� ��m_means[m_clusterNum][m_dimNum]  ����ÿ�����ࣨm_clusterNum���ľ�ֵ���ݣ�m_dimNum��

	int m_initMode;                                                                //    
	int m_maxIterNum;		// The stopping criterion regarding the number of iterations ֹͣ׼������������
	double m_endError;		// The stopping criterion regarding the error                ֹͣ׼�򣺵������

	double GetLabel(const double* x, int* label);                                  //     ��ȡ�����ǩ
	double CalcDistance(const double* x, const double* u, int dimNum);             //     �������
};
