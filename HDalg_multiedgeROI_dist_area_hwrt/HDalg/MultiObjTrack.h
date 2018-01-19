#ifndef _MULTIOBJTRACK_H_
#define _MULTIOBJTRACK_H_

#include <iostream>
#include <vector>
#include <queue>
#include "HDcommon.h"
#include<list>
#include <time.h>   
using std::vector;


#define MAX_OBJ_NUM 50
#define MAX_BLOB_NUM 50
#define MAX_TRACE_LENGTH 5
#define START_WITH 1
#define MAX_RECT_NUM 60


typedef struct _ObjRect                                    //可能目标的矩形框信息 added by gelin 2017-8-23
{
	int x;      //左上角位置x y
	int y;
	int w;
	int h;

}_ObjRect;


typedef struct MatchParam
{
	int nearDistanceTH;	// 最近距离门限
	int farDistanceTH;	// 最远距离门限

	int trackFollows[MAX_OBJ_NUM][MAX_BLOB_NUM];
	int followsCount[MAX_OBJ_NUM];
}MatchParam;

typedef struct SingleObject
{
	int x,y;		// 中心点坐标
	int w,h;		// 目标大小
	int ID;			// blob ID
	int traceLen;	// 出现次数，最大为 MAX_TRACE_LENGTH
#ifdef HISTORY_PARAM
    int hist_x[MAX_TRACE_LENGTH];                      ////存最近五次该目标的中心位置，
    int hist_y[MAX_TRACE_LENGTH];
    //int hist_area[MAX_OBJ_NUM];
    int idx;       // 当前指向的位置
#endif

	std::list<_ObjRect> pts;                          ////多帧（60）中出现有效目标的矩形框信息的队列    added by gelin 2017-8-23
	std::list<time_t> pts_time;

	bool isMatched;	// 是否找到匹配blob
	bool isActive;	// 是否为有效目标 1:有效；0:临时目标
					// isMathced和isActive同时为1时才报警
}SingleObject;

typedef struct ObjNode                                            ////单独一个singleobject的头信息，用于建立Objlist
{
	SingleObject obj;
	ObjNode *next;
}ObjNode;

typedef struct ObjList   //多个目标的list
{
	ObjNode *pObjHead;
	int len;			// 当前帧中object数目
	int interestLen;	// 需要后处理的object数目
}ObjList;

MatchParam* CreateTrackParam(int minTH, int maxTH);
bool InitObjList(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList);
bool DeleteObjList(ObjList *objList);
bool MultiTraceProcess(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList);
bool DeleteTrackParam(MatchParam **ppMP);
#endif