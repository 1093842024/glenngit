#include "extractfeature.h"
//#define CALCUTIME


double *extracttrain_mfcc(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature)
{
	dim=MFCCCOEF;
	feature="mfcc";
	string wav_path=wavpath;
	char wav_name[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	vector<float> wav_data_store;
	for(;wav_idx!=wav_num+1;wav_idx++)
	{
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		printf("start extracting of wav %d\n",wav_idx);
		Wave *w = filter_bank(wav);
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
		{
			wav_data_store.push_back(*(w->MFCCs + i));  //printf("%f ", *(w->MFCCs + i));
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);
	}  
	datasize=wav_data_store.size();
	printf("\ntotal datasize:%ld\n",datasize);
	double *wav_data = new double[datasize];
	for(int i=0;i<datasize;i++) wav_data[i]=wav_data_store[i];
	return wav_data;
}

double *extracttrain_mdiff(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature)
{
	dim=MFCCCOEF;
	feature="mdiff";
	string wav_path=wavpath;
	char wav_name[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	vector<float> wav_data_store;
	for(;wav_idx!=wav_num+1;wav_idx++)
	{
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		printf("start extracting of wav %d\n",wav_idx);
		Wave *w = filter_bank(wav);
		for (int i = MFCCCOEF*1; i < MFCCCOEF*(w->nRow-1); i++)   //输出diff特征，除掉前后各两帧
		{
			wav_data_store.push_back(*(w->DevMFCCs + i));
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);
	}  
	datasize=wav_data_store.size();
	printf("\ntotal datasize:%ld\n",datasize);
	double *wav_data = new double[datasize];
	for(int i=0;i<datasize;i++) wav_data[i]=wav_data_store[i];
	return wav_data;
}

double *extracttrain_mfcc_diff(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature)
{
	dim=MFCCCOEF*2;
	feature="mfcc_diff";
	string wav_path=wavpath;
	char wav_name[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	vector<float> wav_data_store;
	for(;wav_idx!=wav_num+1;wav_idx++)
	{
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		printf("start extracting of wav %d\n",wav_idx);
		Wave *w = filter_bank(wav);
		for (int i = MFCCCOEF; i < MFCCCOEF*(w->nRow-1); i++)   //输出了
		{
			if(i%(MFCCCOEF)==0)
			{
				for(int j=i;j<i+MFCCCOEF;j++)
				{
					wav_data_store.push_back(*(w->MFCCs + j));  
					//printf("%f ", *(w->MFCCs + j));
				}
				//printf("\n");
			}
			wav_data_store.push_back(*(w->DevMFCCs + i));
			//printf("%f ", *(w->DevMFCCs + i));
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);
	}  
	datasize=wav_data_store.size();
	printf("\ntotal datasize:%ld\n",datasize);
	double *wav_data = new double[datasize];
	for(int i=0;i<datasize;i++) wav_data[i]=wav_data_store[i];
	return wav_data;
}

double *extracttrain_mfcc_diff_valide(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature,bool output,string classname)
{
	dim=MFCCCOEF*2;
	feature="mfcc_diff";
	string wav_path=wavpath;
	char wav_name[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	vector<float> wav_data_store;
	for(;wav_idx!=wav_num+1;wav_idx++)
	{
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		printf("start extracting of wav %d\n",wav_idx);
		Wave *w = filter_bank(wav);
		for (int i = MFCCCOEF; i < MFCCCOEF*(w->nRow-1); i++)   //输出了
		{
			if(w->valideframe[i/MFCCCOEF]==1)
			{
			    if(i%(MFCCCOEF)==0)
			    {
				    for(int j=i;j<i+MFCCCOEF;j++)
				    {
				    	wav_data_store.push_back(*(w->MFCCs + j));  
					    //printf("%f ", *(w->MFCCs + j));
				    }
				    //printf("\n");
			    }
			    wav_data_store.push_back(*(w->DevMFCCs + i));
			  //printf("%f ", *(w->DevMFCCs + i));
		      }
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);
	}  
	datasize=wav_data_store.size();
	printf("\ntotal datasize:%ld\n",datasize);
	double *wav_data = new double[datasize];
	for(int i=0;i<datasize;i++) wav_data[i]=wav_data_store[i];
	if(output=true)
	{
		ofstream out_to_svm(wavpath);
		assert(out_to_svm);
		for(int j=0;j<datasize;j=j+26)
		{
			out_to_svm<<classname<<" ";
			for(int i=0;i<26;i++)
			{
				out_to_svm<<i+1<<":"<<wav_data[i+j]<<" ";
			}
			out_to_svm<<endl;
		}
		out_to_svm.close();
	}
	
	return wav_data;
}

void extracttrain_mfcc_diff_valide_singleoutput(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature,bool output,string classname)
{
	dim=MFCCCOEF*2;
	feature="mfcc_diff";
	string wav_path=wavpath;
	char wav_name[100];
	char wav_name2[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	for(;wav_idx!=wav_num+1;wav_idx++)
	{
        vector<float> wav_data_store;
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		sprintf_s(wav_name2, "%03d", wav_idx);   string full_wav_name2=wav_path+wav_name2;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		printf("start extracting of wav %d\n",wav_idx);
		Wave *w = filter_bank(wav);
		for (int i = MFCCCOEF; i < MFCCCOEF*(w->nRow-1); i++)   //输出了
		{
			if(w->valideframe[i/MFCCCOEF]==1)
			{
				if(i%(MFCCCOEF)==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						wav_data_store.push_back(*(w->MFCCs + j));  
						//printf("%f ", *(w->MFCCs + j));
					}
					//printf("\n");
				}
				wav_data_store.push_back(*(w->DevMFCCs + i));
				//printf("%f ", *(w->DevMFCCs + i));
			}
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);

        datasize=wav_data_store.size();

        if(output=true)
	    {
	  	    ofstream out_to_svm(full_wav_name2);
		    assert(out_to_svm);
		    for(int j=0;j<datasize;j=j+26)
		    {
			    out_to_svm<<classname<<" ";
			    for(int i=0;i<26;i++)
			    {
				    out_to_svm<<i+1<<":"<<wav_data_store[i+j]<<" ";
			    }
			    out_to_svm<<endl;
		    }
		    out_to_svm.close();
	    }
		wav_data_store.clear();
	}  
	
}

double *extracttrain_mfcc_energy(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature)
{
	dim=MFCCCOEF+1;
	feature="mfcc_energy";
	string wav_path=wavpath;
	char wav_name[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	vector<float> wav_data_store;
	for(;wav_idx!=wav_num+1;wav_idx++)
	{
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		printf("start extracting of wav %d\n",wav_idx);
		Wave *w = filter_bank(wav);
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
		{
			if (i%MFCCCOEF == 0)
			{
				wav_data_store.push_back(*(w->ShortEnergy + i / MFCCCOEF));  //printf("\n第%d帧：\n", i / MFCCCOEF), printf("short energy:%f\n", *(w->ShortEnergy + i / MFCCCOEF)),printf("MFCCs: \n"); 
			}
			wav_data_store.push_back(*(w->MFCCs + i));  //printf("%f ", *(w->MFCCs + i));
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);
	}  
	datasize=wav_data_store.size();
	printf("\ntotal datasize:%ld\n",datasize);
	double *wav_data = new double[datasize];
	for(int i=0;i<datasize;i++) wav_data[i]=wav_data_store[i];
    return wav_data;
}

double *extracttrain_mfcc_energy_valide(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature)
{
	dim=MFCCCOEF+1;
	feature="mfcc_energy";
	string wav_path=wavpath;
	char wav_name[100];
	int wav_idx=1;
	const int wav_num=wavnum;

	vector<float> wav_data_store;
	for(;wav_idx!=wav_num+1;wav_idx++)
	{
		//clockBegin = clock();
		sprintf_s(wav_name, "%03d.wav", wav_idx);   string full_wav_name=wav_path+wav_name;
		char wav[100];   strcpy(wav,full_wav_name.c_str());
		Wave *w = filter_bank(wav);
        printf("start extracting of wav %d,init row:%d\n",wav_idx,w->nRow);
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
		{
			if(w->valideframe[i/MFCCCOEF]==1)
           {
			   if (i%MFCCCOEF == 0)
			  {
				wav_data_store.push_back(*(w->ShortEnergy + i / MFCCCOEF));  //printf("\n第%d帧：\n", i / MFCCCOEF), printf("short energy:%f\n", *(w->ShortEnergy + i / MFCCCOEF)),printf("MFCCs: \n"); 
			  }
			  wav_data_store.push_back(*(w->MFCCs + i));  //printf("%f ", *(w->MFCCs + i));
			}
		}
		//clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
		deletewav(w);
	}  
	datasize=wav_data_store.size();
	printf("\ntotal datasize:%ld, valide row:%d\n",datasize,datasize/dim);
	double *wav_data = new double[datasize];
	for(int i=0;i<datasize;i++) wav_data[i]=wav_data_store[i];
	return wav_data;
}

void train_save_gmm(const int dim, const int gmm_mix_num, const int size, double *wav_data, string gmm_txt)
{
	GMM *gmm = new GMM(dim,gmm_mix_num);
	gmm->Train(wav_data,size); //Training GMM
	ofstream gmm_file(gmm_txt);	//save GMM to file
	assert(gmm_file);
	gmm_file<<*gmm;
	gmm_file.close();
	delete gmm;
	delete wav_data;
}

GMM* loadgmm(const int dim, const int gmm_mix_num, string load_gmm_struct)
{
	char filename[100];
	strcpy(filename,load_gmm_struct.c_str());
	GMM *gmm1= new GMM(dim,gmm_mix_num);;
	ifstream gmm_file1(filename);
	assert(gmm_file1);
	gmm_file1>>*gmm1;
	gmm_file1.close();
	return gmm1;
}

void testwavs(string wavpath,const int wavnum,GMM *gmm,const int dim,const string feature)
{
#ifdef CALCUTIME
	clock_t clockBegin,clockEnd; int time;
#endif
	string wav_path_test=wavpath;
	const int wav_num_test=wavnum;
	char wav_name_test[100];	int wav_idx_test=1;
	int fitnum1=0,fitnum2=0,fitnum3=0;
	for(;wav_idx_test!=wav_num_test+1;wav_idx_test++)
	{
#ifdef CALCUTIME
		clockBegin = clock();
#endif
		sprintf_s(wav_name_test, "%03d.wav", wav_idx_test);    string full_wav_name_test=wav_path_test+wav_name_test;
		char wav[100];    strcpy(wav,full_wav_name_test.c_str());
		Wave *w = filter_bank(wav);

		double **alldata1=new double *[w->nRow]; //提取测试音频的特征数据，分配参数的空间
		for(int i=0;i<w->nRow;i++)  alldata1[i]=new double[dim];

		if(feature=="mfcc_energy")
		{
             for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
		     {
				if (i%MFCCCOEF == 0)      alldata1[i / MFCCCOEF][0]=*(w->ShortEnergy + i / MFCCCOEF);
			    alldata1[i / MFCCCOEF][i%MFCCCOEF+1]=*(w->MFCCs + i);
             }
		}else if(feature=="mfcc")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->MFCCs + i);
			}
		}else if(feature=="mdiff")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->DevMFCCs + i);
			}
		}else if(feature=="mfcc_diff")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
			{
				if(i%MFCCCOEF==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						alldata1[i / MFCCCOEF][j%MFCCCOEF]=*(w->MFCCs + j);  	
					}
				}
				alldata1[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=*(w->DevMFCCs + i);
			}
		}else 
		{
			printf("no such features!\n");  _sleep(5000);  exit(0);
		}

		double probmax=0,prob,proball=0;  int num=0;
        double *singledata=new double[dim];
		for(int i=0;i<w->nRow;i++)                    //计算音频对于该模型的匹配程度
		{   
			for(int j=0;j<dim;j++) singledata[j]=alldata1[i][j];
			prob=PLUSFACTOR*gmm->GetProbability(singledata);  //printf("The Probability of %f, %f, %f  is %f \n",singledata[0],singledata[1],singledata[2],prob);
			proball+=prob;
			if(prob>probmax) probmax=prob;
		}
		probmax=log10(probmax);proball=log10(proball*FRAMENUM/w->nRow);
		printf("wave:%d  row:%d  maxprobability: %.3lf,allprob: %.2lf   \n",wav_idx_test,w->nRow,probmax,proball);
		
		/*//14维度mfcc+ene blue                              s-65535-1              语音帧训练的结果
		if(proball>-0.5 &&probmax>-1.5) fitnum1++;           //100%  5%             96.5%   5%
		if(proball>-0.3 &&probmax>-1) fitnum2++;            //85%  4%               96.5%   3%
		if(proball>0 &&probmax>-1)fitnum3++;               //85%   0%               90%     0%
		*/

		/*//26维度mfcc  blue                                 s-65535-1               语音帧训练的结果
		if(proball>-13 &&probmax>-14) fitnum1++;             //100%   5%            100%     0%
		if(proball>-12 &&probmax>-13) fitnum2++;             //96%    3%            100%     0%
		if(proball>-11 &&probmax>-12)fitnum3++;              //89%    0%            100%     0%
		*/


		/*//14维度mfcc+ene baby                           s-65535-1                    语音帧训的结果
		if(proball>-4 &&probmax>-5) fitnum1++;             //   80%   70%              75%     70%
		if(proball>-3.5 &&probmax>-4) fitnum2++;           //   54%   45%              60%     50%
		if(proball>-3 &&probmax>-3.5)fitnum3++;            //   14%   10%              23%     30%
		*/

		/*//26维度mfcc baby                           s-65535-1 全部语音训的结果         语音帧训的结果
		if(proball>-22 &&probmax>-23) fitnum1++;            //98%   50%                96%      35%
		if(proball>-21 &&probmax>-22) fitnum2++;            //84%   28%                84%      23%
		if(proball>-20 &&probmax>-20)fitnum3++;             //71%   10%                67%      2%
		*/

//26维度mfcc                                baby       percent     boom    percent      glass       percent       gun        percent     scream    percent
if(proball>-21 &&probmax>-22) fitnum1++;//-22  -21    89%  15%   -17 -16  89%  15%   -17.5 -16.5    94%   7%   -18 -17       78%   15%   -21 -20   85%  8%                  
if(proball>-20 &&probmax>-21) fitnum2++;//-21  -20    78%  5%    -16 -15  86%   5%   -17    -16     86%   2%   -17.5 -16.5   72%  7%     -20 -19   79%  2%                           
if(proball>-19 &&probmax>-20)fitnum3++; //-20  -19    69%  2%    -15 -14  74%   2%   -16.5  -15.5   75%   1%   -17 -16       63%   4%    -19 -18   71%  0%                     

//if(proball>-20 &&probmax>-21) printf("valide wav %d\n",wav_idx_test);
		delete singledata;
		for(int i=0; i<w->nRow;i++) 
			delete []alldata1[i];
		delete alldata1;

		deletewav(w);
#ifdef CALCUTIME
		clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
#endif
	}
	printf("\nFit1 percentage is %.2f,Fit2 percentage is %.2f,Fit3 percentage is %.2f\n",fitnum1*100.0/wav_num_test,fitnum2*100.0/wav_num_test,fitnum3*100.0/wav_num_test);
}

void testwavs_valide(string wavpath,const int wavnum,GMM *gmm,const int dim,const string feature)
{
#ifdef CALCUTIME
	clock_t clockBegin,clockEnd; int time;
#endif
	string wav_path_test=wavpath;
	const int wav_num_test=wavnum;
	char wav_name_test[100];	int wav_idx_test=1;
	int fitnum1=0,fitnum2=0,fitnum3=0;
	int validenum=0;
	for(;wav_idx_test!=wav_num_test+1;wav_idx_test++)
	{
#ifdef CALCUTIME
		clockBegin = clock();
#endif
		sprintf_s(wav_name_test, "%03d.wav", wav_idx_test);    string full_wav_name_test=wav_path_test+wav_name_test;
		char wav[100];    strcpy(wav,full_wav_name_test.c_str());
		Wave *w = filter_bank(wav);

		for(int i=0;i<w->nRow;i++)
		{
			if(w->valideframe[i]==1) validenum++;
		}

		double **alldata1=new double *[w->nRow]; //提取测试音频的特征数据，分配参数的空间
		for(int i=0;i<w->nRow;i++)  alldata1[i]=new double[dim];

		if(feature=="mfcc_energy")
		{
             for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
		     {
				 if(w->valideframe[i/MFCCCOEF]==1)
				 { 
				    if (i%MFCCCOEF == 0)      alldata1[i / MFCCCOEF][0]=*(w->ShortEnergy + i / MFCCCOEF);
			        alldata1[i / MFCCCOEF][i%MFCCCOEF+1]=*(w->MFCCs + i);
				 }else 
				 {
					 if (i%MFCCCOEF == 0)      alldata1[i / MFCCCOEF][0]=0;
					 alldata1[i / MFCCCOEF][i%MFCCCOEF+1]=0;
				 }
             }
		}else if(feature=="mfcc")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				if(w->valideframe[i/MFCCCOEF]==1)
				{
				    alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->MFCCs + i);
				}
				else 
				{
					alldata1[i / MFCCCOEF][i%MFCCCOEF]=0;
				}
			}
		}else if(feature=="mdiff")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				if(w->valideframe[i/MFCCCOEF]==1)
				{
				    alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->DevMFCCs + i);
				}else 
				{
					alldata1[i / MFCCCOEF][i%MFCCCOEF]=0;
				}
			}
		}else if(feature=="mfcc_diff")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
			{
				if(w->valideframe[i/MFCCCOEF]==1)
				{
			  	    if(i%MFCCCOEF==0)
				    {
					    for(int j=i;j<i+MFCCCOEF;j++)
					    {
					  	    alldata1[i / MFCCCOEF][j%MFCCCOEF]=*(w->MFCCs + j);  	
					    }
				    }
				    alldata1[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=*(w->DevMFCCs + i);
				}else 
				{
					if(i%MFCCCOEF==0)
					{
						for(int j=i;j<i+MFCCCOEF;j++)
						{
							alldata1[i / MFCCCOEF][j%MFCCCOEF]=0;  	
						}
					}
					alldata1[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=0;
				}
			}
		}else 
		{
			printf("no such features!\n");  _sleep(5000); exit(0);
		}

		double probmax=0,prob,proball=0;  int num=0;
        double *singledata=new double[dim];
		for(int i=0;i<w->nRow;i++)                    //计算音频对于该模型的匹配程度
		{   
			if(w->valideframe[i]==1)
			{
                for(int j=0;j<dim;j++) singledata[j]=alldata1[i][j];
			    prob=PLUSFACTOR*gmm->GetProbability(singledata);  //printf("The Probability of %f, %f, %f  is %f \n",singledata[0],singledata[1],singledata[2],prob);
			    proball+=prob;
			    if(prob>probmax) probmax=prob;
			}
		}
		probmax=log10(probmax);proball=log10(proball*FRAMENUM/(validenum+1));
		printf("wave:%d  row:%d  maxprobability: %.3lf,allprob: %.2lf   \n",wav_idx_test,w->nRow,probmax,proball);
		
		/*//14维度mfcc+ene blue                              s-65535-1
		if(proball>-0.5 &&probmax>-1.5) fitnum1++;           //100%  5%
		if(proball>-0.3 &&probmax>-1) fitnum2++;            //85%  4%
		if(proball>0 &&probmax>-1)fitnum3++;               //85%   0%
		*/

		/*//26维度mfcc  blue                                 s-65535-1
		if(proball>-13 &&probmax>-14) fitnum1++;             //100%   5%
		if(proball>-12 &&probmax>-13) fitnum2++;             //96%    3%
		if(proball>-11 &&probmax>-12)fitnum3++;              //89%    0%
		*/


		/*//14维度mfcc+ene baby                           s-65535-1   
		if(proball>-5 &&probmax>-6) fitnum1++;             //   80%   70%
		if(proball>-4 &&probmax>-5) fitnum2++;           //   54%   45%
		if(proball>-3 &&probmax>-3.5)fitnum3++;            //   14%   10%
		*/

		
		//26维度mfcc baby                          s-65535-1  全部语音训的模型结果         语音帧训的模型结果
		if(proball>-22 &&probmax>-23) fitnum1++;            //73%   20%                    79%       15% 
		if(proball>-21 &&probmax>-22) fitnum2++;            //59%   10%                    70%       7%
		if(proball>-20 &&probmax>-20)fitnum3++;             //53%   2%                     59%       2%
		


		delete singledata;
		for(int i=0; i<w->nRow;i++) 
			delete []alldata1[i];
		delete alldata1;

		deletewav(w);
#ifdef CALCUTIME
		clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
#endif
	}
	printf("\nFit1 percentage is %.2f,Fit2 percentage is %.2f,Fit3 percentage is %.2f\n",fitnum1*100.0/wav_num_test,fitnum2*100.0/wav_num_test,fitnum3*100.0/wav_num_test);
}

void testsinglewav(string wavpath,GMM *gmm,int dim,const string feature,double &averprob,double &maxprob)
{
#ifdef CALCUTIME
	clock_t clockBegin,clockEnd; int time;
	clockBegin = clock();
#endif
	    string wav_path_test=wavpath;
		char wav[100];    strcpy(wav,wav_path_test.c_str());
		printf("\nstart extracting features \n",wavpath);
		Wave *w = filter_bank(wav);

		double **alldata1=new double *[w->nRow]; //提取测试音频的特征数据，分配参数的空间
		for(int i=0;i<w->nRow;i++)  alldata1[i]=new double[dim];

		if(feature=="mfcc_energy")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				if (i%MFCCCOEF == 0)      alldata1[i / MFCCCOEF][0]=*(w->ShortEnergy + i / MFCCCOEF);
				alldata1[i / MFCCCOEF][i%MFCCCOEF+1]=*(w->MFCCs + i);
			}
		}else if(feature=="mfcc")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->MFCCs + i);
			}
		}else if(feature=="mdiff")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{

				alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->DevMFCCs + i);
			}
		}else if(feature=="mfcc_diff")
		{
			for (int i =0; i < MFCCCOEF*w->nRow; i++)   //输出了
			{
				if(i%MFCCCOEF==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						alldata1[i / MFCCCOEF][j%MFCCCOEF]=*(w->MFCCs + j);  
					}
				}
				alldata1[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=*(w->DevMFCCs + i);
			}
		}else 
		{ 
			printf("no such features!\n");   _sleep(5000);  exit(0);
		}

		double probmax=0,prob,proball=0; 
		double *singledata=new double[dim];
		for(int i=0;i<w->nRow;i++)                    //计算音频对于该模型的匹配程度
		{
			for(int j=0;j<dim;j++) singledata[j]=alldata1[i][j];
			prob=PLUSFACTOR*gmm->GetProbability(singledata);  //printf("The Probability of %f, %f, %f  is %f \n",singledata[0],singledata[1],singledata[2],prob);
			proball+=prob;
			if(prob>probmax) probmax=prob;
		}
		probmax=log10(probmax);proball=log10(proball*FRAMENUM/w->nRow);
		printf("max probability is %.3lf,all prob is %.2lf  \n",probmax,proball);

		delete singledata;
		for(int i=0; i<w->nRow;i++) 
			delete []alldata1[i];
		delete alldata1;
		deletewav(w);

		averprob=proball; maxprob=probmax;
#ifdef CALCUTIME
		clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
#endif
}

void testsinglewav_valide(string wavpath,GMM *gmm,int dim,const string feature,double &averprob,double &maxprob)
{
#ifdef CALCUTIME
	clock_t clockBegin,clockEnd; int time;
	clockBegin = clock();
#endif
	string wav_path_test=wavpath;
	char wav[100];    strcpy(wav,wav_path_test.c_str());
	printf("\nstart extracting features \n",wavpath);
	Wave *w = filter_bank(wav);
	
	int validenum=0;
	for(int i=0;i<w->nRow;i++)   if(w->valideframe[i]==1) validenum++;
	

	double **alldata1=new double *[w->nRow]; //提取测试音频的特征数据，分配参数的空间
	for(int i=0;i<w->nRow;i++)  alldata1[i]=new double[dim];

	if(feature=="mfcc_energy")
	{
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
		{
			if(w->valideframe[i/MFCCCOEF]==1)
			{ 
				if (i%MFCCCOEF == 0)      alldata1[i / MFCCCOEF][0]=*(w->ShortEnergy + i / MFCCCOEF);
				alldata1[i / MFCCCOEF][i%MFCCCOEF+1]=*(w->MFCCs + i);
			}else 
			{
				if (i%MFCCCOEF == 0)      alldata1[i / MFCCCOEF][0]=0;
				alldata1[i / MFCCCOEF][i%MFCCCOEF+1]=0;
			}
		}
	}else if(feature=="mfcc")
	{
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
		{
			if(w->valideframe[i/MFCCCOEF]==1)
			{
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->MFCCs + i);
			}
			else 
			{
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=0;
			}
		}
	}else if(feature=="mdiff")
	{
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
		{
			if(w->valideframe[i/MFCCCOEF]==1)
			{
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=*(w->DevMFCCs + i);
			}else 
			{
				alldata1[i / MFCCCOEF][i%MFCCCOEF]=0;
			}
		}
	}else if(feature=="mfcc_diff")
	{
		for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
		{
			if(w->valideframe[i/MFCCCOEF]==1)
			{
				if(i%MFCCCOEF==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						alldata1[i / MFCCCOEF][j%MFCCCOEF]=*(w->MFCCs + j);  	
					}
				}
				alldata1[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=*(w->DevMFCCs + i);
			}else 
			{
				if(i%MFCCCOEF==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						alldata1[i / MFCCCOEF][j%MFCCCOEF]=0;  	
					}
				}
				alldata1[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=0;
			}
		}
	}else 
	{
		printf("no such features!\n");_sleep(5000);exit(0);
	}

	double probmax=0,prob,proball=0; 
	double *singledata=new double[dim];
	for(int i=0;i<w->nRow;i++)                    //计算音频对于该模型的匹配程度
	{
		if(w->valideframe[i]==1)
		{
		    for(int j=0;j<dim;j++) singledata[j]=alldata1[i][j];
		    prob=PLUSFACTOR*gmm->GetProbability(singledata);  //printf("The Probability of %f, %f, %f  is %f \n",singledata[0],singledata[1],singledata[2],prob);
		    proball+=prob;
		    if(prob>probmax) probmax=prob;
		}
	}
	probmax=log10(probmax);proball=log10(proball*FRAMENUM/(validenum+1));
	printf("all frame: %d, valide framenue: %d, max probability: %.3lf,aver prob: %.2lf \n",w->nRow,validenum,probmax,proball);

	delete singledata;
	for(int i=0; i<w->nRow;i++) 
		delete []alldata1[i];
	delete alldata1;
	deletewav(w);

	averprob=proball; maxprob=probmax;
#ifdef CALCUTIME
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("\nextraction time: %dms\n", time);
#endif
}