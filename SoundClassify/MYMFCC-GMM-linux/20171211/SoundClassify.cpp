#include "SoundClassify.h"

void SoundClassify::InitGMM_Models(int Model_num)
{
	GMM_Model gmmmodel;
	for(int i=0;i<Model_num;i++)
	{
	    if(i==0)
		{
			gmmmodel.GMM_Num=MODEL1;
			gmmmodel.GMM_Name="babycring";
			gmmmodel.GMM_txt="gmm_wav_baby_valide_8khz.txt";
			gmmmodel.Feature_string="mfcc_diff";
			gmmmodel.Dim=26;
			gmmmodel.GMM_Mix_Num=32;
			gmmmodel.MaxProb=-22;
			gmmmodel.AverAllProb=-21;
			gmmmodel.Maxprobstrict=-20;
			gmmmodel.AverAllProbstrict=-19;
			Gmm_model.push_back(gmmmodel);
			//GMM *gmm=new GMM;
			Gmm.insert(std::pair<std::string,GMM*>(gmmmodel.GMM_txt,loadgmm(gmmmodel.Dim,gmmmodel.GMM_Mix_Num,gmmmodel.GMM_txt)));
		}else if(i==1){
			gmmmodel.GMM_Num=MODEL2;
			gmmmodel.GMM_Name="boom";
			gmmmodel.GMM_txt="gmm_wav_boom_valide_8khz.txt";
			gmmmodel.Feature_string="mfcc_diff";
			gmmmodel.Dim=26;
			gmmmodel.GMM_Mix_Num=32;
			gmmmodel.MaxProb=-17;
			gmmmodel.AverAllProb=-16;
			gmmmodel.Maxprobstrict=-15;
			gmmmodel.AverAllProbstrict=-14;
			Gmm_model.push_back(gmmmodel);
			Gmm.insert(std::pair<std::string,GMM*>(gmmmodel.GMM_txt,loadgmm(gmmmodel.Dim,gmmmodel.GMM_Mix_Num,gmmmodel.GMM_txt)));
		}else if(i==2){
			gmmmodel.GMM_Num=MODEL3;
			gmmmodel.GMM_Name="glass";
			gmmmodel.GMM_txt="gmm_wav_glass_valide_8khz.txt";
			gmmmodel.Feature_string="mfcc_diff";
			gmmmodel.Dim=26;
			gmmmodel.GMM_Mix_Num=32;
			gmmmodel.MaxProb=-17.5;
			gmmmodel.AverAllProb=-16.5;
			gmmmodel.Maxprobstrict=-16.5;
			gmmmodel.AverAllProbstrict=-15.5;
			Gmm_model.push_back(gmmmodel);
			Gmm.insert(std::pair<std::string,GMM*>(gmmmodel.GMM_txt,loadgmm(gmmmodel.Dim,gmmmodel.GMM_Mix_Num,gmmmodel.GMM_txt)));
		}
		else if(i==3){
			gmmmodel.GMM_Num=MODEL4;
			gmmmodel.GMM_Name="gun";
			gmmmodel.GMM_txt="gmm_wav_gun_valide_8khz.txt";
			gmmmodel.Feature_string="mfcc_diff";
			gmmmodel.Dim=26;
			gmmmodel.GMM_Mix_Num=32;
			gmmmodel.MaxProb=-18;
			gmmmodel.AverAllProb=-17;
			gmmmodel.Maxprobstrict=-17.5;
			gmmmodel.AverAllProbstrict=-16.5;
			Gmm_model.push_back(gmmmodel);
			Gmm.insert(std::pair<std::string,GMM*>(gmmmodel.GMM_txt,loadgmm(gmmmodel.Dim,gmmmodel.GMM_Mix_Num,gmmmodel.GMM_txt)));
		}
		else if(i==4){
			gmmmodel.GMM_Num=MODEL5;
			gmmmodel.GMM_Name="scream";
			gmmmodel.GMM_txt="gmm_wav_scream_valide_8khz.txt";
			gmmmodel.Feature_string="mfcc_diff";
			gmmmodel.Dim=26;
			gmmmodel.GMM_Mix_Num=32;
			gmmmodel.MaxProb=-21;
			gmmmodel.AverAllProb=-20;
			gmmmodel.Maxprobstrict=-19;
			gmmmodel.AverAllProbstrict=-18;
			Gmm_model.push_back(gmmmodel);
		    Gmm.insert(std::pair<std::string,GMM*>(gmmmodel.GMM_txt,loadgmm(gmmmodel.Dim,gmmmodel.GMM_Mix_Num,gmmmodel.GMM_txt)));
		}
		else if(i==5){
			gmmmodel.GMM_Num=MODEL6;
			gmmmodel.GMM_Name="blue";
			gmmmodel.GMM_txt="gmm_wav_blue_valide.txt";
			gmmmodel.Feature_string="mfcc_diff";
			gmmmodel.Dim=26;
			gmmmodel.GMM_Mix_Num=32;
			gmmmodel.MaxProb=-14;
			gmmmodel.AverAllProb=-13;
			gmmmodel.Maxprobstrict=-13;
			gmmmodel.AverAllProbstrict=-12;
			Gmm_model.push_back(gmmmodel);
			Gmm.insert(std::pair<std::string,GMM*>(gmmmodel.GMM_txt,loadgmm(gmmmodel.Dim,gmmmodel.GMM_Mix_Num,gmmmodel.GMM_txt)));
		}
	}
}

int SoundClassify::ValideSoundClassify(const char *wavdata,int len, int soundtype)
{
	if(len%2!=0||len<512) {printf("sample dots nums are odd\n");return 0;}
	//载入音频，提取各种特征参数
	Wave *w = filter_bank(wavdata,soundtype,len);

	vector<GMM_Model>::iterator gmmnow=Gmm_model.begin();  vector<GMM_Model>::iterator gmmend=Gmm_model.end();
	for(;gmmnow!=gmmend;gmmnow++)
	{
		int dim=gmmnow->Dim;
		string feature=gmmnow->Feature_string;
		int mix_num=gmmnow->GMM_Mix_Num;
		string gmm_struct=gmmnow->GMM_txt;

		//根据数据库中的各个模型，提取需要的特征数据
		double **alldata=new double *[w->nRow];                      //提取测试音频的特征数据，分配参数的空间
		for(int i=0;i<w->nRow;i++)  alldata[i]=new double[dim];

		if(feature=="mfcc_energy")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				if (i%MFCCCOEF == 0)      alldata[i / MFCCCOEF][0]=*(w->ShortEnergy + i / MFCCCOEF);
				alldata[i / MFCCCOEF][i%MFCCCOEF+1]=*(w->MFCCs + i);
			}
		}else if(feature=="mfcc_diff")
		{
			for (int i =0; i < MFCCCOEF*w->nRow; i++)   
			{
				if(i%MFCCCOEF==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						alldata[i / MFCCCOEF][j%MFCCCOEF]=*(w->MFCCs + j);  
					}
				}
				alldata[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=*(w->DevMFCCs + i);
			}
		}else if(feature=="mfcc")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				alldata[i / MFCCCOEF][i%MFCCCOEF]=*(w->MFCCs + i);
			}
		}else 
		{ 
			printf("no such features!\n");  return -1; //返回-1表示发生了错误，数据库中没有对应的特征类型 
		}

		//载入该模型
		std::map<std::string,GMM*>::iterator finditor=Gmm.find(gmm_struct);
		if(finditor==Gmm.end()){ _funcb(0,0);return 0;}
		GMM* gmm=finditor->second;

		//计算该模型的评分
		double prob,proball=0,probmax=0; 
		double *singledata=new double[dim];
		for(int i=0;i<w->nRow;i++)                    //计算音频对于该模型的匹配程度
		{
			for(int j=0;j<dim;j++) singledata[j]=alldata[i][j];
			prob=PLUSFACTOR*gmm->GetProbability(singledata);  //printf("The Probability of %f, %f, %f  is %f \n",singledata[0],singledata[1],singledata[2],prob);
			proball+=prob;
			if(prob>probmax) probmax=prob;
		}
		if(probmax>1E-50)probmax=log10(probmax);
		else probmax=-50;
		if(proball*FRAMENUM/w->nRow>1E-50) proball=log10(proball*FRAMENUM/w->nRow);
		else proball=-50;

		gmmnow->sampleMax=probmax;
		gmmnow->sampleAverall=proball;

		delete singledata;
		for(int i=0; i<w->nRow;i++) 
			delete []alldata[i];
		delete alldata;
	}
	deletewav(w);

	CalculateGoal();
	int classnum;
	if(valide_model.size()!=0)
	{
		GetClassNum(classnum);
		_funcb(validelevel,classnum);
		return validelevel;  //返回1,认为至少找到了一个匹配的类
	}else { 
		classnum=0;
#ifdef SHOWTEST
		printf("no valide class\n");
#endif
		_funcb(validelevel,classnum);
		return validelevel;    //返回0，表示没有找到匹配类型
	}
}

int SoundClassify::ValideSoundClassify(const string soundpath,vector<string> &classname,int soundtype)  //default soundtype=0 means "*.wav" audio
{
	//载入音频，提取各种特征参数
	string wav_path_test=soundpath;
	char wav[100];    strcpy(wav,wav_path_test.c_str());    //printf("\nstart extracting features \n",wavpath);

#ifdef TESTTIME
	double t,time_profile_counter;
	t = cvGetTickCount();
	time_profile_counter = (double)cvGetTickCount();
#endif
	Wave *w = filter_bank(wav,soundtype);
#ifdef TESTTIME
	time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
	std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
#endif
   

	vector<GMM_Model>::iterator gmmnow=Gmm_model.begin();  vector<GMM_Model>::iterator gmmend=Gmm_model.end();
    for(;gmmnow!=gmmend;gmmnow++)
	{
		int dim=gmmnow->Dim;
		string feature=gmmnow->Feature_string;
		int mix_num=gmmnow->GMM_Mix_Num;
		string gmm_struct=gmmnow->GMM_txt;

		//根据数据库中的各个模型，提取需要的特征数据
		double **alldata=new double *[w->nRow];                      //提取测试音频的特征数据，分配参数的空间
		for(int i=0;i<w->nRow;i++)  alldata[i]=new double[dim];

		if(feature=="mfcc_energy")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				if (i%MFCCCOEF == 0)      alldata[i / MFCCCOEF][0]=*(w->ShortEnergy + i / MFCCCOEF);
				alldata[i / MFCCCOEF][i%MFCCCOEF+1]=*(w->MFCCs + i);
			}
		}else if(feature=="mfcc_diff")
		{
			for (int i =0; i < MFCCCOEF*w->nRow; i++)   
			{
				if(i%MFCCCOEF==0)
				{
					for(int j=i;j<i+MFCCCOEF;j++)
					{
						alldata[i / MFCCCOEF][j%MFCCCOEF]=*(w->MFCCs + j);  
					}
				}
				alldata[i / MFCCCOEF][i%MFCCCOEF+MFCCCOEF]=*(w->DevMFCCs + i);
			}
		}else if(feature=="mfcc")
		{
			for (int i = 0; i < MFCCCOEF*w->nRow; i++)       //存储特征数据
			{
				alldata[i / MFCCCOEF][i%MFCCCOEF]=*(w->MFCCs + i);
			}
		}else 
		{ 
			printf("no such features!\n");  return -1; //返回-1表示发生了错误，数据库中没有对应的特征类型 
		}

		//载入该模型
		std::map<std::string,GMM*>::iterator finditor=Gmm.find(gmm_struct);
		if(finditor==Gmm.end()) { _funcb(0,0);return 0;}
		GMM* gmm=finditor->second;

		//计算该模型的评分
		double prob,proball=0,probmax=0; 
		double *singledata=new double[dim];
		for(int i=0;i<w->nRow;i++)                    //计算音频对于该模型的匹配程度
		{
			for(int j=0;j<dim;j++) singledata[j]=alldata[i][j];
			prob=PLUSFACTOR*gmm->GetProbability(singledata);  //printf("The Probability of %f, %f, %f  is %f \n",singledata[0],singledata[1],singledata[2],prob);
			proball+=prob;
			if(prob>probmax) probmax=prob;
		}
		if(probmax>1E-50)probmax=log10(probmax);
		else probmax=-50;
		if(proball*FRAMENUM/w->nRow>1E-50) proball=log10(proball*FRAMENUM/w->nRow);
		else proball=-50;

		gmmnow->sampleMax=probmax;
		gmmnow->sampleAverall=proball;

		delete singledata;
		for(int i=0; i<w->nRow;i++) 
			delete []alldata[i];
		delete alldata;
	}
	deletewav(w);

	CalculateGoal();
	int classnum;
	if(valide_model.size()!=0)
	{
	    GetClassName(classname);
		GetClassNum(classnum);
		_funcb(validelevel,classnum);
        return validelevel;  //返回1,认为至少找到了一个匹配的类
	}else { 
		classnum=0;
		classname.clear();
#ifdef SHOWTEST
		printf("no valide class\n");
#endif
		_funcb(validelevel,classnum);
		return validelevel;    //返回0，表示没有找到匹配类型
	}
}

void SoundClassify::CalculateGoal()
{
	validelevel=0;
	vector<GMM_Model> highgoal;
	vector<GMM_Model> lowgoal;
	for(vector<GMM_Model>::iterator gmmnow=Gmm_model.begin();gmmnow!=Gmm_model.end();gmmnow++)
	{
		if(gmmnow->sampleAverall>gmmnow->AverAllProbstrict&&gmmnow->sampleMax>gmmnow->Maxprobstrict) highgoal.push_back(*gmmnow);
		else if(gmmnow->sampleAverall>gmmnow->AverAllProb&&gmmnow->sampleMax>gmmnow->MaxProb) lowgoal.push_back(*gmmnow);
	}
	 if (highgoal.size()>=1) valide_model.assign(highgoal.begin(),highgoal.end()),validelevel=2;
	 else if(lowgoal.size()>=1) valide_model.assign(lowgoal.begin(),lowgoal.end()),validelevel=1;
     else  valide_model.clear();
}

void SoundClassify::GetClassName(vector<string> &classname)
{
	classname.clear();
#ifdef SHOWTEST
	int n=valide_model.size(); printf("Total valide num:%d\n",n);
#endif
	for(vector<GMM_Model>::iterator it=valide_model.begin();it!=valide_model.end();it++)
	{
		classname.push_back(it->GMM_Name);
#ifdef SHOWTEST
		printf("Valide level:%d, class of this audio includes --- model_num: %d  model_name: '%s'\n",validelevel,it->GMM_Num,it->GMM_Name.c_str());
#endif	
	}
}

void SoundClassify::GetClassNum(int& classnum)
{
	int num=0x00;
#ifdef SHOWTEST
	//int n=valide_model.size();printf("Total valide num:%d\n",n);
#endif
	for(vector<GMM_Model>::iterator it=valide_model.begin();it!=valide_model.end();it++)
	{
		num = num | it->GMM_Num;
#ifdef SHOWTEST
		//printf("Valide level:%d, class of this audio includes --- model_num: %d  model_name: '%s'\n",validelevel,it->GMM_Num,it->GMM_Name.c_str());
#endif	
	}
    classnum=num;
}

int SoundClassify::AddNewGMMModel(GMM_Model model)
{
	int i=0;
	for(vector<GMM_Model>::iterator it=Gmm_model.begin();it!=Gmm_model.end();it++)
	{
	    if(it->GMM_Num==model.GMM_Num) i=1;
	}
	if(i==0) 
	{
		Gmm_model.push_back(model); return 1;
	}else return 0;
}

int SoundClassify::ChangeGMMModel(GMM_Model model,int Gmmnum)
{
	int i=0;
	for(vector<GMM_Model>::iterator it=Gmm_model.begin();it!=Gmm_model.end();it++)
	{
	    if(it->GMM_Num==Gmmnum)
		{
		    it->AverAllProb=model.AverAllProb;
			it->AverAllProbstrict=model.AverAllProbstrict;
			it->Dim=model.Dim;
			it->Feature_string=model.Feature_string;
			it->GMM_Mix_Num=model.GMM_Mix_Num;
			it->GMM_Name=model.GMM_Name;
            it->GMM_txt=model.GMM_txt;
			it->MaxProb=model.MaxProb;
			it->Maxprobstrict=model.Maxprobstrict;
			it->sampleAverall=model.sampleAverall;
			it->sampleMax=model.sampleMax;
			i=1;
		}
	}
	return i;
}
