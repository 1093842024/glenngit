#include "MultiObjTrack.h"
//#include <math.h>
#include <stdlib.h>

MatchParam* CreateTrackParam(int minTH, int maxTH)
{
	MatchParam *tmp = new MatchParam;
	tmp->nearDistanceTH = minTH;
	tmp->farDistanceTH	= maxTH;

	return tmp;
}

bool DeleteTrackParam(MatchParam **ppMP)
{
	if (ppMP!=NULL&&*ppMP!=NULL)
	{
		delete *ppMP;
	}
	return true;
}

/************************************************************************/
/* 简单的初始化，将所有blob初始化为object(before)						*/
/* 初始化时每个blob与objList中blob做距离比较							*/
/* 小于2倍最近距离门限(nearDistanceTH)的blob则不新建obj(now)			*/
/************************************************************************/
bool InitObjList(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList)
{
	ObjNode *p = objList->pObjHead;
	while(p!=NULL)
	{
		ObjNode *tmp = p;
		p = p->next;
		delete tmp;
	}
	objList->len = 0;
	objList->pObjHead = new ObjNode;
	objList->pObjHead->next = NULL;
	p = objList->pObjHead;

	int blobx, bloby, objx, objy;
	int diffX, diffY, distance;
	bool needNewObj;
	ObjNode *pNode = NULL;

	//初始化时每个blob与objList中blob做距离比较，
	//小于2倍的最近距离门限(nearDistanceTH)的blob则不新建obj
	for (int i = 0; i < blobList.size(); i++)
	{	
		blobx = blobList.at(i).x + blobList.at(i).width/2;
		bloby = blobList.at(i).y + blobList.at(i).height/2;
		needNewObj = true;
		pNode = objList->pObjHead;
		for (int j = 0; j < objList->len; j++)
		{
			pNode = pNode->next;
			objx = pNode->obj.x;
			objy = pNode->obj.y;
			diffX = objx - blobx;
			diffY = objy - bloby;
			distance = diffX*diffX + diffY*diffY/2;

			if (distance < 2*pMP->nearDistanceTH)
			{
				needNewObj = false;
				break;
			}
		}
		if (needNewObj && objList->len < MAX_OBJ_NUM)
		{
			HdRect *tmp = &(blobList.at(i));
			ObjNode *objNode = new ObjNode;
			objNode->obj.w = tmp->width;
			objNode->obj.h = tmp->height;
			objNode->obj.x = tmp->x + tmp->width/2;
			objNode->obj.y = tmp->y + tmp->height/2;
			objNode->obj.isActive = false;
			objNode->obj.isMatched = true;
			objNode->obj.traceLen = START_WITH;
#ifdef HISTORY_PARAM
            objNode->obj.idx = 0;
            //objNode->obj.hist_area[0] = objNode->obj.w*objNode->obj.h;
            objNode->obj.hist_x[0] = objNode->obj.x;
            objNode->obj.hist_y[0] = objNode->obj.y;
#endif
			objNode->next  = NULL;

			p->next = objNode;
			p = p->next;
			objList->len ++;
		}		
	}
//	objList->len = blobList->size();
	return true;
}

bool DeleteObjList(ObjList *objList)
{
	ObjNode *p = objList->pObjHead;
	while(p!=NULL)
	{
		ObjNode *tmp = p;
		p = p->next;
		delete tmp;
	}
	objList->pObjHead = NULL;
	return true;
}

/************************************************************************/
/* Blob选择轨迹，如果没有找到适合的则新建轨迹                           */
/************************************************************************/
void BlobChooseTrack(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList)
{
	int i,j;
	int blobx, bloby, objx, objy;
	int blobw, blobh, objw, objh;
	double ratew,rateh;
	int diffX, diffY, distance;
	bool find_In_th;
	int  minDisTHIndex;
	int  minDis;

	ObjNode *pON = objList->pObjHead;
	ObjNode *pNext = NULL;

	objList->interestLen = objList->len;	
	for (i=0; i < MAX_OBJ_NUM; i++)		//将obj对应blob数清空
	{
		pMP->followsCount[i] = 0;
	}
	if (blobList.size() == 0)			//blob列表为空
	{
		while(pON->next!=NULL)
		{
			pNext = pON->next;
			pNext->obj.isMatched = false;
			pNext->obj.isActive = false;
			if (--pNext->obj.traceLen < 0)	//obj生存时间耗完，删除这条轨迹
			{
				pON->next = pNext->next;
				objList->len --;
				delete pNext;
			}
			if (pON->next!=NULL)
			{
				pON = pON->next;
			}
		}
	}
	else
	{
		//blob选track
		for (i = 0; i < blobList.size(); i++)
		{
			minDisTHIndex = -1;
			/*minDis = pMP->farDistanceTH;*/
			minDis = 100000;
			find_In_th = false;
			blobx = blobList.at(i).x + blobList.at(i).width/2;
			bloby = blobList.at(i).y + blobList.at(i).height/2;
			blobw = blobList.at(i).width;
			blobh = blobList.at(i).height;
			pON = objList->pObjHead;
			for (j = 0; j < objList->len; j++)
			{	
				pON = pON->next;
				objx = pON->obj.x;
				objy = pON->obj.y;
				objw = pON->obj.w;
				objh = pON->obj.h;
				diffX = blobx - objx;
				diffY = bloby - objy;
				distance = diffX*diffX + diffY*diffY/2;
				ratew = blobw > objw ? (1.0*blobw/objw):(1.0*objw/blobw);
				rateh = blobh > objh ? (1.0*blobh/objh):(1.0*objh/blobh);
				if (ratew*rateh > 5.0)
				{
					continue;
				}
				if (distance < pMP->nearDistanceTH)
				{
					find_In_th = true;
					if (j < objList->interestLen)		//超过interestLen之后的obj为本帧新建的，不做其他处理
					{
						pMP->trackFollows[j][pMP->followsCount[j]++] = i;
					}					
				}
				/*else if (distance >= pMP->nearDistanceTH && distance <pMP->farDistanceTH)*/
				else if (abs(diffX)<=(blobw+objw)*3/4 && abs(diffY)<=(blobh+objh)/2)
				{
					if (minDis > distance)
					{
						minDis = distance;
						minDisTHIndex = j;
					}
				}
			}
			//检测blob没有匹配到轨迹，应该新建
			if (!find_In_th && minDisTHIndex == -1)
			{
				
				ObjNode *objNode = new ObjNode;
				objNode->obj.w = blobList.at(i).width;
				objNode->obj.h = blobList.at(i).height;
				objNode->obj.x = blobList.at(i).x + blobList.at(i).width/2;
				objNode->obj.y = blobList.at(i).y + blobList.at(i).height/2;
				objNode->obj.isActive = false;
				objNode->obj.isMatched = true;
				objNode->obj.traceLen = START_WITH;
#ifdef HISTORY_PARAM
                objNode->obj.idx = 0;
                //objNode->obj.hist_area[0] = objNode->obj.w*objNode->obj.h;
                objNode->obj.hist_x[0] = objNode->obj.x;
                objNode->obj.hist_y[0] = objNode->obj.y;
#endif
				objNode->next  = pON->next;
				pON->next = objNode;
				objList->len ++;
			}
			//在中间范围找到一个最小的匹配上
			//超过interestLen之后的obj为本帧新建的，不做处理
			if (!find_In_th && minDisTHIndex != -1 && minDisTHIndex<objList->interestLen)
			{
				pMP->trackFollows[minDisTHIndex][pMP->followsCount[minDisTHIndex]++] = i;
			}
		}
	}
}

/************************************************************************/
/* 轨迹从选择自己的blob中选择一个距离最近的作为obj这一帧的位置          */
/************************************************************************/
void TrackFilterBlob(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList)
{
	int i,j,fSize;
	int blobIndex;
	int bestIndex;
	int bestDistance;
	int blobx, bloby, objx, objy;
	int diffX, diffY, distance;

	if (blobList.size() == 0)	//上面函数已经处理完这种情况，再运行将处理两次。
	{
		return;
	}

	//轨迹筛选blob
	ObjNode *pON = objList->pObjHead;
	ObjNode *pNext = NULL;
	int thi = objList->interestLen;						//interestLen不可以用在这里，会直接跳过n个obj，n为删除obj个数
//	for (i = 0; i < objList->interestLen; i++)			//--这里逻辑有问题，重要！！！
	for (i = 0; i < thi; i++)
	{													
		fSize = pMP->followsCount[i];
		pNext = pON->next;
		if (fSize == 0)
		{
			//没人选的情况
			pNext->obj.isMatched = false;
			pNext->obj.isActive = false;	//--added!
			if (--pNext->obj.traceLen < 0)
			{
				pON->next = pNext->next;
				delete pNext;
				objList->len --;
				objList->interestLen --;
				continue;					//--added 在删obj时，pON不可以往下移位
			}								//--跳过末尾的pON = pON->next;
		}
		else 							
		{
			if (fSize == 1)	//只有一个blob选择该obj
			{
				bestIndex = pMP->trackFollows[i][0];
			}
			else	//超过一个blob选择该obj
			{
				bestIndex = -1;
				bestDistance = 100000;
				objx = pNext->obj.x;
				objy = pNext->obj.y;
				for (j = 0; j < fSize; j++)
				{
					blobIndex = pMP->trackFollows[i][j];
					blobx = blobList.at(blobIndex).x + blobList.at(blobIndex).width/2;//-- 这里是否存在越界可能，fsize和trackFollows冲突了？
					bloby = blobList.at(blobIndex).y + blobList.at(blobIndex).height/2;

					diffX = blobx - objx;
					diffY = bloby - objy;
					distance = diffX*diffX + diffY*diffY/2;

					if (distance < bestDistance)
					{
						bestDistance = distance;
						bestIndex = blobIndex;
					}
				}			
			}
			if (bestIndex != -1)//中心点和大小一起动，有可能出现超出屏幕，大小固定为blob的大小。
			{
				pNext->obj.ID = bestIndex;
				pNext->obj.isMatched = true;
				pNext->obj.x = blobList.at(bestIndex).x + blobList.at(bestIndex).width/2;
				pNext->obj.y = blobList.at(bestIndex).y + blobList.at(bestIndex).height/2;
//				int ts = tableScale[pNext->obj.traceLen];
//				pNext->obj.w = (pNext->obj.w*ts + blobList->at(bestIndex).width)/(ts+1);
//				pNext->obj.h = (pNext->obj.h*ts + blobList->at(bestIndex).height)/(ts+1);
				pNext->obj.w =	blobList.at(bestIndex).width;
				pNext->obj.h =	blobList.at(bestIndex).height;
#ifdef HISTORY_PARAM
                pNext->obj.idx = (pNext->obj.idx+1)%MAX_TRACE_LENGTH;
                //pNext->obj.hist_area[pNext->obj.idx] = pNext->obj.w*pNext->obj.h;
                pNext->obj.hist_x[pNext->obj.idx] = pNext->obj.x;
                pNext->obj.hist_y[pNext->obj.idx] = pNext->obj.y;
#endif
				if(pNext->obj.traceLen < MAX_TRACE_LENGTH)
				{
					pNext->obj.traceLen ++;
				}
				if (pNext->obj.traceLen == MAX_TRACE_LENGTH)
				{
					pNext->obj.isActive = true;
				}
			}
		}
		pON = pON->next; //移向下一个节点
	}
}



bool MultiTraceProcess(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList)
{
	BlobChooseTrack(pMP,objList,blobList);
	TrackFilterBlob(pMP,objList,blobList);

	return true;
}