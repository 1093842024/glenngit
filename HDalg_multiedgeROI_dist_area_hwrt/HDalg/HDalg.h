#ifndef _HDALG_H_
#define _HDALG_H_

#define MAX_DETECT_NUM 10
#define MAX_PT         6
#define MOVETIME 2          //计算距离的物体移动的秒数

#define RECTCHANGE 65

#define MOVEDISTTIMES 1    //该参数代表连续帧数下，移动距离值需要大于阈值的次数。如果该参数为1，则还是利用单帧的信息来侦测。
                           //该值越大，误报越少，但也会增加漏报率
                           //对于复杂情况的室外机，若要减少误报，建议值取在2~5之间。对于非复杂场景的情况，建议采用默认值1

/****
*         算法内部使用的过滤条件包括 高宽比 、 面积比例、 矢量移动距离、 标量移动距离。
*         其中算法默认的阈值为 高宽比0.3、 面积比例0.4%、 move_dist -1、 move_dist_mod -1
*         四个条件同时满足则认为是有效目标，其中设为-1表示该条件暂时作为过滤条件不起任何作用
****/

typedef struct ObjRect
{
	int x;
	int y;
	int w;
	int h;
}ObjRect;


typedef struct HdObj
{
    int   bflag;   // 是否为有效报警目标,有效目标下读取后续数值才有意义
    float cx;      // (cx,cy)目标中心点坐标
    float cy;
    float area;    // 目标面积/最小检测矩形框面积比例  或者  目标面积/整个图像面积比例                              默认为第一种
	float AreaK;   // 多边形区域/最小检测矩形框面积比例 或者 整个图像/最小检测矩形的面积比例   该参数用于画图处理， 默认为第一种
    float hvsw;    // 高度除以宽度
    float xspeed;   // x轴方向的移动速度,>0向右移动,<0向左移动,数值物理意义：每秒x方向运动值/检测区域宽度值
    int   nof;     // 靠近还是远离 -1 靠近,1远离,0无法确定,只适合在俯视角度下work

	float move_dist;                               // 目标在最近2s（或60帧里）的矢量移动距离，以像素位单位  added by gelin  2017-10-11
	float move_dist_mod;                           // 目标在最近2s（或60帧里）的标量移动距离                added by gelin 2017-10-11
	ObjRect pts[MAX_PT];                           // 目标最近5次出现的矩形框位置  added by gelin  2017-08-23
	                                     //建议的使用策略：若move_dist大于阈值，则认为是有效的移动；  暂不建议使用move_dist_mod参数 
}HdObj;                                 

typedef struct ReturnObjs{
	HdObj obj[MAX_DETECT_NUM];
	int num;
}ReturnObjs;


typedef void* HumanDetHandle;
HumanDetHandle CreateBackModel();

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
*  @brief SetFilterParam    设置报警过滤参数阈值
*
*  @param phdHandle         实例
*  @param th_dist           default：0
*  @param th_distmod        default：0
*  @return                  true  - 设置成功
                            false - 设置失败
*  @note  
*/
bool SetFilterParamDist(HumanDetHandle phdh, float th_dist, float th_distmod=-1);
/**
*  @brief SetROI            设置侦测区域模块
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
*  @brief SetROI_MultiEdge           设置侦测区域模块(多边形区域)
*
*  @param nvert            多边形的顶点数，点数要大于3
                            
*  @param vertxx vertyy    多边形的顶点的x，y坐标的存储指针，有几个顶点，对应就有几个连续指针有坐标值
                           
						   【注意】：坐标的赋值要按照多边形的边的连接顺序来存储，最后一个点和最开始的点相连

*  @param type              0- 删除之前的ROI，添加新的ROI
                            2- 删除所有的ROI，侦测全图
*  @param phdHandle         实例
*  @return                  true  - 设置成功
                            false - 设置失败
*  @note  
*/
bool SetROI_MultiEdge(HumanDetHandle phdh, int* vertxx, int* vertyy,const int nvert,const int type);

/**
*  @brief UpdateAndHD       移动侦测处理模块
*
*  @param ptr_gray          灰度图像
*  @param width,height,step 灰度图的宽、高和step
*  @param sensitive         灵敏度参数，取值0、1、2、3、4、5、6，灵敏度依次降低
*  @param phdHandle         实例
*
*  @note  
*/
int UpdateAndHD(HumanDetHandle phdHandle, unsigned char* gray, const int width, const int height, const int step, unsigned int sensitive/*, ReturnObjs *out_objs*/);

/**
*  @brief getObjs           获取运动目标位置和大小
*
*  @param phdHandle         实例
*  @note                    如果设置了移动侦测区域，目标位置和大小指的是相对于移动侦测区域而非全图
                            即设目标真实坐标绝对大小为obj.x,obj.y,obj.area
                            cx = (obj.x - roi.x)/(roi.w*1.0)
                            cy = (obj.y - roi.y)/(roi.h*1.0)
                            area = obj.area/(roi.w*roi.h*1.0)
*  @example:

int ret = UpdateAndHD(phdHandle, imageDataGray, width, height, step, 0);
if (3 == ret)
{
    ReturnObjs * out_objs = getObjs(phdHandle);
    for (int idx=0; idx<out_objs->num; idx++)
    {
        if(out_objs->obj[idx].bflag > 0)
        {
            printf("Target %d: position (%f,%f), area %f\n",idx,out_objs->obj[idx].cx,out_objs->obj[idx].cy,out_objs->obj[idx].area);
            printf("bject move distance vec in latest 2s / 60 frame is %f\n",out_objs->obj[kk].move_dist);
		}
    }
}
*  @note  
*/
ReturnObjs* getObjs(HumanDetHandle phdHandle);
bool ReleaseBackModel(HumanDetHandle* phdHandle);
#endif
