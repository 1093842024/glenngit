#ifndef _HDALG_H_
#define _HDALG_H_

#define MAX_DETECT_NUM 10

#define TRACKING_MODE 0x04
#define TURN_LEFT 0x10
#define TURN_RIGHT 0x08

#define RESET_MODE 0x0
#define TURN_STOP 0x71

#define TURN_UP 0x40
#define TURN_DOWN 0x20

#define FREEZECNT 15  //静止跟丢帧数
#define FREEZERATE 0.85   //静止跟丢的速度，值越小越难静止跟丢，值越大越容易静止跟丢

#define MINTRACKSIZE 15 //最小跟丢尺寸    改尺寸可以改为 15
#define MAXTRACKTIME 300 //最大跟踪帧数
#define MIDWIDTH 0.05    //中心宽度的一半
#define MINIWH 0.03     //探测到的目标的最小 宽度 或者 高度 占 图像宽度或高度 的比例   added at 20171108
#define DETECTWITH 0.03  //初始探测忽略的边缘宽度比例
#define FREEZETIME  6     //电机停转判断为跟丢的帧数

#define SHIFTLOSTAERA 0.008   //漂移过程中的最小面积比例
#define SHIFTLOSTGOAL 0.22    //漂移过程中的最低分数

#define MINHWRT    0.7          //最小宽高比例
#define MAXHWRT    3.8           //最大宽高比例
#define MINAERA    0.01         //最小面积比例

#define SENSITIVE 2          //灵敏度 0 1 2 3

#define SIDEFIX  0.5        //对两侧目标修正 初始位置 修正量为目标宽度的比例

#define SIDEWITH  0.2       //两侧进入特殊处理
#define SIDECNT   25        //两侧进入特殊处理帧数

//#define SHOWPRINT

typedef struct HdObj
{
    int   bflag;   // 是否为有效报警目标,有效目标下读取后续数值才有意义
    float cx;      // (cx,cy)目标中心点坐标在整个图像中的归一化位置
    float cy;
    float area;    // 目标面积/检测框面积
    float hvsw;    // 高度除以宽度
    float xspeed;   // x轴方向的移动速度,>0向右移动,<0向左移动,数值物理意义：每秒x方向运动值/检测区域宽度值
	float yspeed;
    int   nof;     // 靠近还是远离 -1 靠近,1远离,0无法确定,只适合在俯视角度下work


	int x,y,w,h;       //移动跟踪初始框的位置     这两组框的位置存在ReturnObjs->obj[0]中
	int xn,yn,wn,hn;   //移动跟踪目标框的实时位置
	bool show;

}HdObj;

typedef struct ReturnObjs{
	HdObj obj[MAX_DETECT_NUM];
	int num;
}ReturnObjs;


typedef void* HumanDetHandle;
/************************************************************************/
/** 

*  @brief CreateBackModel        初始化建立背景模型和移动跟踪器的实例

*  @param use_tracking           是否开启跟踪功能
*  @param tmpl_size              移动跟踪模板尺寸
*  @param padding_size           移动跟踪区域扩展倍率
*  @param scale_size             移动跟踪尺度缩放系数

*  @return 指向图像背景和移动跟踪器的对象的实例

*  @example:

    HumanDetHandle phdHandle=CreateBackModel();

*  @note
**/
HumanDetHandle CreateBackModel(bool use_tracking = true,int tmpl_size = 64,float padding_size = 2.5,float scale_size=1.05);



/**
*  @brief UpdateAndHD       移动侦测与跟踪处理模块
*
*  @param ptr_gray          灰度图像
*  @param width,height,step 灰度图的宽、高和step
*  @param sensitive         灵敏度参数，取值0、1、2、3，灵敏度依次降低
*  @param phdHandle         实例
*  @return bit 6543210 分别为上、下、左、右、跟踪、移动侦测、画面变化；（目前上、下功能不支持）

*  @example:

    int ret = UpdateAndHD(phdHandle, imageDataGray, width, height, step, 0);

    ReturnObjs * out_objs = getObjs(phdHandle);
    for (int idx=0; idx<out_objs->num; idx++)
    {
        if(out_objs->obj[idx].bflag > 0)
        {
            printf("Target %d: position (%f,%f), area %f\n",idx,out_objs->obj[idx].cx,out_objs->obj[idx].cy,out_objs->obj[idx].area);
        }
    }

*  @ ret 的返回值有以下几种情况：
     1（bit 0）        画面变化
     3（bit 1）        找到有效移动侦测区域
     0x04（bit 2）     TRACKING_MODE 进入移动跟踪模式 或者 保持移动跟踪模式 
	 0x10（bit 4）     TURN_LEFT     移动跟踪模式 向左转动
	 0x08（bit 3）     TURN_RIGHT    移动跟踪模式 向右转动

*  @note  
*/
int UpdateAndHD(HumanDetHandle phdHandle,const unsigned char* gray, const int width, const int height, const int step, unsigned int sensitive/*, ReturnObjs *out_objs*/);



/**
*  @brief SetFilterParam    设置报警过滤参数阈值
*
*  @param phdHandle         实例
*  @param th_area           面积阈值，大于比例的目标才有效，default：0.003
*  @param th_hvsw           高度比宽度，大于阈值的目标才有效，default：1.0
*  @return                  true  - 设置成功
                            false - 设置失败
*  @note  
*/
bool SetFilterParam(HumanDetHandle hdHandle, float th_area, float th_hvsw);



/**
*  @brief SetROI            设置侦测区域
*
*  @param x,y,w,h           区域的左上坐标和宽高
                            设备收到的区域为float类型，转化方法(设视频图像宽高为width,height)：
                            x = x_float * width;
                            y = x_float * height;
                            w = w_float * width;
                            h = h_float * height;
*  @param type              0- 删除之前的ROI，添加新的ROI
                            1- 保留之前的ROI，添加新的ROI（已废弃，不要用）
                            2- 删除所有的ROI，侦测全图
*  @param phdHandle         实例
*  @return                  true  - 设置成功
                            false - 设置失败
*  @note  
*/
bool SetROI(HumanDetHandle hdHandle, const int x, const int y, const int w, const int h, const int type);



/**
	*  @brief getObjs           获取运动目标位置和大小
	*
	*  @param phdHandle         实例
	*  @note                    如果设置了移动侦测区域，目标位置和大小指的是相对于移动侦测区域而非全图
	即设目标真实坐标绝对大小为obj.x,obj.y,obj.area
	cx = (obj.x - roi.x)/(roi.w*1.0)
	cy = (obj.y - roi.y)/(roi.h*1.0)
	area = obj.area/(roi.w*roi.h*1.0)**/
ReturnObjs* getObjs(HumanDetHandle phdHandle);

bool ReleaseBackModel(HumanDetHandle* phdHandle);
#endif

