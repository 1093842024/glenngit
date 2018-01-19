#ifndef __TRACK_H__
#define __TRACK_H__

//#include "datatype.h"
#include "cv.h"
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


typedef std::vector<Object> ObjectList;

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
	void Missed();

	int GetMissCount() const {return missCount_; }
	const Object& GetObject() const {return obj_;}
	int GetAppearCount() const {return appearCount_;}
	float GetMoveDist() const;
	void SetID(size_t nID){obj_.ID = nID;}
	size_t GetID() const {return obj_.ID;}
	void DrawTraj(IplImage* pFrame) const;
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

////////////////////////////////////////////////////////////////////////////
class MultObjTrackerGNN
{
public:
	MultObjTrackerGNN(int nWidth, int nHeight);
	~MultObjTrackerGNN();

	void Process(const std::vector<CvRect>& observations);

	void DrawObjects(IplImage* pImg) const;

	void SetMinAppearFrame(int nMinAppearFrame);
	void SetMaxPredictFrame(int nMaxAppearFrame);
	void SetMinMoveDist(float minMoveDist);
	size_t GetCurID() const {return nCurID_;}
protected:
	bool IsObjectInScene(const Object& obj) const;

	void cost_matrix(const std::vector<CvRect>& observations);
	void gate_matrix(const std::vector<CvRect>& observations);

	float pos_cost(const Object& obj, const CvRect& observ);
	float veloc_cost(const Object& obj, const CvRect& observ);
	float size_cost(const Object& obj, const CvRect& observ);
	float association_cost(const Object& obj, const CvRect& observ);

	std::vector<KalmanTracker*> tracks_;

	int nWidth_;
	int nHeight_;

	size_t nCurID_;

	int nMinAppearFrame_;
	int nMaxPredictFrame_;
	float minMoveDist_;

	float** costMatrix_;
	int** gateMatrix_;
	float*  u_;
	float*  v_;
	int* rowAssign_;
	int* colAssign_;

	float alphaPos;
	float alphaSize;
	float alphaVeloc;
};

//////////////////////////////////////////////////////////////////////////
class MultObjTrackerNN
{
public:
	MultObjTrackerNN(int nWidth, int nHeight);
	~MultObjTrackerNN();

	void Process(const std::vector<CvRect>& observations);

	void DrawObjects(IplImage* pImg) const;

	void SetMinAppearFrame(int nMinAppearFrame);
	void SetMaxPredictFrame(int nMaxAppearFrame);
	void SetMinMoveDist(float minMoveDist);
	size_t GetCurID() const {return nCurID_;}
protected:
	bool IsObjectInScene(const Object& obj) const;

	void cost_matrix(const std::vector<CvRect>& observations);
	void gate_matrix(const std::vector<CvRect>& observations);

	void association();
	void predict();
	void new_object_validate();

	float pos_cost(const Object& obj, const CvRect& observ);
	float veloc_cost(const Object& obj, const CvRect& observ);
	float size_cost(const Object& obj, const CvRect& observ);
	float association_cost(const Object& obj, const CvRect& observ);

	std::vector<KalmanTracker*> tracks_;

	int nWidth_;
	int nHeight_;

	size_t nCurID_;

	int nMinAppearFrame_;
	int nMaxPredictFrame_;
	float minMoveDist_;

	float** costMatrix_;
	int** gateMatrix_;
	int* rowAssign_;
	int* colAssign_;

	float alphaPos;
	float alphaSize;
	float alphaVeloc;
};

////////////////////////////////////////////////////////////////////////////
class KalmanTracker2
{
public:
	KalmanTracker2();
	virtual ~KalmanTracker2();

	virtual void Init(const Object& obj);
	virtual int Process(const ObjectList& objects);
	CvPoint GetPos() const;
	int GetMissCount() const {return missCount_; }
	const Object& GetObject() const {return obj_;}
private:
	Object obj_; 

	CvKalman* kalman_;
	CvMat *measurement_;
	int frameCount_;
	int missCount_;
};

////////////////////////////////////////////////////////////////////////////
class MulitObjTracker
{
public:
	MulitObjTracker(int nWidth, int nHeight);
	virtual ~MulitObjTracker();

	void Process(const ObjectList& candidateObjects);
	void DrawObjects(IplImage* pImg) const;
	void SetMinAppearFrame(int nMinAppearFrame);
	void SetMaxPredictFrame(int nMaxAppearFrame);
	void SetMinMoveDist(float minMoveDist);
protected:
	bool IsObjectInScene(const Object& obj) const;

	std::vector<KalmanTracker2*> tracks_;
	ObjectList ObjectList_;
	ObjectList NewObjectList_;
	std::vector<std::vector<CvPoint>> trajectorys_;
	std::vector<int> matchVector_;

	int nWidth_;
	int nHeight_;
	int nMinAppearFrame_;
	int nMaxPredictFrame_;
	float minMoveDist_;
};

#endif