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
			GMM *gmm=new GMM;
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

int SoundClassify::ValideSoundClassify(const string soundpath,vector<string> &classname,int &classnum,int soundtype)  //default soundtype=0 means "*.wav" audio
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
	std::cout << "extrack feature time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
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

#ifdef TESTTIME
        clock_t clockBegin,clockEnd,time;
		clockBegin = clock();
#endif
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
#ifdef TESTTIME
		clockEnd = clock();   time=clockEnd - clockBegin;   printf("classify time: %dms\n", time);
#endif
	}
	deletewav(w);

	CalculateGoal();
	if(valide_model.size()!=0)
	{
	    GetClassName(classname);
		GetClassNum(classnum);
#ifdef TESTTIME
		_funcb(validelevel,classnum);
#endif
        return validelevel;  //返回1,认为至少找到了一个匹配的类
	}else { 
		classnum=0;
		classname.clear();
#ifdef SHOWTEST
		printf("no valide class\n");
#endif
#ifdef TESTTIME
		_funcb(validelevel,classnum);
#endif
		return validelevel;    //返回0，表示没有找到匹配类型
	}
}

int SoundClassify::ValideSoundClassify_SVM(const string soundpath,int soundtype)
{
	//1. 提取语音特征
	long int datasize;  int dim=MFCCCOEF*2;  string feature="mfcc_diff";
	string wav_path=soundpath;    vector<float> wav_data_store;
#ifdef TESTTIME
	clock_t clockBegin,clockEnd,time;
	clockBegin = clock();
#endif
	
	string full_wav_name=wav_path+".wav";   char wav[100];   strcpy(wav,full_wav_name.c_str());
	Wave *w = filter_bank(wav,soundtype);
	for (int i = MFCCCOEF; i < MFCCCOEF*(w->nRow-1); i++)   //输出了
	{
		if(w->valideframe[i/MFCCCOEF]==1)
		{
			if(i%(MFCCCOEF)==0)
			{
				for(int j=i;j<i+MFCCCOEF;j++)  wav_data_store.push_back(*(w->MFCCs + j));  
			}
			wav_data_store.push_back(*(w->DevMFCCs + i));
		}
	}
#ifdef TESTTIME
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("extraction time: %dms\n", time);
#endif
	
	deletewav(w);
	datasize=wav_data_store.size();
#ifdef TESTTIME
	clockBegin = clock();
#endif
	//2.将特定格式的语音特征存入文本中
	if(true)
	{
		ofstream out_to_svm(wav_path);
		assert(out_to_svm);
		for(int j=0;j<datasize;j=j+26)
		{
			out_to_svm<<"+1"<<" ";
			for(int i=0;i<26;i++)
			{
				out_to_svm<<i+1<<":"<<wav_data_store[i+j]<<" ";
			}
			out_to_svm<<endl;
		}
		out_to_svm.close();
	}
#ifdef TESTTIME
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("save feature file time: %dms\n", time);
#endif

#ifdef TESTTIME
	clockBegin = clock();
#endif
	//3.对于该文本的特征，利用liblinear进行分类输出  
    string full_data_output=wav_path+"output";
	int argc=4;
	char** argv=(char **)malloc(4*sizeof(char*));
	for(int i=0;i<4;i++) 
	{argv[i]=(char*)malloc(100*sizeof(char));}
	strcpy(argv[0],"predict");
	strcpy(argv[1],wav_path.c_str());
	strcpy(argv[2],"allsound_l2lr.model");
	strcpy(argv[3],full_data_output.c_str());
	predict_liblinear(argc,argv);
	for(int i=0;i<4;i++)
	{free((void*)argv[i]);}
	free((void*)argv);

	remove(wav_path.c_str());

	//4.分析输出的分类结果，返回分类标签
	ifstream in(full_data_output);
	vector<int> ana;
	int num; 
	int c1=0,c2=0,c3=0,c4=0,c5=0,c6=0,c7=0,c8=0;

	while(!in.eof()){
		in>>num;
		ana.push_back(num);
	}
	in.close();
    remove(full_data_output.c_str());

	std::vector<int>::iterator begin=ana.begin();
	for(;begin!=ana.end();begin++){
		if(*begin==1) c1++;
		else if(*begin==2) c2++;
		else if(*begin==3) c3++;
		else if(*begin==4) c4++;
		else if(*begin==5) c5++;
		else if(*begin==6) c6++;
		else if(*begin==7) c7++;
		else if(*begin==8) c8++;
	}
#ifdef TESTTIME
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("classify time: %dms\n", time);
#endif
	int max=c1,classnum=1;
	if(c2>max)max=c2,classnum=2; 
	else if(c3>max) max=c3,classnum=3; 
	else if(c4>max) max=c4,classnum=4;
	else if(c5>max) max=c5,classnum=5;
	else if(c6>max) max=c6,classnum=6;
	else if(c7>max) max=c7,classnum=7;
	else if(c8>max) max=c8,classnum=8;
#ifdef TESTTIME
	_funcb(1,classnum);
#endif
	return classnum;
}

int SoundClassify::ValideSoundClassify_GMM_SVM(const string soundpathsvm,vector<string> &classname,int soundtype)
{
	string wav_pathgmm=soundpathsvm+".wav";
	string wav_pathsvm=soundpathsvm;

	int gmmvalue,svmvalue;
	int classnumgmm,classnum;

	gmmvalue=ValideSoundClassify(wav_pathgmm,classname,classnumgmm);
    classnum=classnumgmm;
	if(gmmvalue==2)
	{
		printf("GMM: ");
		_funcb(gmmvalue,classnum);
	}
	else if(gmmvalue==1)
	{
		svmvalue=ValideSoundClassify_SVM(wav_pathsvm);
		if(svmvalue==classnum) printf("GMM_SVM: "),_funcb(svmvalue,classnum);
		else classnum=0,_funcb(0,0);
	}
	else
	{
		classnum=0;
		_funcb(0,0);
	}
	return classnum;
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