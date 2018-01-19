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
	GMM(int dimNum = 1, int mixNum = 1);      //构造函数输入  样本的特征维度  混合的高斯模型数量
	~GMM();

	void Copy(GMM* gmm);  //复制gmm对象
	//设置算法内部参数
	void SetMaxIterNum(int i)	{ m_maxIterNum = i; }   
	void SetEndError(double f)	{ m_endError = f; }
	//获取算法内部参数
	int GetDimNum()			{ return m_dimNum; }
	int GetMixNum()			{ return m_mixNum; }
	int GetMaxIterNum()		{ return m_maxIterNum; }
	double GetEndError()	{ return m_endError; }

	double& Prior(int i)	{ return m_priors[i]; }
	double* Mean(int i)		{ return m_means[i]; }
	double* Variance(int i)	{ return m_vars[i]; }

	//设置权重、均值、方差
	void setPrior(int i,double val)	{  m_priors[i]=val; }
	void setMean(int i,double *val)		{ for(int j=0;j<m_dimNum;j++) m_means[i][j]=val[j]; }
	void setVariance(int i,double *val)	{ for(int j=0;j<m_dimNum;j++) m_vars[i][j]=val[j]; }

	//获得样本的的高斯概率密度值
	double GetProbability(const double* sample);

	/*	SampleFile: <size><dim><data>...*/  //两种初始化高斯模型以及训练高斯模型的方式
    void Init(const char* sampleFileName);
	void Train(const char* sampleFileName);
	void Init(double *data, int N);
	void Train(double *data, int N);

	void DumpSampleFile(const char* fileName);

	friend std::ostream& operator<<(std::ostream& out, GMM& gmm);
	friend std::istream& operator>>(std::istream& in, GMM& gmm);

private:
	int m_dimNum;		// 样本维数
	int m_mixNum;		// Gaussian数目
	double* m_priors;	// Gaussian权重  m_priors[m_mixNum]
	double** m_means;	// Gaussian均值  m_means[m_mixNum][m_dimNum]
	double** m_vars;	// Gaussian方差的行列式  m_vars[m_mixNum][m_dimNum]

	// A minimum variance is required. Now, it is the overall variance * 0.01.
	double* m_minVars;      //最小方差 m_minVars[m_dimNum] ： 将整体协方差的0.01倍与1e-10中的最大者作为最小方差值
	int m_maxIterNum;		// The stopping criterion regarding the number of iterations
	double m_endError;		// The stopping criterion regarding the error

private:
	// Return the "j"th pdf, p(x|j).
	double GetProbability(const double* x, int j);    //返回第j个高斯概率密度值
	void Allocate();      //为GMM对象的内部参数分配空间
	void Dispose();       //析构GMM对象的内部的指针参数
};
