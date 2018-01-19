#include "train.h"
#include "predict.h"
#include "linear.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include<assert.h>

using namespace std;

//#define TESTINPUT
#define TESTOUTPUT
//#define SINGLETESTOUTPUT
//#define ANALYSE


int main(int argc, char **argv)  //argc为传入main函数的参数个数 argv代表传入的参数指针
{                                //argv[0]一定是程序的完整路径名称，argv[0][0]代表完整路径的第一个字符
	                             //没有任何参数传入时，argc为1，即只有argv[0]存在

	if(argc==1) printf("no param from other user\n");

#ifdef TESTINPUT 
    int i;
	argc=6;
	argv=(char **)malloc(6*sizeof(char*));
	for(i=0;i<6;i++) 
	{argv[i]=(char*)malloc(100*sizeof(char));}

	/*argv[0][0]='t';argv[0][1]='\0';
	argv[1][0]='-';argv[1][1]='s';argv[1][2]='\0';
	argv[2][0]='0';argv[2][1]='\0';
	argv[3][0]='a';argv[3][1]='1';argv[3][2]='a';argv[3][3]='\0';*/
	strcpy(argv[0],"t");
	strcpy(argv[1],"-s");
	strcpy(argv[2],"1");
	strcpy(argv[3],"-c");
	strcpy(argv[4],"1");
	strcpy(argv[5],"allsound.scale");

	train_liblinear(argc,argv);

	for(i=0;i<6;i++)
	{free((void*)argv[i]);}
	free((void*)argv);
#endif

#ifdef TESTOUTPUT
	int i;
	argc=4;
	argv=(char **)malloc(4*sizeof(char*));
	for(i=0;i<4;i++) 
	{argv[i]=(char*)malloc(100*sizeof(char));}

	strcpy(argv[0],"predict");
	strcpy(argv[1],"baby_audio_pcm");
	strcpy(argv[2],"allsound.model");
	strcpy(argv[3],"baby_audio_pcmoutput");

	predict_liblinear(argc,argv);

	for(i=0;i<4;i++)
	{free((void*)argv[i]);}
	free((void*)argv);
#endif

#ifdef SINGLETESTOUTPUT
	int i;
	argc=4;
	argv=(char **)malloc(4*sizeof(char*));
	for(i=0;i<4;i++) 
	{argv[i]=(char*)malloc(100*sizeof(char));}

	string data_path; int data_num;
	//data_path="sample/baby"; data_num=68;      //>78%  95%
	//data_path="sample/boom"; data_num=67;        //>40%  50% 
	//data_path="sample/glass"; data_num=36;
	//data_path="sample/gun"; data_num=128;
	//data_path="sample/scream"; data_num=49;
    //data_path="sample/red"; data_num=14;
    //data_path="sample/blue"; data_num=28;
	//data_path="sample/yellow"; data_num=13;
	//data_path="sample/test"; data_num=62;
	data_path="sample/alarm"; data_num=23;
	char data_name[100];
	int data_idx=1;

	for(;data_idx!=data_num+1;data_idx++)
	{
		sprintf_s(data_name, "%03d", data_idx);   string full_data_name=data_path+data_name; string full_data_output=full_data_name+"output";
		strcpy(argv[0],"predict");
		strcpy(argv[1],full_data_name.c_str());
		strcpy(argv[2],"allsound_l2l2svcdual.model");
		strcpy(argv[3],full_data_output.c_str());

		predict_liblinear(argc,argv);
	}

	for(i=0;i<4;i++)
	{free((void*)argv[i]);}
	free((void*)argv);
#endif

#ifdef ANALYSE
	string data_path; char data_name[100]; int data_num;
	int data_idx=1; 
	//data_path="sample/baby"; data_num=68;      
	//data_path="sample/boom"; data_num=67;       
	//data_path="sample/glass"; data_num=36;
	//data_path="sample/gun"; data_num=128;
	//data_path="sample/scream"; data_num=49;
	//data_path="sample/red"; data_num=14;
	//data_path="sample/blue"; data_num=28;
	//data_path="sample/yellow"; data_num=13;
	//data_path="sample/test"; data_num=62;
	data_path="sample/alarm";data_num=23;
	int count=0;

	for(;data_idx!=data_num+1;data_idx++)
	{
		sprintf_s(data_name, "%03d", data_idx);   string full_data_name=data_path+data_name; string full_data_output=full_data_name+"output";
		ifstream in(full_data_output);
		vector<int> ana;
		int num; 
		int c1=0,c2=0,c3=0,c4=0,c5=0,c6=0,c7=0,c8=0;

		while(!in.eof())
		{
		    in>>num;
			ana.push_back(num);
		}
		std::vector<int>::iterator begin=ana.begin();
		for(;begin!=ana.end();begin++)
		{
		    if(*begin==1) c1++;
			else if(*begin==2) c2++;
			else if(*begin==3) c3++;
			else if(*begin==4) c4++;
			else if(*begin==5) c5++;
			else if(*begin==6) c6++;
            else if(*begin==7) c7++;
			else if(*begin==8) c8++;
		}
		printf("%s:baby:%.1f,boom:%.1f,glass:%.1f,gun:%.1f,scream:%.1f,alarm:%.1f,color:%.1f,test:%.1f\n",
			full_data_name.c_str(),c1*100.0/ana.size(),c2*100.0/ana.size(),c3*100.0/ana.size(),c4*100.0/ana.size(),
			c5*100.0/ana.size(),c6*100.0/ana.size(),c7*100.0/ana.size(),c8*100.0/ana.size());

		if(c1>c2&&c1>c3&&c1>c4&&c1>c5&&c1>c6&&c1>c7&&c1>c8) count++;

		in.close();
	}

	printf("count:%d",count);

#endif


	system("pause");
	return 0;
}