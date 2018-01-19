
//#include "stdafx.h"
#include <math.h>
#include <stdlib.h>

/*
��ȫѡ��ԪGauss-Jordan����n��ʵ����A�������A^{-1}
���������
double * a��     ԭ����Ϊһ������
int n��          ����ά��
���������
double * a��     ��õ������
����ֵ��
������ر��Ϊ0����ʾ�������죻���򷵻ط�0ֵ
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
		if ( d+1.0 == 1.0 ) /* ����Ϊ������ */
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
һ��Kalman�˲�����
��nά���Զ�̬ϵͳ��mά���Թ۲�ϵͳ
X_k = A_k,k-1*X_k-1 + W_k-1
Y_k = H_k*X_k + V_k
k = 1,2,...
X_kΪnά״̬������Y_kΪmά�۲�������
A_k,k-1(nxnά)Ϊ״̬ת����H_k(nxmά)Ϊ�۲����
W_kΪnά״̬����������һ�����Ϊ��˹���������Ҿ�ֵΪ0��Э����ΪQ_k
V_kΪmά�۲�����������һ�����Ϊ��˹���������Ҿ�ֵΪ0��Э����ΪR_k

Kalman�˲������������֪k���۲�����Y_0,Y_1,...,Y_k�ͳ�ʼ״̬��������X_0
����������Э������P_0���Լ�Q_k,R_k������£����ƹ��Ƹ���x_k��������
����Э������P_k�����⡣������㹫ʽ���£�
P_k|k-1 = A_k,k-1 * P_k-1 * A_k,k-1^T + Q_k-1
K_k = P_k|k-1 * H_k^T * [H_k * P_k|k-1 * H_k^T + R_k]^{-1}
X_k = A_k,k-1 * X_k-1 + K_k * [Y_k - H_k * A_k,k-1 * X_k-1]
P_k = (I-K_k*H_k)*P_k|k-1
���У�
K_k(nxmά)ΪKalman�������
Q_k(nxnά)ΪW_kЭ������
R_k(mxmά)ΪV_kЭ������
P_k(nxnά)Ϊ�������Э������

һ��Kalman�˲�����������
n��     ���ͱ�������̬ϵͳ��ά����״̬��������
m��     ���ͱ������۲�ϵͳ��ά�����۲���������
A��     ˫����2ά���飬nxn��ϵͳ״̬ת�ƾ���
H��     ˫����2ά���飬mxn���۲����
Q��     ˫����2ά���飬nxn��ģ������W_k��Э�������
R��     ˫����2ά���飬mxm���۲�����V_k��Э�������
y_k��   ˫����2ά���飬mx1���۲���������
x_k��   ˫����2ά���飬nx1��״̬�������������С�����x_k-1������x_k
P_k��   ˫����2ά���飬nxn��������Э������P_k-1������ʱ��Ÿ��µĹ������Э
������P_k
�����  x_k, P_k
��������ֵ��
������0��˵��������������������ʧ�ܣ������ط�0����ʾ��������

Kalman�㷨���Ż���
����۲����Э����Q_k-1�Ͳ������Э����R_k����Ϊ��������
�۲����Э���� P_k|k-1 = A_k,k-1 * P_k-1 * A_k,k-1^T + Q_k-1 ����Ϊ������
������K_kҲ����Ϊ������P�ĸ���P_k = ( I - K_k*H_k ) * P_k|k-1 Ҳ���Ʋ���
�����������P_k|k-1, K_k, P_k���������������
�������û����ô�Ż�����˸�ͨ��һЩ��
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
	/* ״̬���Э�����Ԥ�� P_k|k-1 */
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
		/* ��������� K_k */
	PH = new float[n*m];
	for ( y = 0; y < n; y++ ) /* P_k|k-1*H_k^T */
		for ( x = 0; x < m; x++ )
		{
			temp = 0;
			for ( i = 0; i < n; i++ )
				temp += P_kk_1[y*n+i]*H[x*m+i];
			PH[y*m+x] = temp;
		}
	HPHR = new double[m*m]; /* ��H_k*P_k|k-1*H_k^T+R_k */
	for ( y = 0; y < m; y++ )
		for ( x = 0; x < m; x++ )
		{
			temp = 0;
			for ( i = 0; i < n; i++ )
				temp += H[y*n+i]*PH[i*m+x];
			HPHR[y*m+x] = temp + R[y*m+x];
		}
	invRval = brinv( HPHR, m ); /* ���� */
	if ( invRval == 0 )
	{
		delete [] P;
		delete [] P_kk_1;
		delete [] PH;
		delete [] HPHR;
		return( 0 );
	}
	K_k = new float[n*m]; /* ��K_k */
	for ( y = 0; y < n; y++ )
		for ( x = 0; x < m; x++ )
		{
			temp = 0;
			for ( i = 0; i < m; i++ )
				temp += PH[y*m+i] * (float)HPHR[i*m+x];
			K_k[y*m+x] = temp;
		}
		/* ��״̬�Ĺ��� x_k */
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
	for ( y = 0; y < n; y++ )   /* ��x_k */
	{
		temp = 0;
		for ( i = 0; i < m; i++ )
			temp += K_k[y*m+i]*yHAx[i];
		x_k[y] = Ax[y] + temp;
	}
	/* ��������Э���� P_k */
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