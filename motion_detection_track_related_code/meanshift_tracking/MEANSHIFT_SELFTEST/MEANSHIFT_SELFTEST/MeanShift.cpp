#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "kalman.h"
#include "MeanShift.h"

MeanshiftTracker::MeanshiftTracker(){
	bins = R_BIN * G_BIN * B_BIN;
	Model_Hist = new float[bins];

	int x,y;
	Qk = new float[4*4];
	//Initialize the Kalman Filter
	for ( y = 0; y < 4; y++ )
		for ( x = 0; x < 4; x++ )
			Qk[y*4+x] = 0.0;
	for ( y = 0; y < 4; y++ ) Qk[y*4+y] = 1.0;
	Rk = new float[2*2];
	for ( y = 0; y < 2; y++ )
		for ( x = 0; x < 2; x++ )
			Rk[y*2+x] = 0.0;
	for ( y = 0; y < 2; y++ ) Rk[y*2+y] = 1.0;
	Pk = new float[4*4];
	for ( y = 0; y < 4; y++ )
		for ( x = 0; x < 4; x++ )
			Pk[y*4+x] = 0.0;
	for ( y = 0; y < 4; y++ ) Pk[y*4+y] = 10.0;

	Am = new float[4*4];
	for ( y = 0; y < 4; y++ )
		for ( x = 0; x < 4; x++ )
			Am[y*4+x] = 0.0;
	for ( y = 0; y < 4; y++ ) Am[y*4+y] = 1.0;
	Am[0*4+2] = 1.0;  	
	Am[1*4+3] = 1.0;

	Hm = new float[2*4];
	for ( y = 0; y < 2; y++ )
		for ( x = 0; x < 4; x++ )
			Hm[y*4+x] = 0.0;
	Hm[0*4+0] = 1.0; Hm[1*4+1] = 1.0;

	yk = new float[2];
	xk = new float[4];

	yk[0] = 0.0;
	yk[1] = 0.0;


	xk[0] = 0.0;
	xk[1] = 0.0;
	xk[2] = 0.0;
	xk[3] = 0.0;
}

MeanshiftTracker::~MeanshiftTracker(){
	if ( Model_Hist!=NULL )
		delete [] Model_Hist;

	delete [] Qk;
	delete [] Rk;
	delete [] Pk;
	delete [] Am;
	delete [] Hm;
	delete [] yk;
	delete [] xk;

}

int MeanshiftTracker::CalcuEpanechnikovKernel( int Wx, int Hy, float * Kernel, float & C_k )  //计算epannechnikov核函数
{
	int x, y, xP, yP, i, PixelNo;
	int a2;
	float r2, k;

	xP = Wx * 2 + 1;
	yP = Hy * 2 + 1;
	PixelNo = xP * yP;
	if ( PixelNo <= 0 ) return( 0 );
	for ( i = 0; i < PixelNo; i++ )
		Kernel[i] = 0.0;
	a2 = Wx*Wx+Hy*Hy;
	C_k = 0.0;
	for ( y = 0; y < yP; y++ )
	{
		for ( x = 0; x < xP; x++ )
		{
			r2 = (float)(((y-Hy)*(y-Hy)+(x-Wx)*(x-Wx))*1.0/a2);
			k = 1 - r2;
			C_k = C_k + k;              //所有核函数值的和
			Kernel[y*xP+x] = k;         //根据以上计算公式，越靠近目标区域中心，核函数值越大；
		}                               //根据这个特点，在后面计算颜色直方图的过程中，靠近中心位置像素对颜色直方图的权重影响更大，远离中心的像素对颜色直方图的权重影响更小
	}

	if ( C_k < 0.0000001 ) return( 0 );

	return( PixelNo );
}

void MeanshiftTracker::CalcuColorHistogram( int x0, int y0, int Wx, int Hy, unsigned char * image, int W, int H,
						 float * Kernel, float C_k, float * ColorHist, int bins )
{
    int x_begin, y_begin;
	int y_end, x_end;
	int x, y, i, index, Wk;
	int r, g, b;

	Wk = 2 * Wx + 1;                              //目标区域宽度
	for ( i = 0; i < bins; i++ )                  //颜色直方图初始化为0
		ColorHist[i] = 0.0;
	if ( ( x0 < 0 ) || (x0 >= W) || ( y0 < 0 ) || ( y0 >= H )     //目标区域是否在图像范围内，并且大于一个像素
		|| ( Wx <= 0 ) || ( Hy <= 0 ) ) return;

	x_begin = x0 - Wx;                            //目标区域的起始坐标
	y_begin = y0 - Hy;
	if ( x_begin < 0 ) x_begin = 0;               //起始坐标需要在图像范围内
	if ( y_begin < 0 ) y_begin = 0;
	x_end = x0 + Wx;                              //目标区域的终点坐标
	y_end = y0 + Hy;
	if ( x_end >= W ) x_end = W-1;                //终点坐标需要在图像范围内
	if ( y_end >= H ) y_end = H-1;

	for ( y = y_begin; y <= y_end; y++ )            //对于目标区域的计算
	{
		for ( x = x_begin; x <= x_end; x++ )
		{
			r = image[(y*W+x)*3] >> R_SHIFT; //locate the R_bin    将r通道像素值右移，判断r属于r通道的哪一份
			g = image[(y*W+x)*3+1] >> G_SHIFT; 
			b = image[(y*W+x)*3+2] >> B_SHIFT;
			index = r * G_BIN * B_BIN + g * B_BIN + b;        //计算整个对应整个直方图中的哪个容器
			ColorHist[index] += Kernel[(y-y_begin)*Wk+(x-x_begin)];    //将颜色直方图中对应容器的值增加
		}
	}

	for ( i = 0; i < bins; i++ )
		ColorHist[i] = ColorHist[i]/C_k;                        //最后整个直方图归一化处理 即直方图所有元素和为1，每个直方图的容器中的值，图像中像素属于对应直方图的概率

	return;
}

void MeanshiftTracker::CalcuModelHist( int xt, int yt, int Wx, int Hy, unsigned char * image, int W, int H,
					float * ModelHist, int bins )
{
	float * Kernel, C_k;              //定义核函数kernel的指针、以及所有核函数值的和
	int xp, yp, PixelNo;

	xp = 2 * Wx + 1;
	yp = 2 * Hy + 1;
	PixelNo = xp * yp;
	Kernel = new float [PixelNo];     //按照传入的窗口大小，分配核函数覆盖区域的大小

	PixelNo = CalcuEpanechnikovKernel( Wx, Hy, Kernel, C_k );   //计算核函数中的内容，用于meanshift向量的平滑

	CalcuColorHistogram( xt, yt, Wx, Hy, image, W, H, Kernel, C_k, ModelHist, bins );   //计算颜色直方图，越靠近目标区域中心的像素对于颜色直方图的权重贡献越大（由于核函数存在）

	delete [] Kernel;

	return;
}


void MeanshiftTracker::Initial_MeanShift_tracker( int x0, int y0, int Wx, int Hy,unsigned char * image, int W, int H,
							   float DeltaT )								
{
	CalcuModelHist( x0, y0, Wx, Hy, image, W, H, Model_Hist, bins ); //Calculate target mode 计算目标区域的颜色直方图，
	
	Am[0*4+2] = DeltaT;  	
	Am[1*4+3] = DeltaT;


	yk[0] = (float)x0;
	yk[1] = (float)y0;

	
	xk[0] = (float)x0;
	xk[1] = (float)y0;
	xk[2] = 0.0;
	xk[3] = 0.0;

	deltat = DeltaT;

	return;
}

//Calculate Bhattacaryya相似度
float MeanshiftTracker::CalcuBhattacharyya( float * p, float * q, int bins )
{
	int i;
	float rho;

	rho = 0.0;
	for ( i = 0; i < bins; i++ )
		rho = (float)(rho + sqrt( p[i]*q[i] ));     //理论上只有当当前区域与目标区域完全重叠，这个值最大，为1，否则越小

	return( rho );
}



int MeanshiftTracker::Mean_shift_iteration( int xi, int yi, int Wx, int Hy, unsigned char * image, int W, int H,
						 float * ModelHist, int bins, int & xo, int & yo, float & rho )
{
	int x, y, i, p_idx;
	float * Kernel, * ColorHist, C_k;
	float * w_i, * index_weight, sum_wi;
	int xp, yp, PixelNo;
	int x_begin, x_end, y_begin, y_end;
	int r, g, b, indx;
	float xo_f, yo_f, rho1, err;
	int flag;

	xp = 2 * Wx + 1;
	yp = 2 * Hy + 1;
	PixelNo = xp * yp;
	Kernel = new float [PixelNo]; //Kernel result at each pixel of Box     计算核函数的数据容量大小
	w_i = new float [PixelNo];    //Weight Function                         每个核函数的权重
	index_weight = new float [bins]; //index                                每个直方图容器的权重
	ColorHist = new float[ bins ]; //color Mode                             颜色直方图的数据容量

	//Precalculate Kernel Function
	PixelNo = CalcuEpanechnikovKernel( Wx, Hy, Kernel, C_k );              //首先根据输入的目标区域范围，计算核函数分布的值
	//
	flag = 1;
	for ( i = 0; i < MAX_ITERATE_TIMES; i++ )                              
	{
		if ( flag == 1 )
		{
			CalcuColorHistogram( xi, yi, Wx, Hy, image, W, H, Kernel, C_k, ColorHist, bins ); //计算输入图像的颜色直方图
			//Bhattacharyya similar function
			rho = CalcuBhattacharyya( ColorHist, ModelHist, bins );        //根据新输入区域的颜色直方图与初始目标的颜色直方图进行相似度计算
		}
		else
		{
			rho = rho1;
		}
		for ( x = 0; x < PixelNo; x++ )                //初始化核函数权重为0
			w_i[x] = 0.0;
		for ( x = 0; x < bins; x++ )       //颜色直方图每个容器的权重由根据初始目标直方图中值的大小除以当前直方图中值的大小决定
		{                                  
			index_weight[x] = (float) (ColorHist[x] > 0.0000001 ? sqrt(ModelHist[x]/ColorHist[x]) : 0.0 );
            //Calculate similarity of each bin
		}
        x_begin = xi - Wx;                   //计算确定输入图像的在图像范围内，并且不小于0
		y_begin = yi - Hy;
		x_begin = x_begin < 0 ? 0 : x_begin;
		y_begin = y_begin < 0 ? 0 : y_begin;
		x_end = xi + Wx;
		y_end = yi + Hy;
		x_end = x_end >= W ? x_end = W-1 : x_end;
		y_end = y_end >= H ? y_end = H-1 : y_end;
		sum_wi = 0.0;
		xo_f = 0.0; yo_f = 0.0;
		for ( y = y_begin; y <= y_end; y++ )             //根据权重计算输入目标区域的质心
		{	
			for ( x = x_begin; x <= x_end; x++ )
			{
				r = image[(y*W+x)*3] >> R_SHIFT;
				g = image[(y*W+x)*3+1] >> G_SHIFT; 
				b = image[(y*W+x)*3+2] >> B_SHIFT;
				indx = r * G_BIN * B_BIN + g * B_BIN + b;
				p_idx = (y-y_begin)*xp+(x-x_begin);
				w_i[ p_idx ] = index_weight[indx];
				sum_wi += index_weight[indx];
				xo_f += x * w_i[ p_idx ];
				yo_f += y * w_i[ p_idx ];	
			}
		}

		xo = (int)(xo_f / sum_wi + 0.5 );
		yo = (int)(yo_f / sum_wi + 0.5 );

		CalcuColorHistogram( xo, yo, Wx, Hy, image, W, H, Kernel, C_k, ColorHist, bins );  //计算新的质心条件下的颜色直方图

		rho1 = CalcuBhattacharyya( ColorHist, ModelHist, bins );                //就算新位置下的直方图与初始直方图的匹配度

		if ( rho1 < rho )           //比上一次的匹配度低，则重新计算位置为之前位置与新质心位置的中点
		{
			xo = (xo+xi)/2; 
			yo = (yo+yi)/2;
			flag = 1;
		}
		else                          // 否则，新位置xo yo有效
			flag = 0;

		err = (float)(abs(xo-xi)+abs(yo-yi));     //计算前后两次位置的距离
		if ( err <= 1.0 ) //end situation           如果前后两次位置不变，则认为找到了最高评分的质心
		{
			rho = rho1;
			break;
		}
		else                                      //否则将本次得到的位置作为新的起始位置
		{
			xi = xo; yi = yo;
		}		
	}

	delete [] Kernel;
	delete [] w_i;
	delete [] index_weight;
	delete [] ColorHist;

	return( i+1 );    //返回meanshift迭代的次数
}




float MeanshiftTracker::MeanShift_tracker( int xin, int yin, int Win, int Hin, unsigned char * image, int W, int H,
						int & xout, int & yout, int & Wout, int & Hout ) //此函数中用到kalman滤波估计，属于一步估计；之前的kcf移动跟踪kalman滤波属于两步kalman
{
	int rv;
	int xin1, yin1, Win1, Hin1;
	float rho1, rho3, rho;
	int W1, W3, H1, H3, xo1, yo1, xo3, yo3;


	xin1 = (int)( xin + xk[2] * deltat + 0.5 );     //利用之前kalman计算得到的xk中的x，y速度，修正现在的输入位置x，y
	yin1 = (int)( yin + xk[3] * deltat + 0.5 );
    //xin1=xin;
    //yin1=yin;
	if ( xin1 < 0 ) xin1 = 0;                       //保证修正的位置在图像范围内
	if ( xin1 >= W ) xin1 = W-1;
	if ( yin1 < 0 ) yin1 = 0;
	if ( yin1 >= H ) yin1 = H-1;


	Win1 = Win;                                     //输入的图像宽度没有滤波估计，但后面多尺度meanshift跟踪后有更新，因此也需要保证修正的位置在图像范围内
	Hin1 = Hin;
	if ( Win1 <= 0 ) Win1 = 1;
	if ( Hin1 <= 0 ) Hin1 = 1;
	if ( Win1 >= W/3 ) Win1 = W/3;                  //并且保证目标区域长宽的一半 不超过图像长宽的1/3
	if ( Hin1 >= H/3 ) Hin1 = H/3;


	W1 = (int)( Win1*(1.0 - Delta_h) + 0.5 );        //将输入图像的尺度缩小        
	H1 = (int)( Hin1*(1.0 - Delta_h) + 0.5 );
	rv = Mean_shift_iteration( xin1, yin1, W1, H1, image, W, H, Model_Hist, 
		bins, xo1, yo1, rho1 );	//First location          meanshift迭代，得到最高匹配度评分以及对应的位置

	Wout = Win1;                                     //输入图像的尺度保持不变
	Hout = Hin1; 
	rv = Mean_shift_iteration( xin1, yin1, Wout, Hout, image, W, H, Model_Hist, 
		bins, xout, yout, rho ); //Second location       meanshift迭代，得到最高匹配度评分以及对应的位置

	W3 = (int)( Win1*(1.0 + Delta_h) + 0.5 );        // 将输入图像的尺度扩大  
	H3 = (int)( Hin1*(1.0 + Delta_h) + 0.5 );
	rv = Mean_shift_iteration( xin1, yin1, W3, H3, image, W, H, Model_Hist, 
		bins, xo3, yo3, rho3 );  //Third location         meanshift迭代，得到最高匹配度评分以及对应的位置

	if ( rho < rho1 )                        //寻找最大评分以及对应的目标位置和目标框大小
	{
		rho = rho1;
		xout = xo1; yout = yo1;
		Wout = W1; Hout = H1;
	}
	if ( rho < rho3 )
	{
		rho = rho3;
		xout = xo3; yout = yo3;
		Wout = W3; Hout = H3;
	}

	Wout = (int)( GAMMA * Wout + (1 - GAMMA) * Win + 0.5 ); 
	Hout = (int)( GAMMA * Hout + (1 - GAMMA) * Hin + 0.5 );

    //Predict next center
	yk[0] = (float)xout; yk[1] = (float)yout;          //将最新的位置输入kalman滤波器
	rv = Kalman( 4, 2, Am, Hm, Qk, Rk, yk, xk, Pk );   //更新kalman滤波器，输出新的状态量用于计算x，y坐标的变化速度

	return( rho );
}

/*void MeanshiftTracker::Clear_MeanShift_tracker()
{
	if ( Model_Hist!=NULL )
		delete [] Model_Hist;

      delete [] Qk;
	  delete [] Rk;
	  delete [] Pk;
	  delete [] Am;
	  delete [] Hm;
	  delete [] yk;
	  delete [] xk;
	
}*/


/*
用全选主元Gauss-Jordan法求n阶实矩阵A的逆矩阵A^{-1}
输入参数：
double * a：     原矩阵，为一个方阵
int n：          矩阵维数
输出参数：
double * a：     求得的逆矩阵
返回值：
如果返回标记为0，表示矩阵奇异；否则返回非0值
*/
int brinv( double * a, int n )
{ 
	int * is, * js, i, j, k, l, u, v;
	double d,p;

	is = (int *)malloc( n*sizeof(int) );
	js = (int *)malloc( n*sizeof(int) );
	for ( k = 0; k < n; k++ )
	{ 
		d = 0.0;
		for ( i = k; i < n; i++ )
			for ( j = k; j < n; j++ )
			{ 
				l = i*n+j;
				p = fabs(a[l]);
				if ( p > d ) 
				{ 
					d = p; is[k] = i; js[k] = j;
				}
			}
			if ( d+1.0 == 1.0 ) /* 矩阵为奇异阵 */
			{ 
				free( is ); 
				free( js ); 
				// printf("err**not inv\n");
				return( 0 );
			}
			if ( is[k] != k )
				for ( j = 0; j < n; j++ )
				{ 
					u = k*n+j;
					v = is[k]*n+j;
					p = a[u]; a[u] = a[v]; a[v] = p;
				}
				if ( js[k] != k )
					for ( i = 0; i < n; i++ )
					{ 
						u = i*n+k;
						v = i*n+js[k];
						p = a[u]; a[u] = a[v]; a[v] = p;
					}
					l = k*n+k;
					a[l] = 1.0/a[l];
					for ( j = 0; j < n; j++ )
						if ( j != k )
						{ 
							u = k*n+j;
							a[u] = a[u]*a[l];
						}
						for ( i = 0; i < n; i++ )
							if ( i != k )
								for ( j = 0; j < n; j++ )
									if ( j != k )
									{ 
										u = i*n+j;
										a[u] = a[u] - a[i*n+k]*a[k*n+j];
									}
									for ( i = 0; i < n; i++ )
										if ( i != k )
										{ 
											u = i*n+k;
											a[u] = -a[u]*a[l];
										}
	}
	for ( k = n-1; k >= 0; k-- )
	{ 
		if ( js[k] != k )
			for ( j = 0; j <= n-1; j++ )
			{ 
				u = k*n+j;
				v = js[k]*n+j;
				p = a[u]; a[u] = a[v]; a[v] = p;
			}
			if ( is[k] != k )
				for ( i = 0; i < n; i++ )
				{ 
					u = i*n+k;
					v = i*n+is[k];
					p = a[u]; a[u] = a[v]; a[v] = p;
				}
	}
	free( is );
	free( js );

	return(1);
}


/*
一步Kalman滤波程序
对n维线性动态系统与m维线性观测系统
X_k = A_k,k-1*X_k-1 + W_k-1
Y_k = H_k*X_k + V_k
k = 1,2,...
X_k为n维状态向量，Y_k为m维观测向量。
A_k,k-1(nxn维)为状态转移阵，H_k(nxm维)为观测矩阵
W_k为n维状态噪声向量，一般假设为高斯白噪声，且均值为0，协方差为Q_k
V_k为m维观测噪声向量，一般假设为高斯白噪声，且均值为0，协方差为R_k

Kalman滤波问题就是在已知k个观测向量Y_0,Y_1,...,Y_k和初始状态向量估计X_0
及其估计误差协方差阵P_0，以及Q_k,R_k等情况下，递推估计各个x_k及其噪声
估计协方差阵P_k的问题。具体计算公式如下：
P_k|k-1 = A_k,k-1 * P_k-1 * A_k,k-1^T + Q_k-1
K_k = P_k|k-1 * H_k^T * [H_k * P_k|k-1 * H_k^T + R_k]^{-1}
X_k = A_k,k-1 * X_k-1 + K_k * [Y_k - H_k * A_k,k-1 * X_k-1]
P_k = (I-K_k*H_k)*P_k|k-1
其中：
K_k(nxm维)为Kalman增益矩阵
Q_k(nxn维)为W_k协方差阵
R_k(mxm维)为V_k协方差阵
P_k(nxn维)为估计误差协方差阵

一步Kalman滤波函数参数：
n：     整型变量，动态系统的维数（状态量个数）
m：     整型变量，观测系统的维数（观测量个数）
A：     双精度2维数组，nxn，系统状态转移矩阵
H：     双精度2维数组，mxn，观测矩阵
Q：     双精度2维数组，nxn，模型噪声W_k的协方差矩阵
R：     双精度2维数组，mxm，观测噪声V_k的协方差矩阵
y_k：   双精度2维数组，mx1，观测向量序列
x_k：   双精度2维数组，nx1，状态向量估计量序列。输入x_k-1，返回x_k
P_k：   双精度2维数组，nxn，存放误差协方差阵P_k-1。返回时存放更新的估计误差协
方差阵P_k
输出：  x_k, P_k
函数返回值：
若返回0，说明求增益矩阵过程中求逆失败；若返回非0，表示正常返回

Kalman算法的优化：
如果观测误差协方差Q_k-1和测量误差协方差R_k近似为常数，则
观测误差协方差 P_k|k-1 = A_k,k-1 * P_k-1 * A_k,k-1^T + Q_k-1 近似为常数；
这样，K_k也近似为常数，P的更新P_k = ( I - K_k*H_k ) * P_k|k-1 也近似不变
上面的三个量P_k|k-1, K_k, P_k都可以离线算出！
这个程序没有这么优化，因此更通用一些。
*/

int Kalman( int n, int m, float * A, float * H, float * Q, float * R,
	float * y_k, float * x_k, float * P_k )
{ 
	float * Ax, * PH, * P, * P_kk_1, temp, * K_k; 
	float * yHAx, * KH, * I;
	double * HPHR;
	int x, y, i;
	int invRval;

	P = new float [n*n];
	P_kk_1 = new float [n*n];
	/* 状态误差协方差的预测 P_k|k-1 */
	for ( y = 0; y < n; y++ )  /* A_k,k-1*P_k-1 */
		for ( x = 0; x < n; x++ )
		{
			temp = 0;
			for ( i = 0; i < n; i++ )
				temp += A[y*n+i]*P_k[i*n+x];
			P[y*n+x] = temp;
		}
		for ( y = 0; y < n; y++ )  /* (A_k,k-1*P_k-1)*A_k,k-1^T+Q_k-1 */
			for ( x = 0; x < n; x++ )
			{
				temp = 0;
				for ( i = 0; i < n; i++ )
					temp += P[y*n+i]*A[x*n+i];
				P_kk_1[y*n+x] = temp + Q[y*n+x];
			}
			/* 求增益矩阵 K_k */
			PH = new float[n*m];
			for ( y = 0; y < n; y++ ) /* P_k|k-1*H_k^T */
				for ( x = 0; x < m; x++ )
				{
					temp = 0;
					for ( i = 0; i < n; i++ )
						temp += P_kk_1[y*n+i]*H[x*m+i];
					PH[y*m+x] = temp;
				}
				HPHR = new double[m*m]; /* 求H_k*P_k|k-1*H_k^T+R_k */
				for ( y = 0; y < m; y++ )
					for ( x = 0; x < m; x++ )
					{
						temp = 0;
						for ( i = 0; i < n; i++ )
							temp += H[y*n+i]*PH[i*m+x];
						HPHR[y*m+x] = temp + R[y*m+x];
					}
					invRval = brinv( HPHR, m ); /* 求逆 */
					if ( invRval == 0 )
					{
						delete [] P;
						delete [] P_kk_1;
						delete [] PH;
						delete [] HPHR;
						return( 0 );
					}
					K_k = new float[n*m]; /* 求K_k */
					for ( y = 0; y < n; y++ )
						for ( x = 0; x < m; x++ )
						{
							temp = 0;
							for ( i = 0; i < m; i++ )
								temp += PH[y*m+i] * (float)HPHR[i*m+x];
							K_k[y*m+x] = temp;
						}
						/* 求状态的估计 x_k */
						Ax = new float[n];
						for ( y = 0; y < n; y++ ) /* A_k,k-1 * x_k-1 */
						{
							temp = 0;
							for ( i = 0; i < n; i++ )
								temp += A[y*n+i]*x_k[i];
							Ax[y] = temp;
						}
						yHAx = new float[m];
						for ( y = 0; y < m; y++ ) /* y_k - H_k*A_k,k-1*x_k-1 */
						{
							temp = 0;
							for ( i = 0; i < n; i++ )
								temp += H[y*n+i] * Ax[i];
							yHAx[y] = y_k[y] - temp;
						}
						for ( y = 0; y < n; y++ )   /* 求x_k */
						{
							temp = 0;
							for ( i = 0; i < m; i++ )
								temp += K_k[y*m+i]*yHAx[i];
							x_k[y] = Ax[y] + temp;
						}
						/* 更新误差的协方差 P_k */
						KH = new float [n*n];
						for ( y = 0; y < n; y++ )
							for ( x = 0; x < n; x++ )
							{
								temp = 0;
								for ( i = 0; i < m; i++ )
									temp += K_k[y*m+i]*H[i*n+x];
								KH[y*n+x] = temp;
							}
							I = new float [n*n];
							for ( y = 0; y < n; y++ )
								for ( x = 0; x < n; x++ )
									I[y*n+x] = (float)(x==y ? 1 : 0);
							for ( y = 0; y < n; y++ )   /* I - K_k*H_k */
								for ( x = 0; x < n; x++ )
									I[y*n+x] = I[y*n+x] - KH[y*n+x];
							for ( y = 0; y < n; y++ )  /* P_k = ( I - K_k*H_k ) * P_k|k-1 */
								for ( x = 0; x < n; x++ )
								{
									temp = 0;
									for ( i = 0; i < n; i++ )
										temp += I[y*n+i]*P_kk_1[i*n+x];
									P_k[y*n+x] = temp;
								}

								delete [] P;
								delete [] P_kk_1;
								delete [] PH;
								delete [] HPHR;
								delete [] K_k;
								delete [] Ax;
								delete [] yHAx;
								delete [] KH;
								delete [] I;

								return( 1 );
}
