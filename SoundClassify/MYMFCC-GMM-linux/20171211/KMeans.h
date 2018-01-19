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
	enum InitMode                       //初始化模式 随机模式、手动模式、统一模式
	{
		InitRandom,
		InitManual,
		InitUniform,
	};

	KMeans(int dimNum = 1, int clusterNum = 1);    //构造函数：输入数据的维度，分类中心数量
	~KMeans();

	void SetMean(int i, const double* u){ memcpy(m_means[i], u, sizeof(double) * m_dimNum); }   //设置均值
	void SetInitMode(int i)				{ m_initMode = i; }                                     //设置k-means初始化模式
	void SetMaxIterNum(int i)			{ m_maxIterNum = i; }                                   //设置最大迭代次数
	void SetEndError(double f)			{ m_endError = f; }                                     //返回结束错误

	double* GetMean(int i)	{ return m_means[i]; }            //获得第i个聚类的均值
	int GetInitMode()		{ return m_initMode; }            //获得初始化模式
	int GetMaxIterNum()		{ return m_maxIterNum; }          //获得最大迭代次数
	double GetEndError()	{ return m_endError; }            //获得结束错误


	/*	SampleFile: <size><dim><data>...
		LabelFile:	<size><label>...
	*/
	void Cluster(const char* sampleFileName, const char* labelFileName);        //将数据文件（尺寸、维度、数据） 聚类成 标签文件（尺寸、标签）
	void Init(std::ifstream& sampleFile);                                       //用输入的数据文件进行初始化
	void Init(double *data, int N);                                             //用数据以及其样本数量信息 进行初始化
	void Cluster(double *data, int N, int *Label);                              //用数据、数据样本数量 进行聚类，并将聚类信息记录在label中
	friend std::ostream& operator<<(std::ostream& out, KMeans& kmeans);         //定义操作符，将训练得到的信息存入文档中

private:
	int m_dimNum;         //每个样本的数据维度
	int m_clusterNum;     //聚类中心的数量
	double** m_means;     //一个二级指针，存储一个二维数组，第一维代表聚类类型，第二维代表样本的数据维度 即m_means[m_clusterNum][m_dimNum]  代表每个聚类（m_clusterNum）的均值数据（m_dimNum）

	int m_initMode;                                                                //    
	int m_maxIterNum;		// The stopping criterion regarding the number of iterations 停止准则：最大迭代次数
	double m_endError;		// The stopping criterion regarding the error                停止准则：迭代误差

	double GetLabel(const double* x, int* label);                                  //     获取聚类标签
	double CalcDistance(const double* x, const double* u, int dimNum);             //     计算距离
};
