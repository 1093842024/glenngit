#include "SoundClassify.h"
#include "SoundInterface.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include<assert.h>
#include <opencv2/opencv.hpp> 
using namespace std;

#define GMM_MIX_NUM 32    //高斯混合模型阶数   GMM的阶数作为其中kmeans聚类的数量，将聚类中心的均值和方差作为每个高斯混合模型的初始均值和方差


void getans(int val,int num)
{
	printf("val:%d,num:%d\n",val,num);
}

int main(int argc, char **argv)
{

	SoundClassify sound(getans);
	vector<string> soundclass;	double time_profile_counter;  string wav_path; int wavnum;
#ifdef TESTTIME
    wav_path="data/baby013.wav"; int num;
	clock_t clockBegin,clockEnd,time;
	clockBegin = clock();
	sound.ValideSoundClassify(wav_path,soundclass,num);
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("GMM time: %dms\n\n", time);
	wav_path="data/baby013";
	clockBegin = clock();
	sound.ValideSoundClassify_SVM(wav_path);
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("SVM time: %dms\n\n", time);
	clockBegin = clock();
	sound.ValideSoundClassify_GMM_SVM(wav_path,soundclass);
	clockEnd = clock();   time=clockEnd - clockBegin;   printf("GMM_SVM time: %dms\n", time);
	system("pause");
#endif
	

	wav_path="data/baby"; wavnum=68;
	char wav_name[100];
	int wav_idex=1; int val=0; int correct=0,error=0;

	for(;wav_idex!=wavnum+1;wav_idex++)
	{
        sprintf_s(wav_name, "%03d", wav_idex);   string wav_path_test=wav_path+wav_name;
        val=sound.ValideSoundClassify_GMM_SVM(wav_path_test,soundclass);
	    if(val==1) correct++;
		else error++;
		soundclass.clear();
	}
	printf("**********************baby class correct rate:%.1f,error rate:%.1f\n",correct*100.0/wavnum,error*100.0/wavnum);
	system("pause");

	vector<string> other_path;vector<int> othernum;
	other_path.push_back("data/boom");othernum.push_back(67);
	other_path.push_back("data/glass");othernum.push_back(36);
	other_path.push_back("data/gun");othernum.push_back(128);
	other_path.push_back("data/scream");othernum.push_back(49);
	other_path.push_back("data/red");othernum.push_back(14);
	other_path.push_back("data/blue");othernum.push_back(28);
	other_path.push_back("data/yellow");othernum.push_back(13);
	other_path.push_back("data/test");othernum.push_back(62);
	other_path.push_back("data/alarm");othernum.push_back(23);

	vector<string>::iterator itor=other_path.begin(); vector<int>::iterator itor2=othernum.begin();
	int allnum=0,allerror=0;
	for(;itor!=other_path.end();itor++,itor2++)
	{
		wav_path=*itor; wavnum=*itor2;
		int wav_idex=1; int val=0; int correct=0,error=0;
		for(;wav_idex!=wavnum+1;wav_idex++)
		{
			sprintf_s(wav_name, "%03d", wav_idex);   string wav_path_test=wav_path+wav_name;
			val=sound.ValideSoundClassify_GMM_SVM(wav_path_test,soundclass);
			if(val==1) error++;
			else correct++;
			soundclass.clear();
		}
		printf("other correct rate:%.1f,error rate:%.1f\n",correct*100.0/wavnum,error*100.0/wavnum);
		allnum+=wavnum;allerror+=error;
	}
	printf("**********************all class errror rate:%.1f\n",allerror*100.0/allnum);

	//time_profile_counter = (double)cvGetTickCount();
	//std::cout << "time used " <<( (double)cvGetTickCount()-time_profile_counter )/ ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;


	system("pause");
	return 0;
}