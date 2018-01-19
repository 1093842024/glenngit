#ifndef __SOUNDCLASSIFY_H__
#define __SOUNDCLASSIFY_H__
#include "extractfeature.h"
#include "filterbank.h"
#include "KMeans.h"
#include "GMM.h"
#include <map>
#include "SoundInterface.h"

#define MODEL1 0x01
#define MODEL2 0x02
#define MODEL3 0x04   
#define MODEL4 0x08
#define MODEL5 0x10
#define MODEL6 0x20   
#define MODEL7 0x40
#define MODEL8 0x80

//#define SHOWTEST     //�ǲ���ʱ������ע�͵����б����ӡ
//#define TESTTIME

#ifdef TESTTIME
#include <opencv2/opencv.hpp> 
#endif

struct GMM_Model{
	int    GMM_Num;            //ģ�ʹ���
	string GMM_Name;           //ģ�ʹ����������
	string GMM_txt;            //ģ���������txt�ļ����ƣ�����.txt��ȫ�ƣ�
	string Feature_string;     //ģ������Ӧ����������
	int    Dim;	               //ģ�͵���������ά��
	int    GMM_Mix_Num;        //ģ�͵Ļ�Ͻ���
    double MaxProb;            //��ģ�͵����ֵ��ֵ֮һ���Ͽ��ɣ� ʶ���ʽϸߣ�����һ����ʶ����
	double AverAllProb;        //��ģ�͵��ۻ���һ����ֵ֮һ���Ͽ��ɣ�
	double Maxprobstrict;      //��ģ�͵����ֵ��ֵ֮һ����������  ʶ������Եͣ�����ʶ���ʽϵ�
	double AverAllProbstrict;  //��ģ�͵��ۻ���һ����ֵ֮һ����������
	double sampleMax;          //������������õ������ֵ�÷�
	double sampleAverall;      //������������õ����ۻ���һ���÷�
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
* @brief ValideSoundClassify     ���쳣��������ӿڡ�   ���ݴ�������Ĳ�ͬ��Ϊ ���������汾�ӿڡ� �� �����˵��Խӿڡ� �����汾
*                                ����ֵΪ0�����������������κο��е�ģ�ͣ�����ֵΪ1��2�������ҵ�ƥ����������ͣ����з���2����ʶ��Ŀ��Ŷȸߣ�����1����ʶ��Ŀ��Ŷ�һ��
*
* @param wavdata                 ��Ƶ�Ĳ���ֵ�����κ�����ͷ��Ϣ����ָ��wavdata����ÿ�������ݴ���һ������ֵ����һ�������ǵ�8λ���ڶ��������Ǹ�8λ
*
* @param len                     ����ֵ�ĳ��ȣ�len/2Ϊ�����ĵ�����ÿ�������ݴ���һ�������㣩
*
* @param soundtype               ������������ͣ���ͬ�ӿڣ�ʹ��Ĭ�ϲ������ɣ�                                
*	                             1����������ǵ�������Ƶ�������ݣ����κ�ͷ��Ϣ�������ݳ���
**/
	/**�������汾�ӿ�**/
	int ValideSoundClassify(const char *wavdata,int len, int soundtype=1);

	int ValideSoundClassify_Valide(const char *wavdata,int len, int soundtype=1);
	
	int AddNewGMMModel(GMM_Model model);            //�����µ�ģ�ͣ�����1��ʾ��ӳɹ�������0��ʾ�Ѵ��ڸ�ģ�ʹ��ţ����ʧ��

	int ChangeGMMModel(GMM_Model model,int Gmmnum); //���Ҳ��޸�����ģ�ͣ���ģ�ʹ���������������ģ�͵�ƥ�䡣 ����1����Ҳ��޸ĳɹ�������0��δ���ҵ�ģ��
	
	void ChangeGMMgoal(int Gmmnum,int maxgoal,int avergoal);

	void ChangeGMMgoalStrict(int Gmmnum,int maxgoal,int avergoal);
	
	/**���˵����Ͻӿ�**/
	int ValideSoundClassify(const string soundpath,vector<string> &classname,int soundtype=0);   //soundtype=0�����������.wav��ʽ��Ƶ�ĵ�ַ·�� ���� "data/audio.wav"
	//������ֵΪ1������Ƶ���ڼ������ͣ�ͨ����ȡclassname�����ҵ���Ӧ���������ƣ����ܶ�Ӧ�����������������������ֵΪ0������Ƶ�����ڼ������ͣ�classnameҲΪ��   

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
	AnsCB _funcb;       //����һ������ָ�룬���ڻص��������ú���ָ�����ʽ�������ͷ�ļ��ж���
};
#endif