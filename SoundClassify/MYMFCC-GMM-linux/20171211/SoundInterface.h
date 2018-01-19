#ifndef __SOUNDINTERFACE_H__
#define __SOUNDINTERFACE_H__

/**
* @brief ValideSoundClassify     ����Ч�������ӿڡ�    PCʵʱ�����ٶ�Ϊ0.1ms�����һ��
*
* @param wave                 ��Ƶ�Ĳ���ֵ�����κ�����ͷ��Ϣ��8KHZ����ָ�룬��ÿ�������ݴ���һ������ֵ����һ�������ǵ�8λ���ڶ��������Ǹ�8λ
*
* @param len                     ֻ֧�ִ������ݳ���Ϊ1024����512������ֵ
*
* @����ֵ                        0������Ч������1������Ч�����Ŀ�ʼ�ˣ���Ӹ�֡��ʼ¼�Ƴ���Ϊ5s����Ƶ
**/
int ValideSoundDetect(unsigned char* wave,int len=1024);

/**
* @brief ValideSoundClassify ���ص�������
* @param valide               ��������������Ƿ���Ч��ֵΪ0�����������������κο��е�ģ�ͣ�ֵΪ1��2�������ҵ�ƥ����������ͣ�����2����ʶ��Ŀ��Ŷȸߣ�1����ʶ��Ŀ��Ŷ�һ��
* @param classnum             �����쳣�������ʹ���ֵ������0����1�����߶������ֵ�ĸ���ֵ��0x00���������������κο��е�ģ�ͣ�����ֵ��ƥ����ŵĻ�����  
*                               ģ�ͱ����ʽ���£�
*                               MODEL1 0x01 babycrying    MODEL2 0x02 boom     MODEL3 0x04 glass    MODEL4 0x08 gun   MODEL5 0x10 scream    MODEL6 0x20 blue  
*                              �����緵��0x03����MODEL1��MODEL2����Чƥ�䣩
**/
typedef void (*AnsCB)(int valide,int classnum);   

typedef void* HumanDetHandle;
HumanDetHandle CreateSoundClassify(AnsCB funcb);

/**
* @brief ValideSoundClassify     ���쳣��������ӿڡ�   
*
* @param wavdata                 ��Ƶ�Ĳ���ֵ�����κ�����ͷ��Ϣ��8KHZ����ָ��wavdata����ÿ�������ݴ���һ������ֵ����һ�������ǵ�8λ���ڶ��������Ǹ�8λ
*
* @param len                     len=��������*2����Ϊÿ�������ݴ���һ��������
**/
void ValideSoundClassify(HumanDetHandle handle,const char *wavdata,int len);

#endif