#include "HDcommon.h"
#include "VibeModel.h"
#include "Morph.h"
#include "MultiObjTrack.h"
#include "HDalg.h"
//#include<math.h>
#include<list>
#include <iostream>
#include <vector>


int abs_int(int x)
{
   if(x>=0) return x;
   else return -x;
}

int pnpoly (int nvert, int *vertx, int *verty, int testx, int testy) 
{
	int i, j, c = 0;
	for (i = 0, j = nvert-1; i < nvert; j = i++) 
	{
		if ( ( (verty[i]>testy) != (verty[j]>testy) ) &&(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
			c = !c;
	}
	return c;
}

typedef struct HumanDet{
	VibeParam *pVP;
	vector<HdRect> blobList;
	HdSize operateSZ;
	MatchParam* pMP;
	ObjList *objlist;
	bool needInit;
	ReturnObjs *out_objs;

    float th_area;  //-! mark added
    float th_hvsw;
	float th_dist;       
	float th_distmod;
}HumanDet;

//初始化，包括各个模块的初始化
HumanDetHandle CreateBackModel()
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

    phd->th_area = 0.004;//-! mark added
    phd->th_hvsw = 0.3;
	phd->th_dist=-1;                      //这两个距离参数与 vibe中的blockD的大小有关系，即与图像大小、选择侦测区域大小有关系
	phd->th_distmod=-1;
	return (HumanDetHandle)phd;
}

int ObjPostProcess(ObjList *objList, HumanDet* phd/*, ReturnObjs *out_objs*/);

void ChangeGrayPic(uchar *gray,int* vertxx,  int* vertyy,const int nvert, int x, int y, int w, int h,const int step,const int width, const int height)
{
	if(x>1) x--;
	if(y>1) y--;
	if(x+w<width-2) w=w+2;
	if(y+h<height-2) h=h+2;
	for(int i=y;i<y+h;i++)
		for(int j=x;j<x+w;j++)
		{
		    if(!pnpoly(nvert,vertxx,vertyy,j,i)) gray[i*step+j]=255;
		}
}
void calculateRealFg(uchar* src, int* vertxx,  int* vertyy,const int nvert, const int x,const int y,const int width,const int height,const int blockD)
{
	
	int* vertx=new int[nvert];
	int* verty=new int[nvert];
	for(int i=0;i<nvert;i++)
	{
	   vertx[i]=(vertxx[i]-x+blockD/2)/blockD;
	   verty[i]=(vertyy[i]-y+blockD/2)/blockD;
	}
	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++)
		{
		    if(!pnpoly(nvert,vertx,verty,j,i)&&src[i*width+j]==255) src[i*width+j]=0;
		}
	delete vertx;
	delete verty;
}

//整体逻辑过程
int UpdateAndHD(HumanDetHandle phdHandle, uchar* gray, const int width, const int height, const int step, unsigned int sensitive/*, ReturnObjs *out_objs*/)
{
	HumanDet* phd = (HumanDet*)phdHandle;
	int ret = 0;
	phd->out_objs->num = 0;

	/*if(phd->pVP->multiedge==true)
	{
		ChangeGrayPic(gray,phd->pVP->vertx,phd->pVP->verty,phd->pVP->nvert,phd->pVP->roi_x,phd->pVP->roi_y,phd->pVP->roi_w,phd->pVP->roi_h,step,width,height);
	}*/
	int flag = UpdateBgModel( gray, width, height, step, sensitive, phd->pVP ) ;

	if(phd->pVP->multiedge==true)
	{
        calculateRealFg(phd->pVP->pFgImg,phd->pVP->vertx,phd->pVP->verty,phd->pVP->nvert,phd->pVP->roi_x,phd->pVP->roi_y,phd->pVP->w,phd->pVP->h,phd->pVP->blockD);
	}

	if (flag == INIT_MODULE)
	{
		phd->needInit = true;
	}
	else if (flag == ENOUGH_POINTS)
	{		
		ret = 1;
		if(fastFindContourRects(phd->pVP->pFgImg,phd->pVP->w,phd->pVP->h,phd->pVP->w,phd->blobList,phd->operateSZ,0.4) > 0)   
		{

			if (phd->needInit)
			{
				InitObjList(phd->pMP,phd->objlist,phd->blobList);
				phd->needInit = false;
			}
			else
			{
				MultiTraceProcess(phd->pMP,phd->objlist,phd->blobList);
				ret = ObjPostProcess(phd->objlist, phd/*,out_objs*/);
                //if (phd->out_objs->num>0)
                //{
                //    ret = 3;
                //}
			}						
		}
		else
		{
			MultiTraceProcess(phd->pMP,phd->objlist,phd->blobList);
		}
	}
	else if(flag == NOT_ENOUTH_POINTS)	//设备上期初没有这个！！
	{
		MultiTraceProcess(phd->pMP,phd->objlist,phd->blobList);
	}
	phd->blobList.clear();


	if(ret==3)                               /**********11.22号版本缺少的逻辑***********/
	{
		int movetimes=0;static int allmovetimes=0;
		for (int kk=0; kk<phd->out_objs->num; kk++)
		{

			if (phd->out_objs->obj[kk].move_dist> phd->th_dist)   movetimes++;

		}
		if(movetimes>=1) allmovetimes++;
		else allmovetimes=0;

       if(allmovetimes>=MOVEDISTTIMES) ret=3;
	   else ret=1;
	}

	return ret;

}

bool ReleaseBackModel(HumanDetHandle* phdHandle)
{
	HumanDet **phd = (HumanDet**)phdHandle;
	if (phd!=NULL && *phd!=NULL)
	{
		ReleaseBgModel(&((*phd)->pVP));
		(*phd)->blobList.clear();
		/*(*phd)->blobList=NULL;*/
		DeleteTrackParam(&((*phd)->pMP));
		DeleteObjList((*phd)->objlist);
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
		phd->pVP->AeraK=1.0;
		phd->pVP->nvert=0;
		phd->pVP->multiedge=false;
		//printf("****************ok:x:%d,y:%d,w:%d,h:%d\n",x,y,w,h);
        return true;
    }
    else if (2 == type)
    {
        phd->pVP->roi_x = -1;
        phd->pVP->roi_y = -1;
        phd->pVP->roi_w = -1;
        phd->pVP->roi_h = -1;
        phd->pVP->frameCnt = IGNORED_LEN - 1;
		phd->pVP->AeraK=1.0;
		phd->pVP->nvert=0;
		phd->pVP->multiedge=false;
        return true;
    }else{
        return false;
    }
        
}

float calculateAreaK( int* vertxx,  int* vertyy,const int nvert,const int x,const int y,const int w,const int h)
{
	int num=0;
	for(int i=y;i<y+h;i++)
		for(int j=x;j<x+w;j++)
		{
			if(pnpoly(nvert,vertxx,vertyy,j,i)) num++;
		}
	if(num==0) num++;
	//printf("num:%d,w*h:%d\n",num,w*h);
	return (num*1.0/(w*h));
}


bool SetROI_MultiEdge(HumanDetHandle phdh, int* vertxx, int* vertyy,const int nvert,const int type)
{
	if (0 == type)
	{
	    if(nvert<3){ 
		    printf("vert number wrong\n");
		    return false; }
	    HumanDet* phd = (HumanDet*)phdh;
        int *vertx=vertxx;
		int *verty=vertyy;
		
	    int xmin,xmax,ymin,ymax;
	    xmin=*vertx;xmax=*vertx; ymin=*verty;ymax=*verty;
	    vertx++;verty++;
	    for(int i=1;i<nvert;i++)
	    {
	   	    if(*vertx<xmin) xmin=*vertx;
		    else if (*vertx>xmax) xmax=*vertx;
		    if(*verty<ymin) ymin=*verty;
		    else if (*verty>ymax) ymax=*verty;
		    vertx++; verty++;
	    }
	    if(xmin==xmax|| ymin==ymax||xmax<1||ymax<1||xmin<0||ymin<0){ 
		    printf("vert range wrong\n");
		    return false;}

		phd->pVP->roi_x = xmin;
		phd->pVP->roi_y = ymin;
		phd->pVP->roi_w = xmax-xmin;
		phd->pVP->roi_h = ymax-ymin;
		phd->pVP->frameCnt = IGNORED_LEN - 1;

		vertx=vertxx;
		verty=vertyy;
		if(phd->pVP->vertx!=NULL) delete phd->pVP->vertx;
		if(phd->pVP->verty!=NULL) delete phd->pVP->verty;
		phd->pVP->nvert=nvert;
		phd->pVP->vertx=new int[nvert];
		phd->pVP->verty=new int[nvert];
		for(int i=0;i<nvert;i++)
		{
			phd->pVP->vertx[i]=*vertx;
			phd->pVP->verty[i]=*verty;
			printf("x:%d,y:%d\n",phd->pVP->vertx[i],phd->pVP->verty[i]);
			vertx++; verty++;
		}
		phd->pVP->AeraK=calculateAreaK(vertxx,vertyy,nvert,xmin,ymin,xmax-xmin,ymax-ymin);
		phd->pVP->multiedge=true;
        printf("x:%d,y:%d,w:%d,h:%d,areak:%.2f\n",xmin,ymin,xmax-xmin,ymax-ymin,phd->pVP->AeraK);
		return true;
	}
	else if (2 == type)
	{
		HumanDet* phd = (HumanDet*)phdh;
		phd->pVP->roi_x = -1;
		phd->pVP->roi_y = -1;
		phd->pVP->roi_w = -1;
		phd->pVP->roi_h = -1;
		phd->pVP->frameCnt = IGNORED_LEN - 1;
		phd->pVP->AeraK=1.0;
		phd->pVP->nvert=0;
		phd->pVP->multiedge=false;
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

bool SetFilterParamDist(HumanDetHandle phdh, float th_dist, float th_distmod)
{
	HumanDet* phd = (HumanDet*)phdh;
	phd->th_dist = th_dist;
	phd->th_distmod = th_distmod;
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
            //phd->out_objs->obj[phd->out_objs->num].area = (pNO_1->obj.w*pNO_1->obj.h*1.0)/(phd->pVP->w*phd->pVP->h*phd->pVP->AeraK);   //目标占多边形的面积比例 
			//phd->out_objs->obj[phd->out_objs->num].AreaK =phd->pVP->AeraK;
			phd->out_objs->obj[phd->out_objs->num].area = (pNO_1->obj.w*pNO_1->obj.h*phd->pVP->AeraR*1.0)/(phd->pVP->w*phd->pVP->h);  //目标占整个图像的面积比例
            phd->out_objs->obj[phd->out_objs->num].AreaK =1.0/phd->pVP->AeraR;

			phd->out_objs->obj[phd->out_objs->num].hvsw = pNO_1->obj.h*1.0/pNO_1->obj.w;
            //phd->out_objs->obj[phd->out_objs->num].width = pNO_1->obj.w*1.0/phd->pVP->w;;
            //phd->out_objs->obj[phd->out_objs->num].height = pNO_1->obj.h*1.0/phd->pVP->h;
            phd->out_objs->obj[phd->out_objs->num].bflag = 0;
            phd->out_objs->obj[phd->out_objs->num].nof = 0;
            phd->out_objs->obj[phd->out_objs->num].xspeed = 0.0;
			


			if(pNO_1->obj.pts.size()>1)                                          ////计算该目标的移动距离、最新一次框的位置   added by gelin 2017-10-11
			{
				//printf("size of obj:%d, size of time:%d\n",(int)pNO_1->obj.pts.size(),(int)pNO_1->obj.pts_time.size());
				time_t now,before;
				_ObjRect nowpos,beforepos,before2pos;
                std::list<time_t>::reverse_iterator i=pNO_1->obj.pts_time.rbegin();
				std::list<_ObjRect>::reverse_iterator j=pNO_1->obj.pts.rbegin();
				now=*i;
				nowpos=*j;  before2pos=*j;
				int count=1;
				phd->out_objs->obj[phd->out_objs->num].move_dist_mod=0;
				phd->out_objs->obj[phd->out_objs->num].move_dist=0;
				i++;j++;
				for(;i!=pNO_1->obj.pts_time.rend();++i,++j)
				{
					count++;
				    before=*i;
					beforepos=*j;
					if((now-before>=MOVETIME||count>=pNO_1->obj.pts.size()-1)&&
						((nowpos.w-beforepos.w)*(nowpos.w-beforepos.w)<RECTCHANGE) && ((nowpos.h-beforepos.h)*(nowpos.h-beforepos.h)<RECTCHANGE) )
					{
					    phd->out_objs->obj[phd->out_objs->num].move_dist=static_cast<float>((nowpos.x-beforepos.x)*(nowpos.x-beforepos.x)
							+(nowpos.y-beforepos.y)*(nowpos.y-beforepos.y));
						phd->out_objs->obj[phd->out_objs->num].move_dist_mod+=static_cast<float>((before2pos.x-beforepos.x)*(before2pos.x-beforepos.x)
							+(before2pos.y-beforepos.y)*(before2pos.y-beforepos.y));
						break;
					}
					if((before2pos.w-beforepos.w)*(before2pos.w-beforepos.w)<RECTCHANGE && (before2pos.h-beforepos.h)*(before2pos.h-beforepos.h)<RECTCHANGE)
					{	phd->out_objs->obj[phd->out_objs->num].move_dist_mod+=static_cast<float>((before2pos.x-beforepos.x)*(before2pos.x-beforepos.x)
							+(before2pos.y-beforepos.y)*(before2pos.y-beforepos.y));
					    before2pos=*j;
					}
				}
				//printf("there is one obj has multi frame");
		    	//phd->out_objs->obj[phd->out_objs->num].move_dist=static_cast<float>((pNO_1->obj.pts.front().x-pNO_1->obj.pts.back().x)*(pNO_1->obj.pts.front().x-pNO_1->obj.pts.back().x)
				//  +(pNO_1->obj.pts.front().y-pNO_1->obj.pts.back().y)*(pNO_1->obj.pts.front().y-pNO_1->obj.pts.back().y));
			}else{
			    phd->out_objs->obj[phd->out_objs->num].move_dist=0;
				phd->out_objs->obj[phd->out_objs->num].move_dist_mod=0;
			}
			
			for(int i=0;i<MAX_PT;i++)
			{
				phd->out_objs->obj[phd->out_objs->num].pts[i].x=0;
				phd->out_objs->obj[phd->out_objs->num].pts[i].y=0;
				phd->out_objs->obj[phd->out_objs->num].pts[i].w=0;
				phd->out_objs->obj[phd->out_objs->num].pts[i].h=0;
			}
			if(pNO_1->obj.pts.size()<=MAX_PT)
			{   int nm=0;
				for(std::list<_ObjRect>::iterator it=pNO_1->obj.pts.begin();it!=pNO_1->obj.pts.end();it++)
				{
			     phd->out_objs->obj[phd->out_objs->num].pts[nm].x=(*it).x;
                 phd->out_objs->obj[phd->out_objs->num].pts[nm].y=(*it).y;
			     phd->out_objs->obj[phd->out_objs->num].pts[nm].w=(*it).w;
			     phd->out_objs->obj[phd->out_objs->num].pts[nm].h=(*it).h;  
				 nm++;
				}
			}else{
				std::list<_ObjRect>::iterator it=pNO_1->obj.pts.end();
				for(int nm=MAX_PT;nm!=0;nm--)
                { 
                 it--;
			     phd->out_objs->obj[phd->out_objs->num].pts[nm].x=(*it).x;
                 phd->out_objs->obj[phd->out_objs->num].pts[nm].y=(*it).y;
			     phd->out_objs->obj[phd->out_objs->num].pts[nm].w=(*it).w;
			     phd->out_objs->obj[phd->out_objs->num].pts[nm].h=(*it).h; 
				}
			}                                                                          ////修改结束
			 

            //判断当前obj是否为一个真实的目标
            if (pNO_1->obj.isActive&&pNO_1->obj.isMatched /*&& pNO_1->obj.w < pNO_1->obj.h*/)
            {
#ifdef HISTORY_PARAM
                int diff_x = pNO_1->obj.hist_x[pNO_1->obj.idx] - pNO_1->obj.hist_x[(pNO_1->obj.idx+1)%MAX_TRACE_LENGTH];
                phd->out_objs->obj[phd->out_objs->num].xspeed = (float)diff_x*10/((MAX_TRACE_LENGTH-1)*phd->pVP->w);
                int diff_y = pNO_1->obj.hist_y[pNO_1->obj.idx] - pNO_1->obj.hist_y[(pNO_1->obj.idx+1)%MAX_TRACE_LENGTH];
                //float y_speed = (float)diff_y*10/((MAX_TRACE_LENGTH-1)*phd->pVP->w);
                if (diff_y>0&&abs_int(diff_y)>pNO_1->obj.h/6)
                {
                    phd->out_objs->obj[phd->out_objs->num].nof = -1;
                } 
                else if(diff_y<0&&abs_int(diff_y)>pNO_1->obj.h/6)
                {
                    phd->out_objs->obj[phd->out_objs->num].nof = 1;
                }
#endif
                if (phd->out_objs->obj[phd->out_objs->num].hvsw > phd->th_hvsw &&/*phd->out_objs->obj[phd->out_objs->num].hvsw < 4.5 &&*/
                    phd->out_objs->obj[phd->out_objs->num].area > phd->th_area &&
					phd->out_objs->obj[phd->out_objs->num].move_dist> phd->th_dist && phd->out_objs->obj[phd->out_objs->num].move_dist_mod>phd->th_distmod)
                {
                    flag = 3;	//找到真实轨迹
                    phd->out_objs->obj[phd->out_objs->num].bflag = 1;
                }
            }
            phd->out_objs->num++;
        }else{            //-! added 如果目标过多，就重新建模，应对开关灯和设备抖动的情况
            phd->pVP->frameCnt = IGNORED_LEN / 2;
            return 1;
        }
	}
	return flag;
}

ReturnObjs* getObjs(HumanDetHandle phdHandle)
{
	HumanDet* phd = (HumanDet*)phdHandle;
	//	printf("num:%d, address:%d\n",phd->out_objs->num,&(phd->out_objs->num));
	return phd->out_objs;
}




