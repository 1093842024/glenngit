#ifndef __SOUNDINTERFACE_H__
#define __SOUNDINTERFACE_H__

/**
* @brief ValideSoundClassify     【有效声音检测接口】    PC实时处理速度为0.1ms内完成一次
*
* @param wave                 音频的采样值（无任何其他头信息，8KHZ）的指针，且每两个数据代表一个采样值，第一个数据是低8位，第二个数据是高8位
*
* @param len                     只支持传入数据长度为1024，即512个采样值
*
* @param shortenergy zerorate    用于修改有效声音检测的阈值
*
* @返回值                        0代表无效语音，1代表有效语音的开始端，则从该帧开始录制长度为5s的音频
**/
int ValideSoundDetect(unsigned char* wave,int len=1024,float shortenergy=2.0,float zerorate=3.5);

/**
* @brief ValideSoundClassify 【回调函数】
* @param valide               代表输入的声音是否有效，值为0，代表声音不属于任何库中的模型，值为1或2，代表找到匹配的声音类型，其中2代表识别的可信度高，1代表识别的可信度一般
* @param classnum             代表异常声音类型代号值，可能0个、1个或者多个代号值的复合值。0x00代表声音不属于任何库中的模型，其他值是匹配代号的或运算  
*                               模型编号形式如下：
*                               MODEL1 0x01 babycrying    MODEL2 0x02 boom     MODEL3 0x04 glass    MODEL4 0x08 gun   MODEL5 0x10 scream    MODEL6 0x20 blue  
*                              （比如返回0x03代表，MODEL1和MODEL2都有效匹配）
**/
typedef void (*AnsCB)(int valide,int classnum);   

typedef void* HumanDetHandle;
HumanDetHandle CreateSoundClassify(AnsCB funcb);

/**
* @brief ValideSoundClassify     【异常声音分类接口】   
*
* @param wavdata                 音频的采样值（无任何其他头信息，8KHZ）的指针wavdata，且每两个数据代表一个采样值，第一个数据是低8位，第二个数据是高8位
*
* @param len                     len=采样点数*2，因为每两个数据代表一个采样点
*
* @param valide                   选择不同的特征提取方式，默认使用0即可   
*
**/
void ValideSoundClassify(HumanDetHandle handle,const char *wavdata,int len,int valide=0);



/** 
*   @brief   ChangeGoal   ChangeGoalStrict 【修改评分阈值】  
*
**/

void ReleaseSoundClassify(HumanDetHandle handle);


void ChangeGoal(HumanDetHandle handle,int maxgoal,int avergoal,int classnum=1);
void ChangeGoalStrict(HumanDetHandle handle,int maxgoal,int avergoal,int classnum=1);
#endif