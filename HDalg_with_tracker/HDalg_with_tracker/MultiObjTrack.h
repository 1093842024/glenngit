#ifndef _MULTIOBJTRACK_H_
#define _MULTIOBJTRACK_H_

#include <iostream>
#include <vector>
#include "HDcommon.h"
using std::vector;


#define MAX_OBJ_NUM 50
#define MAX_BLOB_NUM 50
#define MAX_TRACE_LENGTH 3
#define START_WITH 1

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
    int hist_x[MAX_TRACE_LENGTH];
    int hist_y[MAX_TRACE_LENGTH];
    //int hist_area[MAX_OBJ_NUM];
    int idx;       // 当前指向的位置
#endif

	bool isMatched;	// 是否找到匹配blob
	bool isActive;	// 是否为有效目标 1:有效；0:临时目标
					// isMathced和isActive同时为1时才报警
}SingleObject;

typedef struct ObjNode
{
	SingleObject obj;
	ObjNode *next;
}ObjNode;

typedef struct ObjList
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