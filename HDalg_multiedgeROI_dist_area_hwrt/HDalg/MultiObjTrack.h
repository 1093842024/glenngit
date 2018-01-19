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


typedef struct _ObjRect                                    //����Ŀ��ľ��ο���Ϣ added by gelin 2017-8-23
{
	int x;      //���Ͻ�λ��x y
	int y;
	int w;
	int h;

}_ObjRect;


typedef struct MatchParam
{
	int nearDistanceTH;	// �����������
	int farDistanceTH;	// ��Զ��������

	int trackFollows[MAX_OBJ_NUM][MAX_BLOB_NUM];
	int followsCount[MAX_OBJ_NUM];
}MatchParam;

typedef struct SingleObject
{
	int x,y;		// ���ĵ�����
	int w,h;		// Ŀ���С
	int ID;			// blob ID
	int traceLen;	// ���ִ��������Ϊ MAX_TRACE_LENGTH
#ifdef HISTORY_PARAM
    int hist_x[MAX_TRACE_LENGTH];                      ////�������θ�Ŀ�������λ�ã�
    int hist_y[MAX_TRACE_LENGTH];
    //int hist_area[MAX_OBJ_NUM];
    int idx;       // ��ǰָ���λ��
#endif

	std::list<_ObjRect> pts;                          ////��֡��60���г�����ЧĿ��ľ��ο���Ϣ�Ķ���    added by gelin 2017-8-23
	std::list<time_t> pts_time;

	bool isMatched;	// �Ƿ��ҵ�ƥ��blob
	bool isActive;	// �Ƿ�Ϊ��ЧĿ�� 1:��Ч��0:��ʱĿ��
					// isMathced��isActiveͬʱΪ1ʱ�ű���
}SingleObject;

typedef struct ObjNode                                            ////����һ��singleobject��ͷ��Ϣ�����ڽ���Objlist
{
	SingleObject obj;
	ObjNode *next;
}ObjNode;

typedef struct ObjList   //���Ŀ���list
{
	ObjNode *pObjHead;
	int len;			// ��ǰ֡��object��Ŀ
	int interestLen;	// ��Ҫ�����object��Ŀ
}ObjList;

MatchParam* CreateTrackParam(int minTH, int maxTH);
bool InitObjList(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList);
bool DeleteObjList(ObjList *objList);
bool MultiTraceProcess(MatchParam *pMP, ObjList *objList, vector<HdRect> &blobList);
bool DeleteTrackParam(MatchParam **ppMP);
#endif