//meanshift.h

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
    void Initial_MeanShift_tracker( int x0, int y0, int Wx, int Hy,
                                   unsigned char * image, int W, int H,
                                   float DeltaT );
    
    float MeanShift_tracker( int xin, int yin, int Win, int Hin,
                            unsigned char * image, int W, int H,
                            int & xout, int & yout, int & Wout, int & Hout );
    
    void Clear_MeanShift_tracker();
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

