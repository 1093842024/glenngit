#include "SoundInterface.h"
#include "SoundClassify.h"


HumanDetHandle CreateSoundClassify(AnsCB funcb)
{
	SoundClassify *sound= new SoundClassify(funcb);
	return (HumanDetHandle)sound;
}


void ValideSoundClassify(HumanDetHandle handle,const char *wavdata,int len)
{
	SoundClassify *sound=(SoundClassify*) handle;
	sound->ValideSoundClassify(wavdata,len);
}

int ValideSoundDetect(unsigned char* wave,int len)
{
	return AudioBeginDetect(wave,len);
}