#ifndef _MEANSHIFT_H
#define _MEANSHIFT_H

# define R_BIN      8  //each bin 8 bin for each color
# define G_BIN      8
# define B_BIN      8

# define R_SHIFT    5  // log2(256/8)  代表把256个值分成8分，每份对应256/8=2^5，即有5位，可通过右移5位获得像素值属于8份中的哪一份
# define G_SHIFT    5
# define B_SHIFT    5


const int MAX_ITERATE_TIMES=20;  //meanshift迭代的最大迭代次数

# define Delta_h     0.1   //meanshift尺度缩放搜索因子
# define GAMMA       0.5   //meanshift尺度变化的动量因子，避免输出的目标框的大小与输入的框的大小变化过快

class MeanshiftTracker{
private:
    float * Model_Hist;
    int bins;
    float * Qk;   //状态噪声协方差矩阵   
    float * Rk;   //观测噪声协方差矩阵 
    float * Pk;   //状态估计协方差矩阵
    float * Am;   //状态转移矩阵
    float * Hm;   //观测转移矩阵
    float * yk;   //观测量, 排列序列 x y
    float * xk;   //状态量，排列序列 x y xv yv
    float deltat; // ??Ƶ????ʱ????????֡?ʵĵ???
public:
	MeanshiftTracker();
	~MeanshiftTracker();

    void Initial_MeanShift_tracker( int x0, int y0, int Wx, int Hy,
                                   unsigned char * image, int W, int H,
                                   float DeltaT );
    
    float MeanShift_tracker( int xin, int yin, int Win, int Hin,
                            unsigned char * image, int W, int H,
                            int & xout, int & yout, int & Wout, int & Hout );
    
    //void Clear_MeanShift_tracker();
    void CalcuColorHistogram( int x0, int y0, int Wx, int Hy,
                             unsigned char * image, int W, int H,
                             float * Kernel, float C_k,
                             float * ColorHist, int bins );
    int CalcuEpanechnikovKernel( int Wx, int Hy, float * Kernel, float & C_k );
    void CalcuModelHist( int xt, int yt, int Wx, int Hy,
                        unsigned char * image, int W, int H,
                        float * ModelHist, int bins );
    int Mean_shift_iteration( int xi, int yi, int Wx, int Hy,
                             unsigned char * image, int W, int H,
                             float * ModelHist, int bins,
                             int & xo, int & yo, float & rho );
    float CalcuBhattacharyya( float * p, float * q, int bins );
};

int Kalman( int n, int m, float * A, float * H, float * Q, float * R,
	float * y_k, float * x_k, float * P_k );


#endif