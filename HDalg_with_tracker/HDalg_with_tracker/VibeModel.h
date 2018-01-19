#ifndef _VIBEMODEL_H_
#define _VIBEMODEL_H_

#include "HDcommon.h"
/************************************************************************/
/*                            BG method                                 */
/************************************************************************/
#define NOT_ENOUTH_POINTS 1
#define ENOUGH_POINTS 2
#define INIT_MODULE 3
#define IGNORED_LEN 8
#define FRAME_IGNORED 8
#define IMG_WIDTH 240
typedef struct VibeParam{
    uchar*  pModel;
    uchar*  pHistoryImage;             // 两点历史值
    uchar*  pFgImg;
    int            lastHistoryImageSwapped;    //用于交换模型点

    int            w, h;                     // 模型宽、高
    int            depth;                    // 模型大小 default: 16  每个像素点的背景模型的样本个数

    int            blockD;                   // Block直径 default: 9                         论文实验 8？8+1
    int            Radius;                   // 距离半径 default: 20，作为灵敏度调节参数之一 论文实验 20
    int            nTh;                      // 个数阈值 default: 2                          论文实验 2
    int            frameCnt;                 // 用于跳过初始若干帧

    int            roi_x;                    // 检测区域(x,y,w,h)
    int            roi_y;
    int            roi_w;
    int            roi_h;

    int            mindiff;                  //帧差所需参数
    uchar*  prevImg;
    uchar*  cntImg;
    int*           jump;                     // 用于模型更新时，跳过若干点。

    int            randorDep;                // 随机数，更新模型下标

    int            alarmCntTh;
    float          alarmPercentage;           // 前景点数达到这个比例会触发报警，灵敏度参数之一
    bool           justInitial;               // 上一帧是否用于初始化
    unsigned int    Sensitive;                // 灵敏度参数

    bool           last_is_fore;              // 快速模糊参数
    bool           last_is_fore_vec[IMG_WIDTH];
}VibeParam;

VibeParam* CreateBgParam();
/**
*  @brief UpdateBgModel		移动侦测处理模块
*
*  @param ptr_gray			灰度图像
*  @param width,height,step	灰度图的宽、高和step
*  @param sensitive			灵敏度参数，取值0、1、2、3，灵敏度依次降低
*  @param phdHandle			实例
*
*  @note  
*/
int UpdateBgModel( const uchar* ptr_gray, const int width, const int height, const int step, unsigned int sensitive, VibeParam* phdHandle);
bool ReleaseBgModel( VibeParam** phdHandle );

#endif	//#define _HDALG_H_
