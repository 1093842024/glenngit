#include "SoundInterface.h"
#include "SoundClassify.h"


HumanDetHandle CreateSoundClassify(AnsCB funcb)
{
	SoundClassify *sound= new SoundClassify(funcb);
	return (HumanDetHandle)sound;
}


void ValideSoundClassify(HumanDetHandle handle,const char *wavdata,int len,int valide)
{
	SoundClassify *sound=(SoundClassify*) handle;
	if(valide==0) sound->ValideSoundClassify(wavdata,len);
	else sound->ValideSoundClassify_Valide(wavdata,len);  //ʹ�ø÷�ʽ����Ҫ����޸���ȡ���������� ��ʱ���� �� ������ ����ֵ����
}

int ValideSoundDetect(unsigned char* wave,int len,float shortenergy,float zerorate)
{
	return AudioBeginDetect(wave,len,shortenergy,zerorate);
}

void ChangeGoal(HumanDetHandle handle,int maxgoal,int avergoal,int classnum)
{
	SoundClassify *sound=(SoundClassify*) handle;
	sound->ChangeGMMgoal(classnum,maxgoal,avergoal);
}

void ChangeGoalStrict(HumanDetHandle handle,int maxgoal,int avergoal,int classnum)
{
	SoundClassify *sound=(SoundClassify*) handle;
	sound->ChangeGMMgoalStrict(classnum,maxgoal,avergoal);
}

void ReleaseSoundClassify(HumanDetHandle handle)
{
	SoundClassify *sound=(SoundClassify*) handle;
	delete sound;
}