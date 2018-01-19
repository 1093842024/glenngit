#ifndef __TRACK_H__
#define __TRACK_H__

//#include "datatype.h"
//#define OPENCVKALMAN
#define CKALMAN

#ifdef OPENCVKALMAN
#include "cv.h"
#endif

#ifdef CKALMAN
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#endif

#include <vector>
/*
struct Object{                       ////added by gelin 20178-25
	float x;
	float y;
	float w;
	float h;
	int ID;
	float vx;
	float vy;
	float vw;      
	float vh;

};
typedef struct Point{
	float x;
	float y;
}point;
typedef struct KLRect{
	float x;
	float y;
	float w;
	float h;
}KLRect;                               ////end added
*/
struct Object{                       ////added by gelin 20178-25
	float x;
	float y;
	float w;
	float h;
	int ID;
	float vx;
	float vy;
};
typedef struct Point{
	float x;
	float y;
}point;
typedef struct KLRect{
	float x;
	float y;
}KLRect;                               ////end added



#ifdef OPENCVKALMAN
////////////////////////////////////////////////////////////////////////
class KalmanTracker
{
public:
	KalmanTracker();
	~KalmanTracker();

	void Init(const Object& obj);
	void Predict();
	void Update(const KLRect& obsv);        ////added by gelin 2017-8-24
	void Update(const CvRect& obsv);
    void DrawTraj(IplImage* pFrame) const;

	void Missed();
    float GetMoveDist() const;
	int GetMissCount() const {return missCount_; }
	const Object& GetObject() const {return obj_;}
	int GetAppearCount() const {return appearCount_;}
	void SetID(size_t nID){obj_.ID = nID;}
	size_t GetID() const {return obj_.ID;}

	
private:
	Object obj_;
	//std::vector<CvPoint> traj_;
	std::vector<Point> traj_;                   ////added by gelin 2017-8-24
	std::vector<Point> trajwh_;
	

	float startX_;
	float startY_;
	float startW_;
	float startH_;                               ////end added


	CvKalman* kalman_;
	CvMat *measurement_;

	int missCount_;
	int appearCount_;
};


#endif


////////////////////////////////////////////////////////////////////////

#ifdef CKALMAN

class  CKalman
{
public:
	CKalman();
	~CKalman();

	void Init(const Object& obj);
	void Predict(int n,int m);
	void Update(const KLRect& obsv,int n,int m);        ////added by gelin 2017-8-24
	const Object& GetObject() const {return obj_;}

protected:

private:
	int brinv(double a[], int n);

	Object obj_;
	std::vector<Point> traj_;                   ////added by gelin 2017-8-24
	//std::vector<Point> trajwh_;

    float *Qk;      //״̬����Э�������   
    float *Rk;      //�۲�����Э�������   
    float *Pk;      //״̬����Э�������
    float *Am;      //״̬ת�ƾ���
    float *Hm;      //�۲�ת�ƾ���
    float *Yk;      //�۲���, �������� x y
    float *Xk;      //״̬������������ x y xv yv

};
#endif


#endif




