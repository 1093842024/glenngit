#ifndef __SOUNDCLASSIFY_H__
#define __SOUNDCLASSIFY_H__
#include "extractfeature.h"
#include "filterbank.h"
#include "KMeans.h"
#include "GMM.h"
#include "predict.h"
#include "train.h"
#include <map>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "SoundInterface.h"

#define MODEL1 0x01
#define MODEL2 0x02
#define MODEL3 0x04   
#define MODEL4 0x08
#define MODEL5 0x10
#define MODEL6 0x20   
#define MODEL7 0x40
#define MODEL8 0x80

//#define SHOWTEST     //非测试时，可以注释掉该行避免打印
//#define TESTTIME

#ifdef TESTTIME
#include <opencv2/opencv.hpp> 
#endif

struct GMM_Model{
	int    GMM_Num;            //模型代号
	string GMM_Name;           //模型代表的类型名
	string GMM_txt;            //模型所储存的txt文件名称（包含.txt的全称）
	string Feature_string;     //模型所对应的特征类型
	int    Dim;	               //模型的输入特征维数
	int    GMM_Mix_Num;        //模型的混合阶数
    double MaxProb;            //该模型的最大值阈值之一（较宽松） 识别率较高，但有一定误识别率
	double AverAllProb;        //该模型的累积归一化阈值之一（较宽松）
	double Maxprobstrict;      //该模型的最大值阈值之一（较严厉）  识别率相对低，但误识别率较低
	double AverAllProbstrict;  //该模型的累积归一化阈值之一（较严厉）
	double sampleMax;          //输入样本计算得到的最大值得分
	double sampleAverall;      //输入样本计算得到的累积归一化得分
};

class SoundClassify
{
public:
	SoundClassify(AnsCB funcb):Model_num(1){
		_funcb=funcb;
		InitGMM_Models(Model_num);}
	~SoundClassify(){
	    std::map<std::string,GMM*>::iterator removeitor=Gmm.begin();
		while(removeitor!=Gmm.end()){
			if(removeitor->second){
		     delete removeitor->second;
		     removeitor=Gmm.erase(removeitor);
			}
		}
	}
/**
* @brief ValideSoundClassify     【异常声音分类接口】   根据传入参数的不同分为 【服务器版本接口】 和 【个人电脑接口】 两个版本
*                                返回值为0，代表声音不属于任何库中的模型，返回值为1或2，代表找到匹配的声音类型，其中返回2代表识别的可信度高，返回1代表识别的可信度一般
*
* @param wavdata                 音频的采样值（无任何其他头信息）的指针wavdata，且每两个数据代表一个采样值，第一个数据是低8位，第二个数据是高8位
*
* @param len                     采样值的长度，len/2为采样的点数（每两个数据代表一个采样点）
*
* @param soundtype               输入的数据类型（不同接口，使用默认参数即可）                                
*	                             1代表输入的是单纯的音频采样数据（无任何头信息）和数据长度
**/
	/**服务器版本接口**/
	int ValideSoundClassify(const char *wavdata,int len, int soundtype=1);
	
	int AddNewGMMModel(GMM_Model model);            //增加新的模型，返回1表示添加成功，返回0表示已存在该模型代号，添加失败

	int ChangeGMMModel(GMM_Model model,int Gmmnum); //查找并修改已有模型，以模型代号来进行与已有模型的匹配。 返回1则查找并修改成功，返回0则未查找到模型

	
	/**个人电脑上接口**/

	int ValideSoundClassify(const string soundpath,vector<string> &classname,int &classnum,int soundtype=0);   //soundtype=0代表输入的是.wav格式音频的地址路径 比如 "data/audio.wav"
	//若返回值为1或2，则音频属于集内类型，通过读取classname可以找到对应的类型名称（可能对应多个类型名参数）；若返回值为0，则音频不属于集内类型，classname也为空   

	int ValideSoundClassify_SVM(const string soundpath,int soundtype=0);

	int ValideSoundClassify_GMM_SVM(const string soundpathsvm,vector<string> &classname,int soundtype=0);

private:
	void InitGMM_Models(int Model_num);
	void CalculateGoal();
    void GetClassName(vector<string> &classname);
	void GetClassNum(int& classnum);
	int Model_num;
	int validelevel;
	std::map<std::string,GMM*> Gmm;
    vector<GMM_Model> Gmm_model;
	vector<GMM_Model> valide_model;
	AnsCB _funcb;
};
#endif