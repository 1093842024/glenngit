#ifndef _HDCOMMON_H_
#define _HDCOMMON_H_
//常用数据类型的定义
#define FOR_DEBUG_USE_
#define HISTORY_PARAM

typedef unsigned char uchar;
typedef struct HdSize{
	int width;
	int height;
}HdSize;

typedef struct HdRect
{
	int x;
	int y;
	int width;
	int height;
}HDRect;

inline HdSize  hdSize( int width, int height )
{
	HdSize s;

	s.width = width;
	s.height = height;

	return s;
}
inline HdRect  hdRect( int x, int y, int width, int height )
{
	HdRect s;

	s.x = x;
	s.y = y;
	s.width = width;
	s.height = height;

	return s;
}

//const int blockRadius = 4;	//采样半径，实际是每(2*blockRadius+1)*(2*blockRadius+1)个点采样一个点
#endif