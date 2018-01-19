/***************************************************************************
Module Name:
	Gaussian Mixture Model with Diagonal Covariance Matrix

History:
	2003/11/01	Fei Wang
	2013 luxiaoxun
***************************************************************************/

#pragma once
#include <fstream>
#include<assert.h>

class GMM
{
public:
	GMM(int dimNum = 1, int mixNum = 1);      //���캯������  ����������ά��  ��ϵĸ�˹ģ������
	~GMM();

	void Copy(GMM* gmm);  //����gmm����
	//�����㷨�ڲ�����
	void SetMaxIterNum(int i)	{ m_maxIterNum = i; }   
	void SetEndError(double f)	{ m_endError = f; }
	//��ȡ�㷨�ڲ�����
	int GetDimNum()			{ return m_dimNum; }
	int GetMixNum()			{ return m_mixNum; }
	int GetMaxIterNum()		{ return m_maxIterNum; }
	double GetEndError()	{ return m_endError; }

	double& Prior(int i)	{ return m_priors[i]; }
	double* Mean(int i)		{ return m_means[i]; }
	double* Variance(int i)	{ return m_vars[i]; }

	//����Ȩ�ء���ֵ������
	void setPrior(int i,double val)	{  m_priors[i]=val; }
	void setMean(int i,double *val)		{ for(int j=0;j<m_dimNum;j++) m_means[i][j]=val[j]; }
	void setVariance(int i,double *val)	{ for(int j=0;j<m_dimNum;j++) m_vars[i][j]=val[j]; }

	//��������ĵĸ�˹�����ܶ�ֵ
	double GetProbability(const double* sample);

	/*	SampleFile: <size><dim><data>...*/  //���ֳ�ʼ����˹ģ���Լ�ѵ����˹ģ�͵ķ�ʽ
    void Init(const char* sampleFileName);
	void Train(const char* sampleFileName);
	void Init(double *data, int N);
	void Train(double *data, int N);

	void DumpSampleFile(const char* fileName);

	friend std::ostream& operator<<(std::ostream& out, GMM& gmm);
	friend std::istream& operator>>(std::istream& in, GMM& gmm);

private:
	int m_dimNum;		// ����ά��
	int m_mixNum;		// Gaussian��Ŀ
	double* m_priors;	// GaussianȨ��  m_priors[m_mixNum]
	double** m_means;	// Gaussian��ֵ  m_means[m_mixNum][m_dimNum]
	double** m_vars;	// Gaussian���������ʽ  m_vars[m_mixNum][m_dimNum]

	// A minimum variance is required. Now, it is the overall variance * 0.01.
	double* m_minVars;      //��С���� m_minVars[m_dimNum] �� ������Э�����0.01����1e-10�е��������Ϊ��С����ֵ
	int m_maxIterNum;		// The stopping criterion regarding the number of iterations
	double m_endError;		// The stopping criterion regarding the error

private:
	// Return the "j"th pdf, p(x|j).
	double GetProbability(const double* x, int j);    //���ص�j����˹�����ܶ�ֵ
	void Allocate();      //ΪGMM������ڲ���������ռ�
	void Dispose();       //����GMM������ڲ���ָ�����
};
