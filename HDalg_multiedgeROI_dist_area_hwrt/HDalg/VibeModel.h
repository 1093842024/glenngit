#ifndef _VIBEMODEL_H_
#define _VIBEMODEL_H_

#include "HDcommon.h"
/************************************************************************/
/*                            BG method                                 */
/************************************************************************/
#define NOT_ENOUTH_POINTS 1
#define ENOUGH_POINTS 2
#define INIT_MODULE 3
#define IGNORED_LEN 10
#define FRAME_IGNORED 4
#define IMG_WIDTH 240
typedef struct VibeParam{
    uchar*  pModel;
    uchar*  pHistoryImage;             // 两点历史值
    uchar*  pFgImg;
    int            lastHistoryImageSwapped;    //用于交换模型点

    int            w, h;          // 模型宽、高
    int            depth;         // 模型大小 default: 16  每个像素点所具有的背景模型样本点个数

    int            blockD;        // Block直径 default: 9  初始化时随机选择邻域中的信息构建该点的背景模型
    int            Radius;        // 距离半径 default: 20，作为灵敏度调节参数之一 计算每个点是否属于背景点，即计算待分类像素与背景模型中N个样本的欧式距离是否小于R
    int            nTh;           // 个数阈值 default: 2  ，  如果待分类像素与背景模型中N个样本的狗屎距离小于R的个数大于该阈值，则认为是与背景相似的点
    int            frameCnt;      // 用于跳过初始若干帧

    int            roi_x;         // 检测区域(x,y,w,h)  
    int            roi_y;
    int            roi_w;
    int            roi_h;
	
	float          AeraR;        //检测区域包围的矩形 /整个图像矩形的比例， 侦测区域为整个图像的时候该值为1

	float          AeraK;          //当开启多边形侦测区域时，该参数为 多边形面积/最小包围矩形 的大小 侦测区域为矩形时该值为1
	int            *vertx;         //多边形顶点坐标
	int            *verty;
	int            nvert;
	bool           multiedge;



    int            mindiff;       //帧差所需参数
    uchar*  prevImg;
    uchar*  cntImg;
    int*           jump;          // 用于模型更新时，跳过若干点。

    int            randorDep;     // 随机数，更新模型下标

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
