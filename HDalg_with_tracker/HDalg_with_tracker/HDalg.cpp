﻿#include "HDcommon.h"
#include "VibeModel.h"
#include "Morph.h"
#include "MultiObjTrack.h"
#include "HDalg.h"
#include "trackerinterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) > (b) ? (b) : (a))

typedef struct HumanDet{
	VibeParam *pVP;                         //vibe背景检测算法参数
	vector<HdRect> blobList;
	HdSize operateSZ;
	MatchParam* pMP;                       //匹配参数
	ObjList *objlist;
	bool needInit;
	ReturnObjs *out_objs;

    float th_area;  //-! mark added
    float th_hvsw;
	float th_areamax;                            //addby glenn

    TrackHandle tracker;
    int md_track_flag;
    int track_freeze_cnt;                  //跟踪位置框基本静止的帧数 用户判断目标丢失
    int track_cx,track_cy,track_width,track_heigth;
    double prev_dcx;                        //之前的跟踪框的x方向位置 
	double prev_dw;                         //判断框是否大小保持不变
	vector<float> prev_allcx;
	int alltracktimes;
	int freezetime;
    bool use_tracker;                      //是否使用跟踪器的标志

	bool sidetrack;                      //目标从两侧进入标志位
	int sidetrackcnt;                    //目标从两侧进入固定跟踪帧数

}HumanDet;

//初始化，包括各个模块的初始化
HumanDetHandle CreateBackModel(bool use_tracking,int tmpl_size,float padding_size,float scale_size)
{
	HumanDet* phd = new HumanDet;
	phd->pVP = CreateBgParam();
	phd->blobList = vector<HdRect>();
	phd->operateSZ = hdSize(2,3);
	phd->pMP = CreateTrackParam(10,200);
	phd->objlist = new ObjList;
	phd->objlist->pObjHead = new ObjNode;
	phd->objlist->pObjHead->next = NULL;
	phd->objlist->len = 0;
	phd->objlist->interestLen = 0;
	phd->needInit = true;
	phd->out_objs = new ReturnObjs;
	phd->out_objs->num = 0;

    phd->th_area = MINAERA;//-! mark added
    phd->th_hvsw = MINHWRT;
	phd->th_areamax=0.2;

    if (use_tracking)
    {
        phd->md_track_flag = 0;
        phd->track_freeze_cnt = 0;
        phd->tracker = CreateTracker(tmpl_size,padding_size,scale_size);
        phd->use_tracker = true;
		phd->sidetrack=false;
		phd->sidetrackcnt=0;
    }else{
        phd->md_track_flag = 0;
        phd->use_tracker = false;
        phd->tracker = NULL;
    }
	return (HumanDetHandle)phd;
}

int ObjPostProcess(ObjList *objList, HumanDet* phd/*, ReturnObjs *out_objs*/);

//整体逻辑过程
int UpdateAndHD(HumanDetHandle phdHandle, const uchar* gray, const int width, const int height, const int step, unsigned int sensitive/*, ReturnObjs *out_objs*/)
{
	HumanDet* phd = (HumanDet*)phdHandle;
	int ret = 0;
	
    if (0 == phd->md_track_flag)                 //非移动跟踪模式，即移动侦测模式
    {
        phd->out_objs->num = 0;
        int flag = UpdateBgModel( gray, width, height, step, SENSITIVE, phd->pVP ) ;
		
        if (flag == INIT_MODULE)                 //如果返回的是需要初始化模式，则将needinit设为true，需要进行初始化
        {
            phd->needInit = true;
#ifdef SHOWPRINT
			printf("init obilist!\n");
#endif
			}
        else if (flag == ENOUGH_POINTS)
        {	
#ifdef SHOWPRINT
			printf("enough points!\n");
#endif
            ret = 1;
			if(fastFindContourRects(phd->pVP->pFgImg,phd->pVP->w,phd->pVP->h,phd->pVP->w,phd->blobList,phd->operateSZ,0.4) > 0)
            {
                if (phd->needInit)                       //如果不是初始化模式，但needinit为true 则初始化
                {
                    InitObjList(phd->pMP,phd->objlist,phd->blobList);
                    phd->needInit = false;
                }
                else
                {
                    MultiTraceProcess(phd->pMP,phd->objlist,phd->blobList);
                    ret = ObjPostProcess(phd->objlist, phd/*,out_objs*/);
                    if (ret == TRACKING_MODE)
                    {
                        BBox init_box;
						float factor=0.7;
                        float init_max_area = width*height*0.1;
						float init_area = phd->track_width*phd->track_heigth;
						if(init_area<init_max_area)
						{
							if (phd->track_width<0.3*phd->track_heigth)
							{
								init_box.w = 1.2*phd->track_width;
								init_box.h = init_box.w*1.0/factor;
								init_box.y = phd->track_cy - phd->track_heigth*0.45;
								init_box.x = phd->track_cx - phd->track_width*1.2/2;
							}
							else if(phd->track_width<0.7*phd->track_heigth)
							{
								init_box.w = phd->track_width;
								init_box.h = phd->track_heigth;
								init_box.y = phd->track_cy - phd->track_heigth*0.45;
								init_box.x = phd->track_cx - phd->track_width/2;
							}else{
								init_box.w = phd->track_width;
								if(factor*init_box.w<phd->track_heigth)
								{
								    init_box.h = factor*init_box.w;
									init_box.y = phd->track_cy -phd->track_heigth*0.45;
								}else{
                                    init_box.h = phd->track_heigth;
                                    init_box.y = phd->track_cy -init_box.h/2;
								}
								init_box.x = phd->track_cx - phd->track_width/2;
							}
						}else{
							float k=sqrt(init_max_area/init_area);
							phd->track_width=k*phd->track_width;
							phd->track_heigth=k*phd->track_heigth;

							if (phd->track_width<phd->track_heigth)
							{
								init_box.w = phd->track_width;
								init_box.h = factor*init_box.w;
								init_box.y = phd->track_cy - phd->track_heigth/k*0.45;
								init_box.x = phd->track_cx - phd->track_width/2;
							}else{
								init_box.w = phd->track_width;
								if(factor*init_box.w<phd->track_heigth)
								{
									init_box.h = factor*init_box.w;
									init_box.y = phd->track_cy -phd->track_heigth/k*0.45;
								}else{
									init_box.h = phd->track_heigth;
									init_box.y = phd->track_cy -init_box.h/2;
								}
								init_box.x = phd->track_cx - phd->track_width/2;
							}
						}

						phd->out_objs->obj[0].x=phd->track_cx - phd->track_width/2;
						phd->out_objs->obj[0].y=phd->track_cy - phd->track_heigth/2;
						phd->out_objs->obj[0].w=phd->track_width;
						phd->out_objs->obj[0].h=phd->track_heigth;
						phd->out_objs->obj[0].xn= init_box.x;
						phd->out_objs->obj[0].yn= init_box.y;
						phd->out_objs->obj[0].wn= init_box.w;
						phd->out_objs->obj[0].hn= init_box.h;
						phd->out_objs->obj[0].show=false;

						phd->prev_allcx.clear();
						phd->alltracktimes=0;
						phd->freezetime=0;

                        printf("init tracker ============================================> (%d,%d,%d,%d) of (%d,%d)\n",
                            int(init_box.x),int(init_box.y),int(init_box.w),int(init_box.h),
                            width,height) ;

                        phd->prev_dcx = phd->track_cx;
						phd->prev_dw  = init_box.w;
						phd->prev_allcx.push_back(phd->track_cx);

                        InitTracker(phd->tracker,init_box,(unsigned char*)gray,width,height,step);
						phd->track_freeze_cnt = 0;

						if (init_box.x+init_box.w/2 > (0.5+MIDWIDTH)*width)
						{
							ret = TURN_RIGHT;
						}
						else if(init_box.x+init_box.w/2 < (0.5-MIDWIDTH)*width)
						{
							ret = TURN_LEFT;
						}
						else{
							ret= TRACKING_MODE;
							phd->alltracktimes=1;
							phd->freezetime=1;
						}
                    }
                }
            }
            else
            {
                MultiTraceProcess(phd->pMP,phd->objlist,phd->blobList);
            }
        }
        else if(flag == NOT_ENOUTH_POINTS)	//设备上期初没有这个！！
        {
#ifdef SHOWPRINT
			printf("not enough points!\n");
#endif
            MultiTraceProcess(phd->pMP,phd->objlist,phd->blobList);
        }
		else if(flag == FRAME_IGNORED)
		{
#ifdef SHOWPRINT
            printf("frame ignored!\n");
#endif
		}
        phd->blobList.clear();
    }
    else if (1 == phd->md_track_flag && phd->use_tracker)
    {
		int times; float goal;
        BBox det_box = UpdateTracker(phd->tracker,(unsigned char*)gray,width,height,step,times,goal);


		phd->out_objs->obj[0].xn= det_box.x;
		phd->out_objs->obj[0].yn= det_box.y;
		phd->out_objs->obj[0].wn= det_box.w;
		phd->out_objs->obj[0].hn= det_box.h;
		phd->out_objs->obj[0].show=true;
		//phd->alltracktimes++;

        if (det_box.x+det_box.w>width || det_box.x<=0 ||det_box.y<=0||det_box.y+det_box.h>height || phd->track_freeze_cnt>=FREEZECNT 
			|| times>14 || det_box.w<MINTRACKSIZE||det_box.h<MINTRACKSIZE||phd->alltracktimes>MAXTRACKTIME
			||phd->freezetime>FREEZETIME||(det_box.w*det_box.h*1.0/(width*height)<SHIFTLOSTAERA && goal<SHIFTLOSTGOAL)||phd->sidetrackcnt>SIDECNT)    
		{
			if(phd->track_freeze_cnt>=FREEZECNT)printf("freeze time outrange======================================= stop tracking!\n");
		    else if(times>14)printf("missing target========================================= stop tracking!\n");
		    else if(det_box.x+det_box.w>width || det_box.x<=0||det_box.y<=0 ||det_box.y+det_box.h>height )printf("area outrange===================================== stop tracking!\n");
			else if(det_box.w<MINTRACKSIZE||det_box.h<MINTRACKSIZE) printf("tracking wide or height too small===================================== stop tracking!\n");
			else if(phd->alltracktimes>MAXTRACKTIME) printf("tracking time lasts too long===================================== stop tracking!\n");
			else if(phd->freezetime>FREEZETIME) printf("freezetime of motor exceed===============================stop tracking\n");
			else if(phd->sidetrackcnt>SIDECNT) printf("side object track cnt exceed===============================stop tracking\n");
			phd->md_track_flag = 0;
			phd->track_freeze_cnt = 0;

			phd->alltracktimes=0;
			phd->freezetime=0;

			 phd->sidetrackcnt=0;
			 phd->sidetrack=false;

			phd->pVP->frameCnt=IGNORED_LEN*0.8;              //丢失目标后进行背景建模更新
			phd->out_objs->obj[0].show=false;
            ret = 0;
        }else{
            ret = TRACKING_MODE;
            double dcx = det_box.x + det_box.w/2;
			double dcy = det_box.y + det_box.h / 2;   
			phd->out_objs->obj[0].cx = dcx / width;
			phd->out_objs->obj[0].cy = dcy / height;
			//printf("dist : %f\n",abs(dcx - phd->prev_allcx[0]));
			if (abs(dcx - phd->prev_dcx) < FREEZERATE*MAX(det_box.w/8,6) && abs(det_box.w - phd->prev_dw) < FREEZERATE*MAX(det_box.w/8,6)&& abs(dcx - phd->prev_allcx[0])<FREEZERATE*MIN(det_box.w/4,5))   //框基本位置和框的宽度大小基本不变
            {
				if(dcx>0.45*width&&dcx<0.55*width) phd->track_freeze_cnt+=2;
                   phd->track_freeze_cnt += 1;

            }else{
				//if(phd->track_freeze_cnt>10) phd->track_freeze_cnt-=10;
				//else if(phd->track_freeze_cnt>5) phd->track_freeze_cnt-=5;
                //else 
					phd->track_freeze_cnt = 0;
            }

            phd->prev_dcx = dcx;   //update prev location
			phd->prev_dw=det_box.w;
			if(phd->prev_allcx.size()<20)
			{
			    phd->prev_allcx.push_back(dcx);
			}else{
				phd->prev_allcx.erase(phd->prev_allcx.begin());
				phd->prev_allcx.push_back(dcx);
			}

            if (dcx > (0.5+MIDWIDTH)*width)
            {
#ifdef SHOWPRINT
                printf("turning right\n");
#endif

                ret = TURN_RIGHT;
				phd->freezetime=0;
            }
            else if(dcx < (0.5-MIDWIDTH)*width)
            {
#ifdef SHOWPRINT
                printf("turning left\n");
#endif
				
				ret = TURN_LEFT;
				phd->freezetime=0;
            }
            else
            {   
#ifdef SHOWPRINT
				printf("hold on\n"); 
#endif
				phd->freezetime++;
				if(goal<0.35)  phd->alltracktimes++;
			}

			if(phd->sidetrack==true) phd->sidetrackcnt++;


        }
    }
	
	return ret;
}

bool ReleaseBackModel(HumanDetHandle* phdHandle)
{
	HumanDet **phd = (HumanDet**)phdHandle;

	if (phd!=NULL && *phd!=NULL)
	{	
		ReleaseBgModel(&((*phd)->pVP));;
		(*phd)->blobList.clear();
		/*(*phd)->blobList=NULL;*/
		DeleteTrackParam(&((*phd)->pMP));
		DeleteObjList((*phd)->objlist);
        ReleaseTracker((*phd)->tracker);    //此函数进行了修改，传入的是指向对象tracker的指针
		delete (*phd)->out_objs;
		delete *phd;
		*phd = NULL;
	}
	return true;
}

bool SetROI(HumanDetHandle phdh, const int x, const int y, const int w, const int h, const int type)
{
    HumanDet* phd = (HumanDet*)phdh;
    
    if (0 == type)
    {
        phd->pVP->roi_x = x;
        phd->pVP->roi_y = y;
        phd->pVP->roi_w = w;
        phd->pVP->roi_h = h;
        phd->pVP->frameCnt = IGNORED_LEN - 1;
        return true;
    }
    else if (2 == type)
    {
        phd->pVP->roi_x = -1;
        phd->pVP->roi_y = -1;
        phd->pVP->roi_w = -1;
        phd->pVP->roi_h = -1;
        phd->pVP->frameCnt = IGNORED_LEN - 1;
        return true;
    }else{
        return false;
    }
        
}

bool SetFilterParam(HumanDetHandle phdh, float th_area, float th_hvsw)
{
    HumanDet* phd = (HumanDet*)phdh;
    phd->th_area = th_area;
    phd->th_hvsw = th_hvsw;
    phd->pVP->frameCnt = IGNORED_LEN - 1;
    return true;
}

/************************************************************************/
/* obj后处理，包括合并轨迹，判定是否有移动物体，真实obj输出             */
/************************************************************************/
int ObjPostProcess(ObjList *objList, HumanDet* phd/*, ReturnObjs *out_objs*/)
{
	//HumanDet* phd = (HumanDet*)phdHandle;
	int flag = 1;
	int i,j;
	ObjNode *pNO_1 = objList->pObjHead;
	ObjNode *pNO_2 = NULL;
	ObjNode *pNext = NULL;

    float max_area = 0.0;
    int main_cx,main_cy,main_width,main_height;
    int main_index=-1;

	for (i = 0; i < objList->interestLen; i++)
	{
		pNO_1 = pNO_1->next;		
		pNO_2 = pNO_1;
		if (!pNO_1->obj.isMatched)			//只处理找到对应blob的object
		{
			continue;
		}
		int tt = objList->interestLen;

		//判断是否有两个obj对应到同一个blob上，有的话则合并
		for (j = i+1; j < tt; j++)
		{
			pNext = pNO_2->next;
			if (!pNext->obj.isMatched)
			{
				continue;
			}
			if (pNext->obj.ID == pNO_1->obj.ID)	//两个目标移动到同一blob上
			{
				pNO_1->obj.isActive = pNO_1->obj.isActive||pNext->obj.isActive;
				pNO_1->obj.traceLen = pNO_1->obj.traceLen > pNext->obj.traceLen ? pNO_1->obj.traceLen : pNext->obj.traceLen;

				pNO_2->next = pNext->next;
				delete pNext;
				objList->len --;
				objList->interestLen --;
			}
			else {if (pNO_2->next != NULL) pNO_2 = pNO_2->next;} //可以不加if
		}

        //逻辑调整
        if (phd->out_objs->num < MAX_DETECT_NUM)	//真实目标输出
        {	
            //宽高都要减一，是因为像素点坐标和目标框之间存在歧义
            phd->out_objs->obj[phd->out_objs->num].cx = pNO_1->obj.x*1.0/phd->pVP->w;
            phd->out_objs->obj[phd->out_objs->num].cy = pNO_1->obj.y*1.0/phd->pVP->h;
            phd->out_objs->obj[phd->out_objs->num].area = (pNO_1->obj.w*pNO_1->obj.h*1.0)/(phd->pVP->w*phd->pVP->h);
            phd->out_objs->obj[phd->out_objs->num].hvsw = pNO_1->obj.h*1.0/pNO_1->obj.w;
            //phd->out_objs->obj[phd->out_objs->num].width = pNO_1->obj.w*1.0/phd->pVP->w;;
            //phd->out_objs->obj[phd->out_objs->num].height = pNO_1->obj.h*1.0/phd->pVP->h;
            phd->out_objs->obj[phd->out_objs->num].bflag = 0;
            phd->out_objs->obj[phd->out_objs->num].nof = 0;
            phd->out_objs->obj[phd->out_objs->num].xspeed = 0.0;
            //判断当前obj是否为一个真实的目标
            if (/*pNO_1->obj.isActive &&*/ pNO_1->obj.isMatched /*&& pNO_1->obj.w < pNO_1->obj.h*/)    //去掉isactive条件，避免该条件产生的ghoast现象
            {
#ifdef HISTORY_PARAM
                int diff_x = pNO_1->obj.hist_x[pNO_1->obj.idx] - pNO_1->obj.hist_x[(pNO_1->obj.idx+1)%MAX_TRACE_LENGTH];
                phd->out_objs->obj[phd->out_objs->num].xspeed = (float)diff_x*10/((MAX_TRACE_LENGTH-1)*phd->pVP->w);
                int diff_y = pNO_1->obj.hist_y[pNO_1->obj.idx] - pNO_1->obj.hist_y[(pNO_1->obj.idx+1)%MAX_TRACE_LENGTH];
				phd->out_objs->obj[phd->out_objs->num].yspeed = (float)diff_y*10/((MAX_TRACE_LENGTH-1)*phd->pVP->h);
                //float y_speed = (float)diff_y*10/((MAX_TRACE_LENGTH-1)*phd->pVP->w);
                if (diff_y>0 && abs(diff_y)>pNO_1->obj.h/6)
                {
                    phd->out_objs->obj[phd->out_objs->num].nof = -1;
                } 
                else if(diff_y<0 && abs(diff_y)>pNO_1->obj.h/6)
                {
                    phd->out_objs->obj[phd->out_objs->num].nof = 1;
                }
#endif
                if (pNO_1->obj.w*1.0/phd->pVP->w>MINIWH&&pNO_1->obj.h*1.0/phd->pVP->h>MINIWH&&
                    (phd->out_objs->obj[phd->out_objs->num].hvsw > MINHWRT &&phd->out_objs->obj[phd->out_objs->num].hvsw <MAXHWRT && phd->out_objs->obj[phd->out_objs->num].area >phd->th_area) && phd->out_objs->obj[phd->out_objs->num].area < phd->th_areamax)
                {
                    flag = 3;	//找到真实轨迹
                    phd->out_objs->obj[phd->out_objs->num].bflag = 1;
 
                    // tracking or not ?
                    if (phd->use_tracker && phd->out_objs->obj[phd->out_objs->num].area > max_area) //首先max_area为0，后面循环整个obj序列的时候找到最大面积的obj
                    {
                        max_area = phd->out_objs->obj[phd->out_objs->num].area;    //将该面积设为max_area
                        main_index = phd->out_objs->num;
						main_cx = pNO_1->obj.x;                            
						main_cy = pNO_1->obj.y;
                        main_width = pNO_1->obj.w;
                        main_height = pNO_1->obj.h;

                        /*main_cx = pNO_1->obj.x+SPEEDFIX*diff_x;                             //加上速度量，将弥补帧间隔时间目标发生了运动距离
                        main_cy = pNO_1->obj.y+SPEEDFIX*diff_y;
                        if(main_cx<=0)main_cx=1;
						else if(main_cx+main_width>=phd->pVP->w) main_cx=main_cx-SPEEDFIX*diff_x-0.5;
						if(main_cy<=0)main_cy=1;
						else if(main_cy+main_height>=phd->pVP->h) main_cy=main_cy-SPEEDFIX*diff_y-0.5;*/

                    }
                }
            }

            phd->out_objs->num++;
        }else{            //-! added 如果目标过多，就重新建模，应对开关灯和设备抖动的情况
            phd->pVP->frameCnt = IGNORED_LEN / 2;
            return 1;
        }
	}


    if (flag == 3 && phd->use_tracker)
    {
        //float main_xspeed = phd->out_objs->obj[main_index].xspeed;
        float main_fcx = phd->out_objs->obj[main_index].cx;
		float main_fcy= phd->out_objs->obj[main_index].cy;

        // tracking or not
		//if ((main_xspeed > 0.15 && main_fcx > 0.5) || (main_xspeed < -0.15 && main_fcx < 0.5))
        if ( main_fcx > DETECTWITH && main_fcx < 1-DETECTWITH &&main_fcy > DETECTWITH && main_fcy < 1-DETECTWITH)
        {
            //tracking!
            //redirect output objects to the tracking one
            if (main_index != 0)
            {
                phd->out_objs->obj[0].area = phd->out_objs->obj[main_index].area;
                phd->out_objs->obj[0].cx = phd->out_objs->obj[main_index].cx;
                phd->out_objs->obj[0].cy = phd->out_objs->obj[main_index].cy;
                phd->out_objs->obj[0].hvsw = phd->out_objs->obj[main_index].hvsw;
            }
            phd->out_objs->obj[0].xspeed = 0.0;
            phd->out_objs->obj[0].nof = 0;
            phd->out_objs->obj[0].bflag = true;
            phd->out_objs->num = 1;

            //init tracker
            phd->track_cx = main_cx*phd->pVP->blockD + phd->pVP->roi_x;
            phd->track_cy = main_cy*phd->pVP->blockD + phd->pVP->roi_y;
            phd->track_width = main_width*phd->pVP->blockD;
            phd->track_heigth = main_height*phd->pVP->blockD;
            phd->md_track_flag = 1;

			if( main_fcx< SIDEWITH || main_fcx > 1-SIDEWITH) 
			{
				phd->sidetrackcnt=0;phd->sidetrack=true;
				
			}else phd->sidetrack=false;

			if(main_fcx< SIDEWITH )          phd->track_cx +=SIDEFIX*phd->track_width;  
			else if(main_fcx > 1-SIDEWITH)   phd->track_cx -=SIDEFIX*phd->track_width;
			
            flag = TRACKING_MODE;
        }
#ifdef SHOWPRINT
		else    printf("not within tracking area range!\n");
#endif
    }
	return flag;
}

ReturnObjs* getObjs(HumanDetHandle phdHandle)
{
	HumanDet* phd = (HumanDet*)phdHandle;
	return phd->out_objs;
}

