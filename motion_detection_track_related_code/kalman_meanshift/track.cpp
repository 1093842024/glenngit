#include "stdafx.h"
#include "track.h"
#include "utility.h"
#include "lap.h"
#include <math.h>
#include <algorithm>


#ifndef MAX
#define MAX(x, y)					((x) > (y) ? (x) : (y))
#endif  

#ifndef MIN
#define MIN(x, y)					((x) < (y) ? (x) : (y))
#endif

const int kMinAppearNumber =		5;
const int kMaxPredictNumber =		7;
const float kMinMoveDist =			25.0f;
const float kSpeedScale =           0.4f;

const float kVelocScale =           0.0f;

const float kSizeScale =            0.35f;

const int kMarginXLeft =			5;
const int kMarginYTop =				5;
const int kMarginXRight =			5;
const int kMarginYBottom =			5;

const int kMaxStoredTrajPts =		150;

const int kBinBit =                 5;
const int kByteShift =              3;
const int kDim =                    3;

const float kMaxCost =              1000;

const int kMaxObjects =             256;

const int kMaxTrajSize =			20;

/////////////////////////////////////////////////////////////////////////////////////
KalmanTracker::KalmanTracker():
missCount_(0),
appearCount_(0)
{
	const float A[] = {
		1, 0, 1, 0,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	const float M[] = {
		1, 0, 0, 0,
		0, 1, 0, 0
	};
	kalman_ = cvCreateKalman(4, 2, 0);
	measurement_ = cvCreateMat(2, 1, CV_32FC1);
	memcpy(kalman_->transition_matrix->data.fl, A, sizeof(A));
	memcpy(kalman_->measurement_matrix->data.fl, M, sizeof(M));
}

KalmanTracker::~KalmanTracker()
{
	cvReleaseKalman(&kalman_);
	cvReleaseMat(&measurement_);
}

void KalmanTracker::Init(const Object& obj)
{
	obj_ = obj;
	obj_.ID = 0;

	kalman_->state_post->data.fl[0] = (float)(obj_.x);
	kalman_->state_post->data.fl[1] = (float)(obj_.y);
	kalman_->state_post->data.fl[2] = 0.f;
	kalman_->state_post->data.fl[3] = 0.f;

	startX_ = obj.x;
	startY_ = obj.y; 

	appearCount_ = 1;
	missCount_ = 0;
	traj_.push_back(cvPoint(obj.x, obj.y));
}

void KalmanTracker::Predict()
{
	const CvMat* prediction = cvKalmanPredict(kalman_, 0);

	obj_.vx = prediction->data.fl[0] - obj_.x;
	obj_.vy = prediction->data.fl[1] - obj_.y;
	
	//obj_.x = prediction->data.fl[0];
	//obj_.y = prediction->data.fl[1];
}

void KalmanTracker::Update(const CvRect& obsv)
{
	missCount_ = 0;
	appearCount_++;
	
	obj_.x = obsv.x + obsv.width/2;
	obj_.y = obsv.y + obsv.height/2;
	
	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;
	
	obj_.w = obsv.width;
	obj_.h = obsv.height;

	measurement_->data.fl[0] = obj_.x;
	measurement_->data.fl[1] = obj_.y;
	cvKalmanCorrect(kalman_, measurement_);

	traj_.push_back(cvPoint(obj_.x, obj_.y));
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
}

void KalmanTracker::Missed()
{
	missCount_++;

	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;

	traj_.push_back(cvPoint(obj_.x, obj_.y));
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
}

float KalmanTracker::GetMoveDist() const
{
	float dx = obj_.x - startX_;
	float dy = obj_.y - startY_;

	return sqrt(dx*dx + dy*dy);
}

void KalmanTracker::DrawTraj(IplImage* pFrame) const
{
	for (int i = 0; i < traj_.size() - 1; i++)
		cvLine(pFrame, traj_[i], traj_[i + 1], CV_RGB(0, 0, 255), 2, 8, 0);
}

/////////////////////////////////////////////////////////////////////////////////////
MultObjTrackerGNN::MultObjTrackerGNN(int nWidth, int nHeight):
nWidth_(nWidth),
nHeight_(nHeight),
nMinAppearFrame_(kMinAppearNumber),
nMaxPredictFrame_(kMaxPredictNumber),
minMoveDist_(kMinMoveDist),
nCurID_(0),
alphaPos(0.4),
alphaSize(0.3),
alphaVeloc(0.3)
{
	costMatrix_ = new float*[kMaxObjects];
	for (int i = 0; i < kMaxObjects; i++)
		costMatrix_[i] = new float[kMaxObjects];

	gateMatrix_ = new int*[kMaxObjects];
	for (int i = 0; i < kMaxObjects; i++)
		gateMatrix_[i] = new int[kMaxObjects];

	u_ = new float[kMaxObjects];
	v_ = new float[kMaxObjects];

	rowAssign_ = new int[kMaxObjects];
	colAssign_ = new int[kMaxObjects];
}

MultObjTrackerGNN::~MultObjTrackerGNN()
{
	for (int i = 0; i < tracks_.size(); i++)
		delete tracks_[i];

	for (int i = 0; i < kMaxObjects; i++)
		delete [](costMatrix_[i]);
	delete []costMatrix_;

	for (int i = 0; i < kMaxObjects; i++)
		delete [](gateMatrix_[i]);
	delete []gateMatrix_;

	delete []u_;
	delete []v_;
	delete []rowAssign_;
	delete []colAssign_;
}

void MultObjTrackerGNN::cost_matrix(const std::vector<CvRect>& observations)
{
	for (int i = 0; i < kMaxObjects; i++)
		for (int j = 0; j < kMaxObjects; j++)
			costMatrix_[i][j] = kMaxCost;

	float dx, dy;
	for (int i = 0; i < tracks_.size(); i++)
		for (int j = 0; j < observations.size(); j++)
		{
			const Object& obj = tracks_[i]->GetObject();
			dx = obj.x - (observations[j].x + observations[j].width/2);
			dy = obj.y - (observations[j].y + observations[j].height/2);

			costMatrix_[i][j] = sqrt(dx*dx + dy*dy) + 0.01f;
		}
}

void MultObjTrackerGNN::gate_matrix(const std::vector<CvRect>& observations)
{
	for (int i = 0; i < tracks_.size(); i++)
		for (int j = 0; j < observations.size(); j++)
		{
			const Object& obj = tracks_[i]->GetObject();

			float distThreshX = 0.5*kSpeedScale*(obj.w + observations[j].width);
			float distThreshY = 0.5*kSpeedScale*(obj.h + observations[j].height);

			//float velocCost = veloc_cost(obj, observations[j]);

			if (costMatrix_[i][j] <= distThreshX && costMatrix_[i][j] <= distThreshY /*&&
				velocCost >= 0*/)
				gateMatrix_[i][j] = 1;
			else
				gateMatrix_[i][j] = 0;
		}
}

void MultObjTrackerGNN::Process(const std::vector<CvRect>& observations)
{
	for (int i = 0; i < tracks_.size(); i++)
		tracks_[i]->Predict();

	if (observations.size() == 0)
	{
		for (int i = 0; i < tracks_.size(); i++)
			tracks_[i]->Missed();

		for (int i = tracks_.size() - 1; i >= 0; i--)
			if (tracks_[i]->GetMissCount() > nMaxPredictFrame_)
			{
				delete tracks_[i];
				tracks_.erase(tracks_.begin() + i);
			}
		return;
	}
	else if (tracks_.size() == 0)
	{
		for (int i = 0; i < observations.size(); i++)
		{
			KalmanTracker* pTracker = new KalmanTracker();

			Object temp;
			temp.x = observations[i].x + observations[i].width/2;
			temp.y = observations[i].y + observations[i].height/2;
			temp.w = observations[i].width;
			temp.h = observations[i].height;

			pTracker->Init(temp);
			tracks_.push_back(pTracker);
		}

		return;
	}

	cost_matrix(observations);
	gate_matrix(observations);

	int nSize = 2*MAX(tracks_.size(), observations.size());
	lap(nSize, costMatrix_, rowAssign_, colAssign_, u_, v_);

	std::vector<KalmanTracker*> newTracks;
	for (int i = 0; i < observations.size(); i++)
	{
		int j = colAssign_[i];
		if (j < tracks_.size() && gateMatrix_[j][i] == 1)
			continue;

		KalmanTracker* pTracker = new KalmanTracker();

		Object temp;
		temp.x = observations[i].x + observations[i].width/2;
		temp.y = observations[i].y + observations[i].height/2;
		temp.w = observations[i].width;
		temp.h = observations[i].height;
		
		pTracker->Init(temp);
		newTracks.push_back(pTracker);
	}

	for (int i = tracks_.size() - 1; i >= 0; i--)
	{
		int j = rowAssign_[i];

		if (j < observations.size() && gateMatrix_[i][j] == 1)
			tracks_[i]->Update(observations[j]);
		else
			tracks_[i]->Missed();

		if (tracks_[i]->GetMissCount() > nMaxPredictFrame_)
		{
			delete tracks_[i];
			tracks_.erase(tracks_.begin() + i);
		}
	}

	for (int i = 0; i < newTracks.size(); i++)
		tracks_.push_back(newTracks[i]);

	for (int i = 0; i < tracks_.size(); i++)
		if (tracks_[i]->GetAppearCount() >= nMinAppearFrame_ && tracks_[i]->GetID() == 0 &&
			tracks_[i]->GetMoveDist() >= minMoveDist_)
		{
			nCurID_++;
			tracks_[i]->SetID(nCurID_);
		}
}

void MultObjTrackerGNN::DrawObjects(IplImage* pImg) const
{
	CvFont font = cvFont(1, 1);
	char szText[64];
	for (int i = 0; i < tracks_.size(); i++)
	{
		if (tracks_[i]->GetAppearCount() < nMinAppearFrame_)
			continue;

		if (tracks_[i]->GetMoveDist() < minMoveDist_)
			continue;

		DrawObejct(pImg, &(tracks_[i]->GetObject()), 0, 0, 255);

		//sprintf(szText, "%d", tracks_[i]->GetID());
		//sprintf(szText, "%.3f,%.3f", tracks_[i]->GetObject().vx, tracks_[i]->GetObject().vy);
		//cvPutText(pImg, szText, cvPoint(tracks_[i]->GetObject().x + 5, tracks_[i]->GetObject().y + 5), 
		//	&font, CV_RGB(255, 0, 0));
		tracks_[i]->DrawTraj(pImg);
	}
}

bool MultObjTrackerGNN::IsObjectInScene(const Object& obj) const
{
	if (obj.x <= kMarginXLeft || obj.x >= nWidth_ - kMarginXRight ||
		obj.y <= kMarginYTop || obj.y >= nHeight_ - kMarginYBottom)
		return false;
	else
		return true;
}

void MultObjTrackerGNN::SetMinAppearFrame(int nMinAppearFrame)
{
	nMinAppearFrame_ = nMinAppearFrame;
}
	
void MultObjTrackerGNN::SetMaxPredictFrame(int nMaxAppearFrame)
{
	nMaxPredictFrame_ = nMaxAppearFrame;
}

void MultObjTrackerGNN::SetMinMoveDist(float minMoveDist)
{
	minMoveDist_ = minMoveDist;
}

float MultObjTrackerGNN::pos_cost(const Object& obj, const CvRect& observ)
{
	float dx = obj.x - (observ.x + observ.width/2);
	float dy = obj.y - (observ.y + observ.height/2);

	return sqrt(dx*dx + dy*dy);
}

float MultObjTrackerGNN::veloc_cost(const Object& obj, const CvRect& observ)
{
	//float dvx = obj.vx - (observ.x + observ.width/2 - obj.x);
	//float dvy = obj.vy - (observ.y + observ.height/2 - obj.y);

	//return sqrt(dvx*dvx + dvy*dvy);
	
	float vx = observ.x + observ.width/2 - obj.x;
	float vy = observ.y + observ.height/2 - obj.y;

	float nv = vx*vx + vy*vy;
	float nvp = obj.vx*obj.vx + obj.vy*obj.vy;

	if (nv == 0 || nvp == 0)
		return 0;

	return (vx*obj.vx + vy*obj.vy)/(sqrt(nv)*sqrt(nvp));
}

float MultObjTrackerGNN::size_cost(const Object& obj, const CvRect& observ)
{
	float dw = obj.w - observ.width;
	float dh = obj.h - observ.height;

	return sqrt(dw*dw + dh*dh);
}

float MultObjTrackerGNN::association_cost(const Object& obj, const CvRect& observ)
{
	return alphaPos*pos_cost(obj, observ) + alphaVeloc* veloc_cost(obj, observ) +
		alphaSize*size_cost(obj, observ);
}

/////////////////////////////////////////////////////////////////////////////////////
MultObjTrackerNN::MultObjTrackerNN(int nWidth, int nHeight):
nWidth_(nWidth),
nHeight_(nHeight),
nMinAppearFrame_(kMinAppearNumber),
nMaxPredictFrame_(kMaxPredictNumber),
minMoveDist_(kMinMoveDist),
nCurID_(0),
alphaPos(0.4),
alphaSize(0.3),
alphaVeloc(0.3)
{
	costMatrix_ = new float*[kMaxObjects];
	for (int i = 0; i < kMaxObjects; i++)
		costMatrix_[i] = new float[kMaxObjects];

	gateMatrix_ = new int*[kMaxObjects];
	for (int i = 0; i < kMaxObjects; i++)
		gateMatrix_[i] = new int[kMaxObjects];

	rowAssign_ = new int[kMaxObjects];
	colAssign_ = new int[kMaxObjects];
}

MultObjTrackerNN::~MultObjTrackerNN()
{
	for (int i = 0; i < tracks_.size(); i++)
		delete tracks_[i];

	for (int i = 0; i < kMaxObjects; i++)
		delete [](costMatrix_[i]);
	delete []costMatrix_;

	for (int i = 0; i < kMaxObjects; i++)
		delete [](gateMatrix_[i]);
	delete []gateMatrix_;

	delete []rowAssign_;
	delete []colAssign_;
}

void MultObjTrackerNN::cost_matrix(const std::vector<CvRect>& observations)
{
	for (int i = 0; i < kMaxObjects; i++)
		for (int j = 0; j < kMaxObjects; j++)
			costMatrix_[i][j] = kMaxCost;

	float dx, dy;
	for (int i = 0; i < tracks_.size(); i++)
		for (int j = 0; j < observations.size(); j++)
		{
			const Object& obj = tracks_[i]->GetObject();
			dx = obj.x - (observations[j].x + observations[j].width/2);
			dy = obj.y - (observations[j].y + observations[j].height/2);

			costMatrix_[i][j] = sqrt(dx*dx + dy*dy) + 0.01f;
		}
}

void MultObjTrackerNN::gate_matrix(const std::vector<CvRect>& observations)
{
	for (int i = 0; i < tracks_.size(); i++)
		for (int j = 0; j < observations.size(); j++)
		{
			const Object& obj = tracks_[i]->GetObject();

			float distThreshX = 0.5*kSpeedScale*(obj.w + observations[j].width);
			float distThreshY = 0.5*kSpeedScale*(obj.h + observations[j].height);

			if (costMatrix_[i][j] <= distThreshX && costMatrix_[i][j] <= distThreshY)
				gateMatrix_[i][j] = 1;
			else
				gateMatrix_[i][j] = 0;
		}
}

void MultObjTrackerNN::Process(const std::vector<CvRect>& observations)
{
	predict();

	if (observations.size() == 0)
	{
		for (int i = 0; i < tracks_.size(); i++)
			tracks_[i]->Missed();

		for (int i = tracks_.size() - 1; i >= 0; i--)
			if (tracks_[i]->GetMissCount() > nMaxPredictFrame_)
			{
				delete tracks_[i];
				tracks_.erase(tracks_.begin() + i);
			}
	}
	else if (tracks_.size() == 0)
	{
		for (int i = 0; i < observations.size(); i++)
		{
			KalmanTracker* pTracker = new KalmanTracker();

			Object temp;
			temp.x = observations[i].x + observations[i].width/2;
			temp.y = observations[i].y + observations[i].height/2;
			temp.w = observations[i].width;
			temp.h = observations[i].height;
			temp.vx = 0.f;
			temp.vy = 0.f;

			pTracker->Init(temp);
			tracks_.push_back(pTracker);
		}
	}
	else
	{
		association();
	}

	new_object_validate();
}

void MultObjTrackerNN::DrawObjects(IplImage* pImg) const
{
	CvFont font = cvFont(1, 1);
	char szText[64];
	for (int i = 0; i < tracks_.size(); i++)
	{
		if (tracks_[i]->GetAppearCount() < nMinAppearFrame_)
			continue;

		if (tracks_[i]->GetMoveDist() < minMoveDist_)
			continue;

		DrawObejct(pImg, &(tracks_[i]->GetObject()), 0, 0, 255);

		sprintf(szText, "%d", tracks_[i]->GetID());
		cvPutText(pImg, szText, cvPoint(tracks_[i]->GetObject().x + 5, tracks_[i]->GetObject().y + 5), 
			&font, CV_RGB(255, 0, 0));

		tracks_[i]->DrawTraj(pImg);
	}
}

bool MultObjTrackerNN::IsObjectInScene(const Object& obj) const
{
	if (obj.x <= kMarginXLeft || obj.x >= nWidth_ - kMarginXRight ||
		obj.y <= kMarginYTop || obj.y >= nHeight_ - kMarginYBottom)
		return false;
	else
		return true;
}

void MultObjTrackerNN::SetMinAppearFrame(int nMinAppearFrame)
{
	nMinAppearFrame_ = nMinAppearFrame;
}
	
void MultObjTrackerNN::SetMaxPredictFrame(int nMaxAppearFrame)
{
	nMaxPredictFrame_ = nMaxAppearFrame;
}

void MultObjTrackerNN::SetMinMoveDist(float minMoveDist)
{
	minMoveDist_ = minMoveDist;
}

void MultObjTrackerNN::predict()
{
	for (int i = 0; i < tracks_.size(); i++)
		tracks_[i]->Predict();
}

void MultObjTrackerNN::association()
{

}

void MultObjTrackerNN::new_object_validate()
{
	for (int i = 0; i < tracks_.size(); i++)
		if (tracks_[i]->GetAppearCount() >= nMinAppearFrame_ && tracks_[i]->GetID() == 0 &&
			tracks_[i]->GetMoveDist() >= minMoveDist_)
		{
			nCurID_++;
			tracks_[i]->SetID(nCurID_);
		}
}

float MultObjTrackerNN::pos_cost(const Object& obj, const CvRect& observ)
{
	float dx = obj.x - (observ.x + observ.width/2);
	float dy = obj.y - (observ.y + observ.height/2);

	return sqrt(dx*dx + dy*dy);
}

float MultObjTrackerNN::veloc_cost(const Object& obj, const CvRect& observ)
{
	float dvx = obj.vx - (observ.x + observ.width/2 - obj.x);
	float dvy = obj.vy - (observ.y + observ.height/2 - obj.y);

	return sqrt(dvx*dvx + dvy*dvy);
}

float MultObjTrackerNN::size_cost(const Object& obj, const CvRect& observ)
{
	float dw = obj.w - observ.width;
	float dh = obj.h - observ.height;

	return sqrt(dw*dw + dh*dh);
}

float MultObjTrackerNN::association_cost(const Object& obj, const CvRect& observ)
{
	return alphaPos*pos_cost(obj, observ) + alphaVeloc* veloc_cost(obj, observ) +
		alphaSize*size_cost(obj, observ);
}

////////////////////////////////////////////////////////////////////////////////
KalmanTracker2::KalmanTracker2():
frameCount_(0),
missCount_(0)
{
	const float A[] = {
		1, 0, 1, 0,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	const float M[] = {
		1, 0, 0, 0,
		0, 1, 0, 0
	};
	kalman_ = cvCreateKalman(4, 2, 0);
	measurement_ = cvCreateMat(2, 1, CV_32FC1);
	memcpy(kalman_->transition_matrix->data.fl, A, sizeof(A));
	memcpy(kalman_->measurement_matrix->data.fl, M, sizeof(M));
}

KalmanTracker2::~KalmanTracker2()
{
	cvReleaseKalman(&kalman_);
	cvReleaseMat(&measurement_);
}

void KalmanTracker2::Init(const Object& obj)
{
	obj_ = obj;

	kalman_->state_post->data.fl[0] = (float)(obj_.x);
	kalman_->state_post->data.fl[1] = (float)(obj_.y);
	kalman_->state_post->data.fl[2] = 0;
	kalman_->state_post->data.fl[3] = 0;
}

int KalmanTracker2::Process(const ObjectList& objects)
{
	frameCount_++;

	{
		const CvMat* prediction = cvKalmanPredict(kalman_, 0);
		obj_.x = prediction->data.fl[0];
		obj_.y = prediction->data.fl[1];
	}

	int bestIndex = -1;
	float minDist;

	for (int i = 0; i < objects.size(); i++)
	{
		Object candidate = objects[i];

		float scaleW = fabs(obj_.w - candidate.w)/(float)MAX(obj_.w, candidate.w);
		float scaleH = fabs(obj_.h - candidate.h)/(float)MAX(obj_.h, candidate.h);

		if (scaleH >= 0.25 || scaleW >= 0.25)
			continue;

		float dx = obj_.x - candidate.x;
		float dy = obj_.y - candidate.y;
		float dist = sqrt(dx*dx + dy*dy);

		if (dist <= 0.40*(obj_.w + obj_.h))
		{
			if (bestIndex == -1 || dist < minDist)
			{
				bestIndex = i;
				minDist = dist;
			}
		}
	}

	if (bestIndex == -1)
	{
		missCount_ ++;	
	}
	else
	{
		missCount_ = 0;
		obj_.x = objects[bestIndex].x;
		obj_.y = objects[bestIndex].y;
		obj_.w = objects[bestIndex].w;
		obj_.h = objects[bestIndex].h;

		measurement_->data.fl[0] = obj_.x;
		measurement_->data.fl[1] = obj_.y;
		cvKalmanCorrect(kalman_, measurement_);
	}

	return bestIndex;
}

CvPoint KalmanTracker2::GetPos() const
{
	CvPoint temp;
	temp.x = obj_.x;
	temp.y = obj_.y;

	return temp;
}

////////////////////////////////////////////////////////////////////////////////
MulitObjTracker::MulitObjTracker(int nWidth, int nHeight):
nWidth_(nWidth),
nHeight_(nHeight),
nMinAppearFrame_(kMinAppearNumber),
nMaxPredictFrame_(kMaxPredictNumber),
minMoveDist_(kMinMoveDist)
{
}

MulitObjTracker::~MulitObjTracker()
{
	for (int i = 0; i < tracks_.size(); i++)
		delete tracks_[i];
}

void MulitObjTracker::Process(const ObjectList& candidateObjects)
{
	matchVector_.clear();

	for (int i = 0; i < tracks_.size(); i++)
	{
		int bestMatch = tracks_[i]->Process(candidateObjects);

		CvPoint pt = tracks_[i]->GetPos();
		trajectorys_[i].push_back(pt);

		if (trajectorys_[i].size() > kMaxStoredTrajPts)
			trajectorys_[i].erase(trajectorys_[i].begin(), trajectorys_[i].begin() + 
			trajectorys_[i].size() - kMaxStoredTrajPts);

		ObjectList_[i] = tracks_[i]->GetObject();

		if (bestMatch != -1)
			matchVector_.push_back(bestMatch);
	}

	NewObjectList_.clear();

	for (int i = 0; i < candidateObjects.size(); i++)
	{
		if (std::find(matchVector_.begin(), matchVector_.end(), i) == matchVector_.end())
			NewObjectList_.push_back(candidateObjects[i]);
	}

	for (int i = 0; i < NewObjectList_.size(); i++)
	{
		KalmanTracker2* pTracker = new KalmanTracker2();

		pTracker->Init(NewObjectList_[i]);
		tracks_.push_back(pTracker);

		ObjectList_.push_back(NewObjectList_[i]);

		std::vector<CvPoint> trajectory;
		trajectorys_.push_back(trajectory);

		CvPoint pt;
		pt.x = (int)NewObjectList_[i].x;
		pt.y = (int)NewObjectList_[i].y;

		trajectorys_[trajectorys_.size() - 1].push_back(pt);
	}

	for (int i = ObjectList_.size() - 1; i >= 0; i--)
	{
		if (!IsObjectInScene(ObjectList_[i]) || tracks_[i]->GetMissCount() >= nMaxPredictFrame_)
		{
			ObjectList_.erase(ObjectList_.begin() + i);
			delete tracks_[i];
			tracks_.erase(tracks_.begin() + i);
			trajectorys_.erase(trajectorys_.begin() + i);
		}
	}
}

void MulitObjTracker::DrawObjects(IplImage* pImg) const
{
	for (int i = 0; i < tracks_.size(); i++)
	{
		if (trajectorys_[i].size() < nMinAppearFrame_)
			continue;

		float dx = trajectorys_[i][0].x - trajectorys_[i][trajectorys_[i].size() - 1].x;
		float dy = trajectorys_[i][0].y - trajectorys_[i][trajectorys_[i].size() - 1].y;

		if (sqrt(dx*dx + dy*dy) <= minMoveDist_)
			continue;

		DrawObejct(pImg, &(ObjectList_[i]), 0, 0, 255);

		for (int j = 0; j < trajectorys_[i].size() - 1; j++)
			cvLine(pImg, trajectorys_[i][j], trajectorys_[i][j + 1], CV_RGB(255, 0, 0), 2, 8, 0);
	}
}

bool MulitObjTracker::IsObjectInScene(const Object& obj) const
{
	if (obj.x <= kMarginXLeft || obj.x >= nWidth_ - kMarginXRight ||
		obj.y <= kMarginYTop || obj.y >= nHeight_ - kMarginYBottom)
		return false;
	else
		return true;
}

void MulitObjTracker::SetMinAppearFrame(int nMinAppearFrame)
{
	nMinAppearFrame_ = nMinAppearFrame;
}

void MulitObjTracker::SetMaxPredictFrame(int nMaxAppearFrame)
{
	nMaxPredictFrame_ = nMaxAppearFrame;
}

void MulitObjTracker::SetMinMoveDist(float minMoveDist)
{
	minMoveDist_ = minMoveDist;
}