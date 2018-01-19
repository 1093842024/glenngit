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
	m_dimNum = dimNum;      //����������ά��
	m_mixNum = mixNum;      //Gaussian��Ŀ

	m_maxIterNum = 500;     //����������
	m_endError = 0.000001;     //����ֹͣ���

	Allocate();

	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = 1.0 / m_mixNum;         //ƽ����ʼ��ÿ��Gaussianģ�͵�Ȩ��

		for (int d = 0; d < m_dimNum; d++)
		{
			m_means[i][d] = 0;                //��ÿ��ģ�͵ľ�ֵ��Ϊ0
			m_vars[i][d] = 1;                 //��ÿ��ģ�͵ķ�����Ϊ1
		}
	}
}

GMM::~GMM()
{
	Dispose();
}

void GMM::Allocate()
{
	m_priors = new double[m_mixNum];   //���䱣�� GaussianȨ�صĿռ�
	m_means = new double*[m_mixNum];   //���䱣�� Gaussian��ֵ�Ŀռ�
	m_vars = new double*[m_mixNum];    //���䱣�� Gaussian����Ŀռ�

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

void GMM::Copy(GMM* gmm)   //�������gmm�����е����ݽ��и���
{
	assert(m_mixNum == gmm->m_mixNum && m_dimNum == gmm->m_dimNum);   //ȷ�������ƶ����븴�ƶ���Ļ�ϸ�˹ģ�ͽ���������ά����һ�µ�

	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = gmm->Prior(i);
		memcpy(m_means[i], gmm->Mean(i), sizeof(double) * m_dimNum);
		memcpy(m_vars[i], gmm->Variance(i), sizeof(double) * m_dimNum);
	}
	memcpy(m_minVars, gmm->m_minVars, sizeof(double) * m_dimNum);
}

double GMM::GetProbability(const double* sample)     //���ظ���������ø�˹���ģ�͵ĸ����ܶ�֮��
{
	double p = 0;
	for (int i = 0; i < m_mixNum; i++)
	{
		p += m_priors[i] * GetProbability(sample, i); //����ϸ�˹ģ���и���ģ�͵ĸ����ܶ�ֵ��Ȩ���
	}
	return p;     //���ظ���������ø�˹���ģ�͵ĸ����ܶ�֮��
}

double GMM::GetProbability(const double* x, int j)  //���ص�j����˹�����ܶ�ֵ
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

void GMM::Train(double *data, int N)    //����  ����data  ��   ����data�������ĸ���N
{
	Init(data,N); //��ʼ��GMM��ÿ����˹ģ�͵�Ȩ�أ������ڸ�ģ�͵���������ռ�ı���������ֵ�������ڸ�ģ�͵������ľ�ֵ����Э��������ڸ�ģ�͵�������Э����ľ�ֵ��

	int size = N;

	// Reestimation
	bool loop = true;
	double iterNum = 0;
	double lastL = 0;
	double currL = 0;
	int unchanged = 0;
	double* x = new double[m_dimNum];	// Sample data  ÿ����������
	double* next_priors = new double[m_mixNum];    //ÿ����˹ģ�͸��µ�Ȩ��
	double** next_vars = new double*[m_mixNum];    //ÿ����˹ģ�͸��µķ���
	double** next_means = new double*[m_mixNum];   //ÿ����˹ģ�͸��µľ�ֵ

	for (int i = 0; i < m_mixNum; i++)
	{
		next_means[i] = new double[m_dimNum];
		next_vars[i] = new double[m_dimNum];
	}

	while (loop)
	{
		// Clear buffer for reestimation
		memset(next_priors, 0, sizeof(double) * m_mixNum);  //���Ƚ����µ�ÿ����˹ģ��Ȩ����Ϊ0
		for (int i = 0; i < m_mixNum; i++)    //�����µ�ÿ����˹ģ�͵ķ���;�ֵҲ��Ϊ0
		{
			memset(next_vars[i], 0, sizeof(double) * m_dimNum);
			memset(next_means[i], 0, sizeof(double) * m_dimNum);
		}

		lastL = currL;
		currL = 0;

		// Predict
		for (int k = 0; k < size; k++)
		{
			for(int j=0;j<m_dimNum;j++)    //����ÿ������
				x[j]=data[k*m_dimNum+j];
			double p = GetProbability(x);  //��ø��������ٵ�ǰGMMģ���µĸ�˹�����ܶ�ֵ

			for (int j = 0; j < m_mixNum; j++)
			{
				double pj = GetProbability(x, j) * m_priors[j] / p;  //���������x�����е�j��ģ�͵ĸ����ܶ�ֵռ�ܸ����ܶ�ֵ��p�ı�����Сpj��Ҳ��Ȩ��

				next_priors[j] += pj;  //�����������ڵ�j����˹ģ���е�Ȩ���ۼӼ�¼

				for (int d = 0; d < m_dimNum; d++)  //�����������ڵ�j����˹ģ���еľ�ֵ��Э���Ȩ���ۼӼ�¼
				{
					next_means[j][d] += pj * x[d];
					next_vars[j][d] += pj* x[d] * x[d];
				}
			}

			currL += (p > 1E-40) ? log10(p) : -40;   //���������������ĸ�˹�����ܶ�ֵ�Ķ����ۼ�
		}
		currL /= size;      //��ôε���������һ�θ��µ�GMM�����£��ĸ�˹�����ܶȾ�ֵ

		// Reestimation: generate new priors, means and variances.
		for (int j = 0; j < m_mixNum; j++)
		{
			m_priors[j] = next_priors[j] / size;  //����ÿ����˹ģ�͵�Ȩ��

			if (m_priors[j] > 0)
			{
				for (int d = 0; d < m_dimNum; d++)    //����ÿ����˹ģ�͵ľ�ֵ��Э����
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
		if (fabs(currL - lastL) < m_endError * fabs(lastL))  //������ε����ĸ�˹�����ܶȾ�ֵ��ֵС��0.01% ��unchanged����1
		{
			unchanged++;
		}else unchanged=0;
		if (iterNum >= m_maxIterNum || unchanged >= 3)   //��������������������������������ε����Ĳ�ֵ�㹻С
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

void GMM::Init(double *data, int N)    //����  ����data  ��  ����data�������ĸ���   
{                                     //��ʼ��GMM��ÿ����˹ģ�͵�Ȩ�أ������ڸ�ģ�͵���������ռ�ı���������ֵ�������ڸ�ģ�͵������ľ�ֵ����Э��������ڸ�ģ�͵�������Э����ľ�ֵ��
	const double MIN_VAR = 1E-10;

	KMeans* kmeans = new KMeans(m_dimNum, m_mixNum);  //����k-means�࣬����GMM������ά��  �Լ� GMM�Ľ�����Ϊ���������
	kmeans->SetInitMode(KMeans::InitUniform);
	int *Label;
	Label=new int[N];              
	kmeans->Cluster(data,N,Label);                  //��N�������������о��࣬����ÿ��ľ�ֵ �Լ� ÿ�������������ڵ���

	int* counts = new int[m_mixNum];           //���ڼ�¼GMM��ÿ����˹ģ�Ͷ�Ӧ����������
	double* overMeans = new double[m_dimNum];	// Overall mean of training data ѵ������ÿһά�ȵľ�ֵ
	for (int i = 0; i < m_mixNum; i++)    //����GMM��ÿһ����˹ģ��
	{
		counts[i] = 0;
		m_priors[i] = 0;    //��ʼ����i����˹ģ��Ȩ��Ϊ0
		memcpy(m_means[i], kmeans->GetMean(i), sizeof(double) * m_dimNum);//��ʼ����i����˹ģ�;�ֵΪ��i������ľ�ֵ
		memset(m_vars[i], 0, sizeof(double) * m_dimNum);  //��ʼ����i����˹ģ�͵ķ���Ϊ0
	}
	memset(overMeans, 0, sizeof(double) * m_dimNum);  //ѵ������ÿ��ά�ȵ������ֵΪ0
	memset(m_minVars, 0, sizeof(double) * m_dimNum);  //ѵ������ÿ��ά�ȵ���С����Ϊ0

	int size = 0;
	size=N;

	double* x = new double[m_dimNum];
	int label = -1;

	for (int i = 0; i < size; i++)
	{
		for(int j=0;j<m_dimNum;j++)  //ÿ�ν�x�и�ֵһ����������
			x[j]=data[i*m_dimNum+j];
		label=Label[i];              //label��¼���������ݵı�ǩֵ

		// Count each Gaussian
		counts[label]++;                //����Ӧ�ñ�ǩֵ�ĸ�˹ģ�͵�counts������һ��
		double* m = kmeans->GetMean(label);  //��øñ�ǩ�ľ��ࣨ��Ӧ�ø�˹ģ�ͣ��ľ�ֵ
		for (int d = 0; d < m_dimNum; d++)
		{
			m_vars[label][d] += (x[d] - m[d]) * (x[d] - m[d]);//����ñ�ǩ�ĸ�˹ģ�͵�Э����
		}

		// Count the overall mean and variance.  ���������ֵ�ĺ��Լ�Э����ĺ�
		for (int d = 0; d < m_dimNum; d++)
		{
			overMeans[d] += x[d];
			m_minVars[d] += x[d] * x[d];
		}
	}

	// Compute the overall variance (* 0.01) as the minimum variance.
	for (int d = 0; d < m_dimNum; d++)  //��������ÿһά�ȵ������ֵ�Լ���СЭ����
	{
		overMeans[d] /= size;
		m_minVars[d] = max(MIN_VAR, 0.01 * (m_minVars[d] / size - overMeans[d] * overMeans[d]));
	}

	// Initialize each Gaussian.   ��ʼ��ÿ����˹ģ��
	for (int i = 0; i < m_mixNum; i++)
	{
		m_priors[i] = 1.0 * counts[i] / size;   //��������ͳ�Ƶ�����ÿ����˹ģ�͵������ĸ�����������ʼ��ÿ����˹ģ�͵�Ȩ��

		if (m_priors[i] > 0)    //����ø�˹ģ�Ͷ�Ӧ������
		{
			for (int d = 0; d < m_dimNum; d++)
			{
				m_vars[i][d] = m_vars[i][d] / counts[i];   //��Э����ȡ������������Э����ľ�ֵ

				// A minimum variance for each dimension is required.
				if (m_vars[i][d] < m_minVars[d])      //���Э����С����СֵЭ���� ��Э������Ϊ��Сֵ
				{ 
					m_vars[i][d] = m_minVars[d];       
				}
			}
		}
		else    //�������һ����˹ģ��û�ж�Ӧ�κ�����������С������ģ�ͣ�����ӡ����
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
