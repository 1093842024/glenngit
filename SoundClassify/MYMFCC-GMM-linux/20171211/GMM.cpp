/***************************************************************************

Module Name:

	Gaussian Mixture Model with Diagonal Covariance Matrix

History:

	2003/11/01	Fei Wang
	2013 luxiaoxun
***************************************************************************/
#include <math.h>
#include <iostream>
#include "GMM.h"
#include "KMeans.h"
using namespace std;

//double M_PI=3.14159;

GMM::GMM(int dimNum, int mixNum)
{
	m_dimNum = dimNum;      //样本的特征维数
	m_mixNum = mixNum;      //Gaussian数目

	m_maxIterNum = 500;     //最大迭代次数
	m_endError = 0.000001;     //迭代停止误差

	Allocate();

	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = 1.0 / m_mixNum;         //平均初始化每个Gaussian模型的权重

		for (int d = 0; d < m_dimNum; d++)
		{
			m_means[i][d] = 0;                //将每个模型的均值设为0
			m_vars[i][d] = 1;                 //将每个模型的方差设为1
		}
	}
}

GMM::~GMM()
{
	Dispose();
}

void GMM::Allocate()
{
	m_priors = new double[m_mixNum];   //分配保存 Gaussian权重的空间
	m_means = new double*[m_mixNum];   //分配保存 Gaussian均值的空间
	m_vars = new double*[m_mixNum];    //分配保存 Gaussian方差的空间

	for (int i = 0; i < m_mixNum; i++)
	{
		m_means[i] = new double[m_dimNum];
		m_vars[i] = new double[m_dimNum];
	}

	m_minVars = new double[m_dimNum];
}

void GMM::Dispose()
{
	delete[] m_priors;

	for (int i = 0; i < m_mixNum; i++)
	{
		delete[] m_means[i];
		delete[] m_vars[i];
	}
	delete[] m_means;
	delete[] m_vars;

	delete[] m_minVars;
}

void GMM::Copy(GMM* gmm)   //将传入的gmm对象中的内容进行复制
{
	assert(m_mixNum == gmm->m_mixNum && m_dimNum == gmm->m_dimNum);   //确定被复制对象与复制对象的混合高斯模型结束与特征维度是一致的

	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = gmm->Prior(i);
		memcpy(m_means[i], gmm->Mean(i), sizeof(double) * m_dimNum);
		memcpy(m_vars[i], gmm->Variance(i), sizeof(double) * m_dimNum);
	}
	memcpy(m_minVars, gmm->m_minVars, sizeof(double) * m_dimNum);
}

double GMM::GetProbability(const double* sample)     //返回该样本传入该高斯混合模型的概率密度之和
{
	double p = 0;
	for (int i = 0; i < m_mixNum; i++)
	{
		p += m_priors[i] * GetProbability(sample, i); //将混合高斯模型中各个模型的概率密度值加权相加
	}
	return p;     //返回该样本传入该高斯混合模型的概率密度之和
}

double GMM::GetProbability(const double* x, int j)  //返回第j个高斯概率密度值
{
	double p = 1;
	for (int d = 0; d < m_dimNum; d++)
	{
		p *= 1 / sqrt(2 * 3.1415926 * m_vars[j][d]);
		p *= exp(-0.5 * (x[d] - m_means[j][d]) * (x[d] - m_means[j][d]) / m_vars[j][d]);
	}
	return p;
}

void GMM::Train(const char* sampleFileName)
{
	//DumpSampleFile(sampleFileName);
	Init(sampleFileName);

	ifstream sampleFile(sampleFileName, ios_base::binary);
	assert(sampleFile);

	int size = 0;
	sampleFile.seekg(0, ios_base::beg);
	sampleFile.read((char*)&size, sizeof(int));

	// Reestimation
	bool loop = true;
	double iterNum = 0;
	double lastL = 0;
	double currL = 0;
	int unchanged = 0;
	double* x = new double[m_dimNum];	// Sample data
	double* next_priors = new double[m_mixNum];
	double** next_vars = new double*[m_mixNum];
	double** next_means = new double*[m_mixNum];

	for (int i = 0; i < m_mixNum; i++)
	{
		next_means[i] = new double[m_dimNum];
		next_vars[i] = new double[m_dimNum];
	}

	while (loop)
	{
		// Clear buffer for reestimation
		memset(next_priors, 0, sizeof(double) * m_mixNum);
		for (int i = 0; i < m_mixNum; i++)
		{
			memset(next_vars[i], 0, sizeof(double) * m_dimNum);
			memset(next_means[i], 0, sizeof(double) * m_dimNum);
		}

		lastL = currL;
		currL = 0;

		// Predict
		sampleFile.seekg(2 * sizeof(int), ios_base::beg);
		for (int k = 0; k < size; k++)
		{
			sampleFile.read((char*)x, sizeof(double) * m_dimNum);
			double p = GetProbability(x);

			for (int j = 0; j < m_mixNum; j++)
			{
				double pj = GetProbability(x, j) * m_priors[j] / p;
				next_priors[j] += pj;
				for (int d = 0; d < m_dimNum; d++)
				{
					next_means[j][d] += pj * x[d];
					next_vars[j][d] += pj* x[d] * x[d];
				}
			}
			currL += (p > 1E-40) ? log10(p) : -40;
		}
		currL /= size;

		// Reestimation: generate new priors, means and variances.
		for (int j = 0; j < m_mixNum; j++)
		{
			m_priors[j] = next_priors[j] / size;

			if (m_priors[j] > 0)
			{
				for (int d = 0; d < m_dimNum; d++)
				{
					m_means[j][d] = next_means[j][d] / next_priors[j];
					m_vars[j][d] = next_vars[j][d] / next_priors[j] - m_means[j][d] * m_means[j][d];
					if (m_vars[j][d] < m_minVars[d])
					{
						m_vars[j][d] = m_minVars[d];
					}
				}
			}
		}

		// Terminal conditions
		iterNum++;
		if (fabs(currL - lastL) < m_endError * fabs(lastL))
		{
			unchanged++;
		}else unchanged=0;
		if (iterNum >= m_maxIterNum || unchanged >= 3)
		{
			loop = false;
		}
		//--- Debug ---
		//cout << "Iter: " << iterNum << ", Average Log-Probability: " << currL << endl;
	}

	sampleFile.close();
	delete[] next_priors;
	for (int i = 0; i < m_mixNum; i++)
	{
		delete[] next_means[i];
		delete[] next_vars[i];
	}
	delete[] next_means;
	delete[] next_vars;
	delete[] x;
}

void GMM::Train(double *data, int N)    //传入  数据data  与   数据data中样本的个数N
{
	Init(data,N); //初始化GMM中每个高斯模型的权重（按属于该模型的样本数量占的比例）、均值（按属于该模型的样本的均值）、协方差（按属于该模型的样本的协方差的均值）

	int size = N;

	// Reestimation
	bool loop = true;
	double iterNum = 0;
	double lastL = 0;
	double currL = 0;
	int unchanged = 0;
	double* x = new double[m_dimNum];	// Sample data  每个样本数据
	double* next_priors = new double[m_mixNum];    //每个高斯模型更新的权重
	double** next_vars = new double*[m_mixNum];    //每个高斯模型更新的方差
	double** next_means = new double*[m_mixNum];   //每个高斯模型更新的均值

	for (int i = 0; i < m_mixNum; i++)
	{
		next_means[i] = new double[m_dimNum];
		next_vars[i] = new double[m_dimNum];
	}

	while (loop)
	{
		// Clear buffer for reestimation
		memset(next_priors, 0, sizeof(double) * m_mixNum);  //首先将更新的每个高斯模型权重设为0
		for (int i = 0; i < m_mixNum; i++)    //将更新的每个高斯模型的方差和均值也设为0
		{
			memset(next_vars[i], 0, sizeof(double) * m_dimNum);
			memset(next_means[i], 0, sizeof(double) * m_dimNum);
		}

		lastL = currL;
		currL = 0;

		// Predict
		for (int k = 0; k < size; k++)
		{
			for(int j=0;j<m_dimNum;j++)    //对于每个样本
				x[j]=data[k*m_dimNum+j];
			double p = GetProbability(x);  //获得该样本的再当前GMM模型下的高斯概率密度值

			for (int j = 0; j < m_mixNum; j++)
			{
				double pj = GetProbability(x, j) * m_priors[j] / p;  //计算该样本x在其中第j个模型的概率密度值占总概率密度值的p的比例大小pj，也即权重

				next_priors[j] += pj;  //将所有样本在第j个高斯模型中的权重累加记录

				for (int d = 0; d < m_dimNum; d++)  //将所有样本在第j个高斯模型中的均值和协方差按权重累加记录
				{
					next_means[j][d] += pj * x[d];
					next_vars[j][d] += pj* x[d] * x[d];
				}
			}

			currL += (p > 1E-40) ? log10(p) : -40;   //将所有样本产生的高斯概率密度值的对数累加
		}
		currL /= size;      //求该次迭代（即上一次更新的GMM参数下）的高斯概率密度均值

		// Reestimation: generate new priors, means and variances.
		for (int j = 0; j < m_mixNum; j++)
		{
			m_priors[j] = next_priors[j] / size;  //更新每个高斯模型的权重

			if (m_priors[j] > 0)
			{
				for (int d = 0; d < m_dimNum; d++)    //更新每个高斯模型的均值和协方差
				{
					m_means[j][d] = next_means[j][d] / next_priors[j];
					m_vars[j][d] = next_vars[j][d] / next_priors[j] - m_means[j][d] * m_means[j][d];
					if (m_vars[j][d] < m_minVars[d])
					{
						m_vars[j][d] = m_minVars[d];
					}
				}
			}
		}

		// Terminal conditions
		iterNum++;
		if (fabs(currL - lastL) < m_endError * fabs(lastL))  //如果两次迭代的高斯概率密度均值差值小于0.01% 则将unchanged数加1
		{
			unchanged++;
		}else unchanged=0;
		if (iterNum >= m_maxIterNum || unchanged >= 3)   //如果迭代次数大于最大次数，或者有三次迭代的差值足够小
		{
			if(iterNum >= m_maxIterNum ) printf("GMM iteration reach max iternum!\n");
			loop = false;
		}

		//--- Debug ---
		printf("GMM Iter:%1.0f , Average Log-Probability: %f \n",iterNum,currL);
		//cout << "GMM Iter: " << iterNum << ", Average Log-Probability: " << currL << endl;
	}
	delete[] next_priors;
	for (int i = 0; i < m_mixNum; i++)
	{
		delete[] next_means[i];
		delete[] next_vars[i];
	}
	delete[] next_means;
	delete[] next_vars;
	delete[] x;
}

void GMM::Init(double *data, int N)    //传入  数据data  与  数据data中样本的个数   
{                                     //初始化GMM中每个高斯模型的权重（按属于该模型的样本数量占的比例）、均值（按属于该模型的样本的均值）、协方差（按属于该模型的样本的协方差的均值）
	const double MIN_VAR = 1E-10;

	KMeans* kmeans = new KMeans(m_dimNum, m_mixNum);  //创建k-means类，输入GMM的样本维数  以及 GMM的阶数作为聚类的数量
	kmeans->SetInitMode(KMeans::InitUniform);
	int *Label;
	Label=new int[N];              
	kmeans->Cluster(data,N,Label);                  //对N个数据样本进行聚类，计算每类的均值 以及 每个样本数据属于的类

	int* counts = new int[m_mixNum];           //用于记录GMM中每个高斯模型对应的样本数量
	double* overMeans = new double[m_dimNum];	// Overall mean of training data 训练数据每一维度的均值
	for (int i = 0; i < m_mixNum; i++)    //对于GMM中每一个高斯模型
	{
		counts[i] = 0;
		m_priors[i] = 0;    //初始化第i个高斯模型权重为0
		memcpy(m_means[i], kmeans->GetMean(i), sizeof(double) * m_dimNum);//初始化第i个高斯模型均值为第i个聚类的均值
		memset(m_vars[i], 0, sizeof(double) * m_dimNum);  //初始化第i个高斯模型的方差为0
	}
	memset(overMeans, 0, sizeof(double) * m_dimNum);  //训练数据每个维度的整体均值为0
	memset(m_minVars, 0, sizeof(double) * m_dimNum);  //训练数据每个维度的最小方差为0

	int size = 0;
	size=N;

	double* x = new double[m_dimNum];
	int label = -1;

	for (int i = 0; i < size; i++)
	{
		for(int j=0;j<m_dimNum;j++)  //每次将x中赋值一个样本数据
			x[j]=data[i*m_dimNum+j];
		label=Label[i];              //label记录该样本数据的标签值

		// Count each Gaussian
		counts[label]++;                //将对应该标签值的高斯模型的counts数增加一个
		double* m = kmeans->GetMean(label);  //获得该标签的聚类（对应该高斯模型）的均值
		for (int d = 0; d < m_dimNum; d++)
		{
			m_vars[label][d] += (x[d] - m[d]) * (x[d] - m[d]);//计算该标签的高斯模型的协方差
		}

		// Count the overall mean and variance.  计算整体均值的和以及协方差的和
		for (int d = 0; d < m_dimNum; d++)
		{
			overMeans[d] += x[d];
			m_minVars[d] += x[d] * x[d];
		}
	}

	// Compute the overall variance (* 0.01) as the minimum variance.
	for (int d = 0; d < m_dimNum; d++)  //计算样本每一维度的整体均值以及最小协方差
	{
		overMeans[d] /= size;
		m_minVars[d] = max(MIN_VAR, 0.01 * (m_minVars[d] / size - overMeans[d] * overMeans[d]));
	}

	// Initialize each Gaussian.   初始化每个高斯模型
	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = 1.0 * counts[i] / size;   //根据上面统计的属于每个高斯模型的样本的个数比例，初始化每个高斯模型的权重

		if (m_priors[i] > 0)    //如果该高斯模型对应了样本
		{
			for (int d = 0; d < m_dimNum; d++)
			{
				m_vars[i][d] = m_vars[i][d] / counts[i];   //将协方差取该类所有样本协方差的均值

				// A minimum variance for each dimension is required.
				if (m_vars[i][d] < m_minVars[d])      //如果协方差小于最小值协方差 则将协方差设为最小值
				{ 
					m_vars[i][d] = m_minVars[d];       
				}
			}
		}
		else    //如果其中一个高斯模型没有对应任何样本，则将最小方差赋予该模型，并打印警告
		{
			memcpy(m_vars[i], m_minVars, sizeof(double) * m_dimNum);
			cout << "[WARNING] Gaussian " << i << " of GMM is not used!\n";
		}
	}
	delete kmeans;
	delete[] x;
	delete[] counts;
	delete[] overMeans;
	delete[] Label;

}

void GMM::Init(const char* sampleFileName)
{
	const double MIN_VAR = 1E-10;

	KMeans* kmeans = new KMeans(m_dimNum, m_mixNum);
	kmeans->SetInitMode(KMeans::InitUniform);
	kmeans->Cluster(sampleFileName, "gmm_init.tmp");

	int* counts = new int[m_mixNum];
	double* overMeans = new double[m_dimNum];	// Overall mean of training data
	for (int i = 0; i < m_mixNum; i++)
	{
		counts[i] = 0;
		m_priors[i] = 0;
		memcpy(m_means[i], kmeans->GetMean(i), sizeof(double) * m_dimNum);
		memset(m_vars[i], 0, sizeof(double) * m_dimNum);
	}
	memset(overMeans, 0, sizeof(double) * m_dimNum);
	memset(m_minVars, 0, sizeof(double) * m_dimNum);

	// Open the sample and label file to initialize the model
	ifstream sampleFile(sampleFileName, ios_base::binary);
	assert(sampleFile);

	ifstream labelFile("gmm_init.tmp", ios_base::binary);
	assert(labelFile);

	int size = 0;
	sampleFile.read((char*)&size, sizeof(int));
	sampleFile.seekg(2 * sizeof(int), ios_base::beg);
	labelFile.seekg(sizeof(int), ios_base::beg);

	double* x = new double[m_dimNum];
	int label = -1;

	for (int i = 0; i < size; i++)
	{
		sampleFile.read((char*)x, sizeof(double) * m_dimNum);
		labelFile.read((char*)&label, sizeof(int));

		// Count each Gaussian
		counts[label]++;
		double* m = kmeans->GetMean(label);
		for (int d = 0; d < m_dimNum; d++)
		{
			m_vars[label][d] += (x[d] - m[d]) * (x[d] - m[d]);
		}

		// Count the overall mean and variance.
		for (int d = 0; d < m_dimNum; d++)
		{
			overMeans[d] += x[d];
			m_minVars[d] += x[d] * x[d];
		}
	}

	// Compute the overall variance (* 0.01) as the minimum variance.
	for (int d = 0; d < m_dimNum; d++)
	{
		overMeans[d] /= size;
		m_minVars[d] = max(MIN_VAR, 0.01 * (m_minVars[d] / size - overMeans[d] * overMeans[d]));
	}

	// Initialize each Gaussian.
	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = 1.0 * counts[i] / size;

		if (m_priors[i] > 0)
		{
			for (int d = 0; d < m_dimNum; d++)
			{
				m_vars[i][d] = m_vars[i][d] / counts[i];

				// A minimum variance for each dimension is required.
				if (m_vars[i][d] < m_minVars[d])
				{
					m_vars[i][d] = m_minVars[d];
				}
			}
		}
		else
		{
			memcpy(m_vars[i], m_minVars, sizeof(double) * m_dimNum);
			cout << "[WARNING] Gaussian " << i << " of GMM is not used!\n";
		}
	}

	delete kmeans;
	delete[] x;
	delete[] counts;
	delete[] overMeans;

	sampleFile.close();
	labelFile.close();
}

void GMM::DumpSampleFile(const char* fileName)
{
	ifstream sampleFile(fileName, ios_base::binary);
	assert(sampleFile);

	int size = 0;
	sampleFile.read((char*)&size, sizeof(int));
	cout << size << endl;

	int dim = 0;
	sampleFile.read((char*)&dim, sizeof(int));
	cout << dim << endl;

	double* f = new double[dim];
	for (int i = 0; i < size; i++)
	{
		sampleFile.read((char*)f, sizeof(double) * dim);

		cout << i << ":";
		for (int j = 0; j < dim; j++)
		{
			cout << " " << f[j];
		}
		cout << endl;
	}

	delete[] f;
	sampleFile.close();
}

ostream& operator<<(ostream& out, GMM& gmm)
{
	out << "<GMM>" << endl;
	out << "<DimNum> " << gmm.m_dimNum << " </DimNum>" << endl;
	out << "<MixNum> " << gmm.m_mixNum << " </MixNum>" << endl;

	out << "<Prior> ";
	for (int i = 0; i < gmm.m_mixNum; i++)
	{
		out << gmm.m_priors[i] << " ";
	}
	out << "</Prior>" << endl;

	out << "<Mean>" << endl;
	for (int i = 0; i < gmm.m_mixNum; i++)
	{
		for (int d = 0; d < gmm.m_dimNum; d++)
		{
			out << gmm.m_means[i][d] << " ";
		}
		out << endl;
	}
	out << "</Mean>" << endl;

	out << "<Variance>" << endl;
	for (int i = 0; i < gmm.m_mixNum; i++)
	{
		for (int d = 0; d < gmm.m_dimNum; d++)
		{
			out << gmm.m_vars[i][d] << " ";
		}
		out << endl;
	}
	out << "</Variance>" << endl;

	out << "</GMM>" << endl;

	return out;
}

istream& operator>>(istream& in, GMM& gmm)
{
	char label[50];
	in >> label; // "<GMM>"
	assert(strcmp(label, "<GMM>") == 0);

	gmm.Dispose();

	in >> label >> gmm.m_dimNum >> label; // "<DimNum>"
	in >> label >> gmm.m_mixNum >> label; // "<MixNum>"

	gmm.Allocate();

	in >> label; // "<Prior>"
	for (int i = 0; i < gmm.m_mixNum; i++)
	{
		in >> gmm.m_priors[i];
	}
	in >> label;

	in >> label; // "<Mean>"
	for (int i = 0; i < gmm.m_mixNum; i++)
	{
		for (int d = 0; d < gmm.m_dimNum; d++)
		{
			in >> gmm.m_means[i][d];
		}
	}
	in >> label;

	in >> label; // "<Variance>"
	for (int i = 0; i < gmm.m_mixNum; i++)
	{
		for (int d = 0; d < gmm.m_dimNum; d++)
		{
			in >> gmm.m_vars[i][d];
		}
	}
	in >> label;

	in >> label; // "</GMM>"
	return in;
}
