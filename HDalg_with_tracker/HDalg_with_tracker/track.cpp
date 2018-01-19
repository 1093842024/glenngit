//#include <stdafx.h>
#include "track.h"
#include <math.h>
#include <algorithm>


//#define DEBUG
#ifdef OPENCVKALMAN
CV_IMPL
#else
extern "C"
#endif

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




#ifdef OPENCVKALMAN
/////////////////////////////////////////////////////////////////////////////////////
KalmanTracker::KalmanTracker():
missCount_(0),
appearCount_(0)
{
	/*
	const float A[] = {
		1, 0, 1, 0, 0, 0, 0, 0,
		0, 1, 0, 1, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 1, 0,
		0, 0, 0, 0, 0, 1, 0, 1,
		0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 1,
	};
	const float M[] = {
		1, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0,
	};
	kalman_ = cvCreateKalman(8, 4, 0);                 //状态转移矩阵（状态预测），状态向量维度，测量向量维度
	measurement_ = cvCreateMat(4, 1, CV_32FC1);         //测量矩阵，测量向量个数，测量向量维度
	*/
	const float A[] = {
		1, 0, 1, 0, 
		0, 1, 0, 1, 
		0, 0, 1, 0, 
		0, 0, 0, 1, 
	};
	const float M[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
	};
	kalman_ = cvCreateKalman(4, 2, 0);                 //状态转移矩阵（状态预测），状态向量维度，测量向量维度
	measurement_ = cvCreateMat(2, 1, CV_32FC1);         //测量矩阵，测量向量个数，测量向量维度
	memcpy(kalman_->transition_matrix->data.fl, A, sizeof(A));
	memcpy(kalman_->measurement_matrix->data.fl, M, sizeof(M));
}

KalmanTracker::~KalmanTracker()
{
	cvReleaseKalman(&kalman_);
	cvReleaseMat(&measurement_);
}
/*
void KalmanTracker::Init(const Object& obj)
{
	obj_ = obj;
	obj_.ID = 0;

	kalman_->state_post->data.fl[0] = (float)(obj_.x);
	kalman_->state_post->data.fl[1] = (float)(obj_.y);
	kalman_->state_post->data.fl[2] = 0.f;
	kalman_->state_post->data.fl[3] = 0.f;
	kalman_->state_post->data.fl[4] = (float)(obj_.w);
	kalman_->state_post->data.fl[5] = (float)(obj_.h);
	kalman_->state_post->data.fl[6] = 0.f;
	kalman_->state_post->data.fl[7] = 0.f;
	

	startX_ = obj.x;
	startY_ = obj.y; 
	startW_ = obj.w;
	startH_ = obj.h; 

	appearCount_ = 1;
	missCount_ = 0;

	Point point;
	point.x=obj.x;
	point.y=obj.y;
	traj_.push_back(point);
	point.x=obj.w;
	point.y=obj.h;
	trajwh_.push_back(point);

}  
*/
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

	Point point;
	point.x=obj.x;
	point.y=obj.y;
	traj_.push_back(point);
}  

/*
void KalmanTracker::Predict()
{
	const CvMat* prediction = cvKalmanPredict(kalman_, 0);
#ifdef DEBUG
	printf("origin:x %d y %d w %d h %d \n",obj_.x,obj_.y,obj_.w,obj_.h);
	printf("origin:vx %0.1f vy %0.1f vw %0.1f vh %0.1f \n",obj_.vx,obj_.vy,obj_.vw,obj_.vh);
	printf("predict:x %0.1f y %0.1f w %0.1f h %0.1f \n",prediction->data.fl[0],prediction->data.fl[1],prediction->data.fl[4],prediction->data.fl[5]);
	printf("predict:vx %0.1f vy %0.1f vw %0.1f vh %0.1f \n",prediction->data.fl[2],prediction->data.fl[3],prediction->data.fl[6],prediction->data.fl[7]);
#endif
	obj_.vx = prediction->data.fl[0] - obj_.x;
	obj_.vy = prediction->data.fl[1] - obj_.y;
	obj_.vw =  prediction->data.fl[4] - obj_.w;
	obj_.vh =  prediction->data.fl[5] - obj_.h;

	obj_.x = prediction->data.fl[0];
	obj_.y = prediction->data.fl[1];
	//obj_.w = prediction->data.fl[4];
	//obj_.h = prediction->data.fl[5];
#ifdef DEBUG
	printf("after:x %d y %d w %d h %d \n",obj_.x,obj_.y,obj_.w,obj_.h);
	printf("after:vx %0.1f vy %0.1f vw %0.1f vh %0.1f \n",obj_.vx,obj_.vy,obj_.vw,obj_.vh);
#endif
}
*/

void KalmanTracker::Predict()
{
	const CvMat* prediction = cvKalmanPredict(kalman_, 0);
#ifdef DEBUG
	printf("origin:x %d y %d  \n",obj_.x,obj_.y);
	printf("origin:vx %0.1f vy %0.1f  \n",obj_.vx,obj_.vy);
	printf("predict:x %0.1f y %0.1f  \n",prediction->data.fl[0],prediction->data.fl[1]);
	printf("predict:vx %0.1f vy %0.1f  \n",prediction->data.fl[2],prediction->data.fl[3]);
#endif
	obj_.vx = prediction->data.fl[0] - obj_.x;
	obj_.vy = prediction->data.fl[1] - obj_.y;

	obj_.x = prediction->data.fl[0];
	obj_.y = prediction->data.fl[1];

#ifdef DEBUG
	printf("after:x %d y %d \n",obj_.x,obj_.y);
	printf("after:vx %0.1f vy %0.1f \n",obj_.vx,obj_.vy);
#endif
}

/*
void KalmanTracker::Update(const KLRect& obsv)            ////added by gelin 2017-8-25
{
	missCount_ = 0;
	appearCount_++;

	obj_.x = obsv.x;
	obj_.y = obsv.y;
	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;
	
	obj_.w = obsv.w;
	obj_.h = obsv.h;
	obj_.vw = obj_.w - trajwh_[trajwh_.size() - 1].x;
	obj_.vh = obj_.h - trajwh_[trajwh_.size() - 1].y;
#ifdef DEBUG
	printf("preloc:x-1 %d y-1 %d w-1 %d h-1 %d \n",traj_[traj_.size() - 1].x,traj_[traj_.size() - 1].y,trajwh_[trajwh_.size() - 1].x,trajwh_[trajwh_.size() - 1].y);
    printf("nowloc:x %d y %d w %d h %d \n",obj_.x,obj_.y,obj_.w,obj_.h);
#endif
	measurement_->data.fl[0] = obj_.x;
	measurement_->data.fl[1] = obj_.y;
	measurement_->data.fl[2] = obj_.w;
	measurement_->data.fl[3] = obj_.h;
	cvKalmanCorrect(kalman_, measurement_);

	Point point;
	point.x=obj_.x;
	point.y=obj_.y;
	traj_.push_back(point);
	point.x=obj_.w;
	point.y=obj_.h;
    trajwh_.push_back(point);
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
	if (trajwh_.size() > kMaxTrajSize)
		trajwh_.erase(trajwh_.begin());
}                                                            ////end added
*/

void KalmanTracker::Update(const KLRect& obsv)            ////added by gelin 2017-8-25
{
	missCount_ = 0;
	appearCount_++;

	obj_.x = obsv.x;
	obj_.y = obsv.y;
	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;
	
#ifdef DEBUG
	printf("preloc:x-1 %d y-1 %d \n",traj_[traj_.size() - 1].x,traj_[traj_.size() - 1].y);
    printf("nowloc:x %d y %d \n",obj_.x,obj_.y);
#endif
	measurement_->data.fl[0] = obj_.x;
	measurement_->data.fl[1] = obj_.y;
	cvKalmanCorrect(kalman_, measurement_);

	Point point;
	point.x=obj_.x;
	point.y=obj_.y;
	traj_.push_back(point);
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
}   
                                                         ////end added

void KalmanTracker::Missed()
{
	missCount_++;

	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;

	Point point;
	point.x=obj_.x;
	point.y=obj_.y;
	traj_.push_back(point);
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
}

float KalmanTracker::GetMoveDist() const
{
	float dx = obj_.x - startX_;
	float dy = obj_.y - startY_;

	return sqrt(dx*dx + dy*dy);
}

void KalmanTracker::Update(const CvRect& obsv)
{
	missCount_ = 0;
	appearCount_++;

	obj_.x = obsv.x;
	obj_.y = obsv.y;

	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;

	obj_.w = obsv.width;
	obj_.h = obsv.height;

	measurement_->data.fl[0] = obj_.x;
	measurement_->data.fl[1] = obj_.y;
	measurement_->data.fl[2] = obj_.w;
	measurement_->data.fl[3] = obj_.h;
	cvKalmanCorrect(kalman_, measurement_);

	Point point;
	point.x=obj_.x;
	point.y=obj_.y;
	traj_.push_back(point);
	point.x=obj_.w;
	point.y=obj_.h;
	trajwh_.push_back(point);
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
	if (trajwh_.size() > kMaxTrajSize)
		trajwh_.erase(trajwh_.begin());
}

void KalmanTracker::DrawTraj(IplImage* pFrame) const
{
	for (int i = 0; i < traj_.size() - 1; i++)
	{
		CvPoint point1,point2;
		point1.x=traj_[i].x;
		point1.y=traj_[i].y;
		point2.x=traj_[i].x;
		point2.y=traj_[i].y;
		cvLine(pFrame, point1, point2, CV_RGB(0, 0, 255), 2, 8, 0);
		//cvLine(pFrame, traj_[i], traj_[i + 1], CV_RGB(0, 0, 255), 2, 8, 0);
	}
}
#endif





#ifdef CKALMAN

CKalman::CKalman()
{
	 Qk = new float[4 * 4]; 
	for (int y = 0; y < 4; y++)  
		for (int x = 0; x < 4; x++)  
			Qk[y * 4 + x] = 0.0;  
	for (int y = 0; y < 4; y++)  
		Qk[y * 4 + y] = 1.0;  /* 对角矩阵的协方差都为1.0，且相互独立 */  

	Rk = new float[2 * 2];  // 观测阵协方差  
	for (int y = 0; y < 2; y++)  
		for (int x = 0; x < 2; x++)  
			Rk[y * 2 + x] = 0.0;  
	for (int y = 0; y < 2; y++)  
		Rk[y * 2 + y] = 1.0;  /* 协方差都为1.0，且相互独立 */

	Pk = new float[4 * 4];   // 状态测量误差协方差  
	for (int y = 0; y < 4; y++)  
		for (int x = 0; x < 4; x++)  
			Pk[y * 4 + x] = 0.0;  
	for (int y = 0; y < 4; y++)  
		Pk[y * 4 + y] = 10.0;  /* 初始协方差都为10，且相互独立 */  


	Am = new float[4 * 4];     // 状态转移阵  
	for (int y = 0; y < 4; y++)  
		for (int x = 0; x < 4; x++)  
			Am[y * 4 + x] = 0.0;  
	for (int y = 0; y < 4; y++)  
		Am[y * 4 + y] = 1.0;   /* 对角为1 */  
	Am[0 * 4 + 2] = 1;  
	Am[1 * 4 + 3] = 1;  

	Hm = new float[2 * 4];     // 观测矩阵  
	for (int y = 0; y < 2; y++)  
		for (int x = 0; x < 4; x++)  
			Hm[y * 4 + x] = 0.0;  
	Hm[0 * 4 + 0] = 1.0; Hm[1 * 4 + 1] = 1.0;  


}
CKalman::~CKalman()
{
    delete[] Qk;
	delete[] Rk;
	delete[] Pk;
	delete[] Am;
	delete[] Hm;
	delete[] Yk;
	delete[] Xk;

}

void CKalman::Init(const Object& obj)
{
	obj_=obj;

	Xk = new float[4]; // 状态量，排列顺序：x, y, xv, yv  
	Xk[0] = (float)obj_.x;  
	Xk[1] = (float)obj_.y;  
	Xk[2] = 0.0;  
	Xk[3] = 0.0;
	Yk =new float[2];

	Point point;
	point.x=obj.x;
	point.y=obj.y;
	traj_.push_back(point);
}

void CKalman::Predict(int n,int m)    //定义状态量维度n和观测维度m  这里只能是4和2
{
	float temp, *X_k;
	X_k=new float[n*n];
	for (int y = 0; y < n; y++)   /* 求X_k */  
	{  
		temp = 0;  
		for (int x= 0; x < n; x++)  
			temp += Am[y*n+x]*Xk[x];
		X_k[y] = temp;//n*1  
	}

	for(int i=0; i<n; i++) Xk[i]=X_k[i];   //将求出的X_k更新给Xk
	delete[] X_k;

#ifdef DEBUG
		printf("origin:x %d y %d  \n",obj_.x,obj_.y);
		printf("origin:vx %0.1f vy %0.1f  \n",obj_.vx,obj_.vy);
		printf("predict:x %0.1f y %0.1f  \n",Xk[0],Xk[1]);
		printf("predict:vx %0.1f vy %0.1f  \n",Xk[2],Xk[3]);
#endif		
		obj_.vx = Xk[0] - obj_.x;
		obj_.vy = Xk[1] - obj_.y;
		obj_.x = Xk[0];
		obj_.y = Xk[1];

#ifdef DEBUG
	printf("after:x %d y %d \n",obj_.x,obj_.y);
	printf("after:vx %0.1f vy %0.1f \n",obj_.vx,obj_.vy);
#endif

}

void CKalman::Update(const KLRect& obsv,int n,int m)    //定义状态量维度n和观测维度m  这里只能是4和2
{

	obj_.x = obsv.x;
	obj_.y = obsv.y;
	obj_.vx = obj_.x - traj_[traj_.size() - 1].x;
	obj_.vy = obj_.y - traj_[traj_.size() - 1].y;

#ifdef DEBUG
	printf("preloc:x-1 %d y-1 %d \n",traj_[traj_.size() - 1].x,traj_[traj_.size() - 1].y);
	printf("nowloc:x %d y %d \n",obj_.x,obj_.y);
#endif
	Yk[0] = obj_.x;
	Yk[1] = obj_.y;

	Point point;
	point.x=obj_.x;
	point.y=obj_.y;
	traj_.push_back(point);
	if (traj_.size() > kMaxTrajSize)
		traj_.erase(traj_.begin());
    
	int x, y, i;  
	float temp, *P, *P_kk_1;                       //1.计算状态噪声协方差矩阵的中间变量
	float  *K_k, *PH;   double * HPHR;  int invRval;    //2. 计算kalman增益矩阵的中间变量
	float * Ax,* yHAx ;                                 //3.更新状态量的中间变量
	float *KH, *I;                                      //4.更新状态估计协方差矩阵的中间变量
	
	 

	P = new float[n*n];  
	P_kk_1 = new float[n*n];  
	/* 1.状态误差协方差的预测 P_k|k-1 */  
	for (y = 0; y < n; y++)  /* A_k,k-1*P_k-1 */  
		for (x = 0; x < n; x++)  
		{  
			temp = 0;  
			for (i = 0; i < n; i++)  
				temp += Am[y*n + i] * Pk[i*n + x];   
			P[y*n + x] = temp;  
		}  
	for (y = 0; y < n; y++)  /* (A_k,k-1*P_k-1)*A_k,k-1^T+Q_k-1 */  
		for (x = 0; x < n; x++)  
		{  
			temp = 0;  
			for (i = 0; i < n; i++)  
				temp += P[y*n + i] * Am[x*n + i];//A为状态转移阵的转置  
			P_kk_1[y*n + x] = temp + Qk[y*n + x];  
		}  

	/* 2.求增益矩阵 K_k */  
	PH = new float[n*m];  
	for (y = 0; y < n; y++) /* P_k|k-1*H_k^T */  
		for (x = 0; x < m; x++)  
		{  
			temp = 0;  
			for (i = 0; i < n; i++)  
				temp += P_kk_1[y*n + i] * Hm[x*m + i];      //对Hm的处理方式是相当于乘以Hm的转置
			PH[y*m + x] = temp;//nxm维数  
		}  
	HPHR = new double[m*m]; /* 求H_k*P_k|k-1*H_k^T+R_k */  
	for (y = 0; y < m; y++)  
		for (x = 0; x < m; x++)  
		{  
			temp = 0;  
			for (i = 0; i < n; i++)  
				temp += Hm[y*n + i] * PH[i*m + x];//mxm维数  
			HPHR[y*m + x] = temp + Rk[y*m + x];//mxm维数  
		}  
	invRval = brinv(HPHR, m); /* 求逆 */  
	if (invRval == 0)  
	{  
		delete[] P;  
		delete[] P_kk_1;  
		delete[] PH;  
		delete[] HPHR;  
		return;  
	}  
	K_k = new float[n*m]; /* 求K_k */  
	for (y = 0; y < n; y++)  
	    for (x = 0; x < m; x++)  
		{  
			temp = 0;  
			for (i = 0; i < m; i++)  
				temp += PH[y*m + i] * (float)HPHR[i*m + x];//n*m x m*n  
			K_k[y*m + x] = temp;//n*n  
		}  


    /* 3.求状态的估计 Xk */  
	Ax = new float[n];  
	for (y = 0; y < n; y++) /* A_k,k-1 * x_k-1 */  
	{  
			temp = 0;  
			for (i = 0; i < n; i++)  
				temp += Am[y*n + i] * Xk[i];  
			Ax[y] = temp;//n*1  
		}  

	yHAx = new float[m];  
	for (y = 0; y < m; y++) /* y_k - H_k*(A_k,k-1)*x_k-1 */   
	{  
		temp = 0;  
		for (i = 0; i < n; i++)  
			temp += Hm[y*n + i] * Ax[i];//m*n x n*1=m*1  
		yHAx[y] = Yk[y] - temp;//m*1  
	}  
	for (y = 0; y < n; y++)   /* 求x_k */  
	{  
		temp = 0;  
		for (i = 0; i < m; i++)  
			temp += K_k[y*m + i] * yHAx[i];//n*m x m*1  
		Xk[y] = Ax[y] + temp;//n*1  
	}  

	/* 4.更新误差的协方差 Pk */  
	KH = new float[n*n];  
	for (y = 0; y < n; y++)               //K_k*H_k
		for (x = 0; x < n; x++)  
		{  
			temp = 0;  
			for (i = 0; i < m; i++)  
				temp += K_k[y*m + i] * Hm[i*n + x];//n*m xm*n=n*n  
			KH[y*n + x] = temp;//n*n  
		}  

	I = new float[n*n];  
	for (y = 0; y < n; y++)  
		for (x = 0; x < n; x++)  
			I[y*n + x] = (float)(x == y ? 1 : 0);//对角线为1的n*n矩阵  

	for (y = 0; y < n; y++)   /* I - K_k*H_k */  
		for (x = 0; x < n; x++)  
				I[y*n + x] = I[y*n + x] - KH[y*n + x];//n*n  

	for (y = 0; y < n; y++)  /* P_k = ( I - K_k*H_k ) * P_k|k-1 */  
		for (x = 0; x < n; x++)  
		{  
			temp = 0;  
			for (i = 0; i < n; i++)  
				temp += I[y*n + i] * P_kk_1[i*n + x];//n*n x n*n=n*n  
			Pk[y*n + x] = temp;  
		}  

	delete[] P;  
	delete[] P_kk_1;  
	delete[] PH;  
	delete[] HPHR;  
	delete[] K_k;  
	delete[] Ax;  
	delete[] yHAx;  
	delete[] KH;  
	delete[] I;  

}

int CKalman::brinv(double a[], int n)
{ 
	int *is,*js,i,j,k,l,u,v;
    double d,p;
    is=new int[n];
    js=new int[n];

	for (k=0; k<=n-1; k++)  
	{  
		d=0.0;  
		for (i=k; i<=n-1; i++)  
			for (j=k; j<=n-1; j++)  
			{  
				l=i*n+j; p=fabs(a[l]);  
				if (p>d) { d=p; is[k]=i; js[k]=j;}  
			}  
		if (d+1.0==1.0)  
			{  
				free(is); free(js); printf("err**not inv\n");  
				return(0);  
			}  
		if (is[k]!=k)  
			for (j=0; j<=n-1; j++)  
			{  
				u=k*n+j; v=is[k]*n+j;  
				p=a[u]; a[u]=a[v]; a[v]=p;  
			}  
		if (js[k]!=k)  
			for (i=0; i<=n-1; i++)  
			{  
				u=i*n+k; v=i*n+js[k];  
				p=a[u]; a[u]=a[v]; a[v]=p;  
			}  
		l=k*n+k;  
		a[l]=1.0/a[l];  
		for (j=0; j<=n-1; j++)  
			if (j!=k)  
			{ u=k*n+j; a[u]=a[u]*a[l];}  
		for (i=0; i<=n-1; i++)  
			if (i!=k)  
				for (j=0; j<=n-1; j++)  
	    if (j!=k)  
		{  
			u=i*n+j;  
			a[u] -= a[i*n+k]*a[k*n+j];  
		}  
		for (i=0; i<=n-1; i++)  
			if (i!=k)  
			{  
				u=i*n+k;  
				a[u] = -a[u]*a[l];  
			}  
	} 

	for (k=n-1; k>=0; k--)  
	{  
		if (js[k]!=k)  
			for (j=0; j<=n-1; j++)  
			{  
				u=k*n+j; v=js[k]*n+j;  
				p=a[u]; a[u]=a[v]; a[v]=p;  
			}  
		if (is[k]!=k)  
			for (i=0; i<=n-1; i++)  
			{   
				u=i*n+k; v=i*n+is[k];  
				p=a[u]; a[u]=a[v]; a[v]=p;  
			}  
	}  

   delete[] is; 
   delete[] js;
   return(1);
}


#endif

