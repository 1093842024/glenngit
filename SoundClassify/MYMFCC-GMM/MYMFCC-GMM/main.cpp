#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <vector>
#include "SoundClassify.h"
#include "filterbank.h"
#include <time.h>
#include <opencv2/opencv.hpp> 



using namespace std;

#define GMM_MIX_NUM 32    //高斯混合模型阶数   GMM的阶数作为其中kmeans聚类的数量，将聚类中心的均值和方差作为每个高斯混合模型的初始均值和方差

//#define ANALYZEWAV
//#define OUTPUT 
//#define TRAIN
//#define TEST
#define INTERFACE
//#define INTERFACE1

void getans(int val,int num)
{
	printf("val:%d,num:%d\n",val,num);
}


int main()
{

#ifdef ANALYZEWAV

	string wav_path="data/baby039.wav";
	char wav[100];   strcpy(wav,wav_path.c_str());
	Wave *w = filter_bank(wav);
    deletewav(w);


#endif



#ifdef OUTPUT 

	string wav_path="data/baby_audio_pcm";    int wav_num=1;  double *wav_data;  //50 68
	long int datasize; /*特征数据量大小*/  int dim;/*特征向量的维度*/   string feature;
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+1");  
	delete wav_data;
	/*wav_path="data/boom";   wav_num=50;     //50 67
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+2");  
	delete wav_data;
	wav_path="data/glass";  wav_num=25;     //25 36
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+3");  
	delete wav_data;
	wav_path="data/gun";    wav_num=80;     //80 128
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+4");  
	delete wav_data;
	wav_path="data/scream"; wav_num=35;     //35 49
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+5");  
	delete wav_data;
	wav_path="data/alarm"; wav_num=15;      //15 23
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+6");  
	delete wav_data;
	wav_path="data/red"; wav_num=9;         //9 14
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	delete wav_data;
    wav_path="data/blue";   wav_num=18;     //18 28
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	delete wav_data;
	wav_path="data/yellow"; wav_num=8;      //8 13
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	delete wav_data;
	wav_path="data/test"; wav_num=45;       //45  62
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+8");  
	delete wav_data;*/
	
	/*
	wav_path="data/test/baby";    wav_num=18;   //50 68
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+1");  
	delete wav_data;
	wav_path="data/test/boom";   wav_num=17;     //50 67
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+2");  
	delete wav_data;
	wav_path="data/test/glass";  wav_num=11;     //25 36
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+3");  
	delete wav_data;
	wav_path="data/test/gun";    wav_num=48;     //80 128
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+4");  
	delete wav_data;
	wav_path="data/test/scream"; wav_num=14;     //35 49
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+5");  
	delete wav_data;
	wav_path="data/test/alarm"; wav_num=8;      //15 23
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+6");  
	delete wav_data;
	wav_path="data/test/red"; wav_num=5;         //9 14
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	delete wav_data;
	wav_path="data/test/blue";   wav_num=10;     //18 28
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	delete wav_data;
	wav_path="data/test/yellow"; wav_num=5;      //8 13
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	delete wav_data;
	wav_path="data/test/test"; wav_num=17;       //45  62
	wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature,true,"+8");  
	delete wav_data;
	*/

	
	/*wav_path="data/baby";    wav_num=68;   //50 68
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+1");  
	wav_path="data/boom";   wav_num=67;     //50 67
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+2");  
	wav_path="data/glass";  wav_num=36;     //25 36
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+3");  
	wav_path="data/gun";    wav_num=128;     //80 128
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+4");  
	wav_path="data/scream"; wav_num=49;     //35 49
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+5");  
	wav_path="data/alarm"; wav_num=23;      //15 23
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+6");  
	wav_path="data/red"; wav_num=14;         //9 14
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	wav_path="data/blue";   wav_num=28;     //18 28
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	wav_path="data/yellow"; wav_num=13;      //8 13
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+7");  
	wav_path="data/test"; wav_num=62;       //45  62
	extracttrain_mfcc_diff_valide_singleoutput(wav_path,wav_num,datasize,dim,feature,true,"+8");  
	*/

#endif




#ifdef TRAIN
//1. 提取所有训练样本的数据，用于后续模型的训练
	string wav_path="device/babym";   const int wav_num=27;   
	//string wav_path="data/blue";   const int wav_num=28;  
	//string wav_path="data/boom";   const int wav_num=67;   
	//string wav_path="data/glass";   const int wav_num=36;   
	//string wav_path="data/gun";   const int wav_num=128; 
	//string wav_path="data/scream";   const int wav_num=49;   
	long int datasize; /*特征数据量大小*/  int dim;/*特征向量的维度*/   string feature;
    //double *wav_data=extracttrain_mfcc_energy(wav_path,wav_num,datasize,dim,feature);     //返回所有特征数据值
	//double *wav_data=extracttrain_mfcc_energy_valide(wav_path,wav_num,datasize,dim,feature);
	//double *wav_data=extracttrain_mfcc_diff(wav_path,wav_num,datasize,dim,feature); 
	double *wav_data=extracttrain_mfcc_diff_valide(wav_path,wav_num,datasize,dim,feature); 
	//double *wav_data=extracttrain_mfcc(wav_path,wav_num,datasize,dim,feature);
	//double *wav_data=extracttrain_mdiff(wav_path,wav_num,datasize,dim,feature);   
	system("pause");
	
//2. 训练模型
	const int mix_num=GMM_MIX_NUM; const int size =datasize/dim;     /*特征向量的个数*/
	string gmm_struct="gmm_wav_baby_valide_8khz.txt";	//save GMM to file
	train_save_gmm(dim,mix_num,size,wav_data,gmm_struct);
#endif
#ifdef TEST
	 const int mix_num=GMM_MIX_NUM;
	//string gmm_struct="gmm_wav_baby14.txt";string feature="mfcc_energy";int dim=14;
	//string gmm_struct="gmm_wav_baby14_valide.txt";string feature="mfcc_energy";int dim=14;  //string gmm_struct="gmm_wav_baby13.txt";string feature="mfcc";int dim=13;
	//string gmm_struct="gmm_wav_baby.txt"; string feature="mfcc_diff"; int dim=26;
	string gmm_struct="gmm_wav_baby_valide_8khz.txt"; string feature="mfcc_diff";int dim=26;


//3. 载入模型
	printf("\nTest GMM:\n");
	GMM *gmm1= loadgmm(dim,mix_num,gmm_struct);

//4. 数据测试
    

	string wav_path_test="device/babym";  int wav_num_test=27;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	system("pause");
	//wav_path_test="data/baby_audio_pcm";    wav_num_test=3; 
	//testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/boom";           wav_num_test=67;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/glass";           wav_num_test=36;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/gun";           wav_num_test=128;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/scream";          wav_num_test=49;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
    wav_path_test="data/test";           wav_num_test=62;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/blue";             wav_num_test=28;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/red";              wav_num_test=14;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/yellow";           wav_num_test=13;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/alarm";           wav_num_test=23;
	testwavs(wav_path_test,wav_num_test,gmm1,dim,feature);

  
	/*string wav_path_test="data/baby001.wav"; double averprob,maxprob;
    testsinglewav_valide(wav_path_test,gmm1,dim,feature,averprob,maxprob);
	printf("average prob is:%.3f,maxprob is: %.3f\n",averprob,maxprob);*/

   
   /* string wav_path_test="device/babym";  int wav_num_test=27;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);
	system("pause");

	wav_path_test="data/baby";           wav_num_test=64;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/test";           wav_num_test=60;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/blue";             wav_num_test=28;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/red";              wav_num_test=14;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/yellow";           wav_num_test=13;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);
	wav_path_test="data/alarm";           wav_num_test=23;
	testwavs_valide(wav_path_test,wav_num_test,gmm1,dim,feature);*/
	

	delete gmm1;
    system("pause");
#endif

	

#ifdef INTERFACE
	unsigned char* wave=new unsigned char[1024];
	for(int i=0; i<1024;i++) 
	{
		wave[i]=250;i++;  wave[i]=127;}
	 wave[0]=0;wave[1]=0;
	 wave[1022]=254;wave[1023]=127;
	 double t,time_profile_counter;
	 t = cvGetTickCount();
	 time_profile_counter = (double)cvGetTickCount();
	 int ans=AudioBeginDetect(wave,1024);      
	 time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
	//std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
	//printf("ans is %d\n",ans);


	SoundClassify sound(getans);
	vector<string> soundclass;

	string wav_path_test="device/babym007.wav";

	
	int val=sound.ValideSoundClassify(wav_path_test,soundclass);
	
	/*for(int i=0;i<10000;i++)
	{
		printf("******************try time %d\n",i);
	    wav_path_test="device/babym021.wav";
	    char wav[100];    strcpy(wav,wav_path_test.c_str());    //printf("\nstart extracting features \n",wavpath);
	    Wave *w = filter_bank(wav,0);
	    deletewav(w);
	}*/

	/*wav_path_test="timit.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
    wav_path_test="data/alarm003.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
    wav_path_test="data/red007.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="data/test042.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="data/yellow004.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);*/

	/*wav_path_test="device/babym001.wav";
	val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="device/babym005.wav";
	val=sound.ValideSoundClassify(wav_path_test,soundclass);

	wav_path_test="device/babym009.wav";
	val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="device/babym011.wav";
    val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="device/babym028.wav";
	val=sound.ValideSoundClassify(wav_path_test,soundclass);*/
	for(int i=0;i<10000;i++)
	{
		printf("******************try time %d\n",i);
        wav_path_test="device/babym021.wav";
	    val=sound.ValideSoundClassify(wav_path_test,soundclass);
	}

	/*wav_path_test="data/blue010.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="data/boom030.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="data/glass014.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="data/gun090.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);
	wav_path_test="data/scream013.wav";val=sound.ValideSoundClassify(wav_path_test,soundclass);*/
	
#endif

#ifdef INTERFACE1
	 char* wave=new char[2048];
	for(int i=0; i<2048;i++) 
	{
		wave[i]=250;i++;  wave[i]=127;}

	HumanDetHandle handle;
	handle=CreateSoundClassify(getans);
	for(int i=0;i<1000;i++)
	  ValideSoundClassify(handle,wave,2048);

#endif

	system("pause");
     return 0;
}


	/*
    double data[] = {
        0.0, 0.2, 0.4,
        0.3, 0.2, 0.4,
        0.4, 0.2, 0.4,
        0.5, 0.2, 0.4,
        5.0, 5.2, 8.4,
        6.0, 5.2, 7.4,
        4.0, 5.2, 4.4,
        10.3, 10.4, 10.5,
        10.1, 10.6, 10.7,
        11.3, 10.2, 10.9
    };
    const int size = 10; //Number of samples
    const int dim = 3;   //Dimension of feature

    const int cluster_num = 4; //Cluster number

	//1. 测试k-means算法

    KMeans* kmeans = new KMeans(dim,cluster_num);
    int* labels = new int[size];
    kmeans->SetInitMode(KMeans::InitUniform);
	kmeans->Cluster(data,size,labels);

	printf("Clustering result by k-meams:\n");
	for(int i = 0; i < size; ++i)     printf("%f, %f, %f belongs to %d cluster\n", data[i*dim+0], data[i*dim+1], data[i*dim+2], labels[i]);
	for(int i=0;i<cluster_num;i++)    printf("mean %f belong to %d cluster \n",*(kmeans->GetMean(i)),i);
	
	delete []labels;     //delete 数组
	delete kmeans;       //delete  对象
	*/

	/*
	//2. 测试GMM算法的训练
    double test_data[4][3] = {
        0.1, 0.2, 0.3,
        0.4, 0.5, 0.6,
        5.0, 6.2, 8.4,
        10.3, 10.4, 10.5
    };

    GMM *gmm = new GMM(dim,4); //GMM has 3 SGM   GMM的阶数必须小于训练样本的数量size，否则内部的k-means算法会报错
    gmm->Train(data,size); //Training GMM

    printf("\nTest GMM:\n");
    for(int i = 0; i < 4; ++i)
	{
	    printf("The Probability of %f, %f, %f  is %f \n",test_data[i][0],test_data[i][1],test_data[i][2],gmm->GetProbability(test_data[i]));
	}

    //save GMM to file
	ofstream gmm_file("gmm.txt");
	assert(gmm_file);
	gmm_file<<*gmm;
	gmm_file.close();

    delete gmm;

	

	//3. 测试已有GMM模型的概率识别
    GMM *gmm1= new GMM(dim,4);;
	ifstream gmm_file1("gmm.txt");
	assert(gmm_file1);
	gmm_file1>>*gmm1;
	gmm_file1.close();
	
	double test_data1[4][3] = {
		0.2, 0.3, 0.4,
		1.4, 1.5, 1.6,
		4.0, 5.2, 7.4,
		11.3, 11.4, 11.5
	};
	printf("\nTest GMM1:\n");
	for(int i = 0; i < 4; ++i)
	{
		printf("The Probability of %f, %f, %f  is %f \n",test_data1[i][0],test_data1[i][1],test_data1[i][2],gmm1->GetProbability(test_data1[i]));
	}

	delete gmm1;
	*/
