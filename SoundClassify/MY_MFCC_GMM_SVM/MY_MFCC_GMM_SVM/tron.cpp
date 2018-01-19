#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "tron.h"
#include "blas.h"
#include "blasp.h"

#ifndef min
template <class T> static inline T min(T x,T y) { return (x<y)?x:y; }
#endif

#ifndef max
template <class T> static inline T max(T x,T y) { return (x>y)?x:y; }
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern double dnrm2_(int *, double *, int *);
extern double ddot_(int *, double *, int *, double *, int *);
extern int daxpy_(int *, double *, double *, int *, double *, int *);
extern int dscal_(int *, double *, double *, int *);

#ifdef __cplusplus
}
#endif

static void default_print(const char *buf)
{
	fputs(buf,stdout);
	fflush(stdout);
}

void TRON::info(const char *fmt,...)
{
	char buf[BUFSIZ];
	va_list ap;
	va_start(ap,fmt);
	vsprintf(buf,fmt,ap);
	va_end(ap);
	(*tron_print_string)(buf);
}

TRON::TRON(const function *fun_obj, double eps, double eps_cg, int max_iter)
{
	this->fun_obj=const_cast<function *>(fun_obj);
	this->eps=eps;
	this->eps_cg=eps_cg;
	this->max_iter=max_iter;
	tron_print_string = default_print;
}

TRON::~TRON()
{
}

void TRON::tron(double *w)                //可信域牛顿法求解的主体，w为返回的权重参数
{
	// Parameters for updating the iterates.  常数eta，用于更新迭代
	double eta0 = 1e-4, eta1 = 0.25, eta2 = 0.75;

	// Parameters for updating the trust region size delta.  常数sigama，用于更新可信域的大小delta
	double sigma1 = 0.25, sigma2 = 0.5, sigma3 = 4;

	int n = fun_obj->get_nr_variable();
	int i, cg_iter;
	double delta, snorm, one=1.0;
	double alpha, f, fnew, prered, actred, gs;
	int search = 1, iter = 1, inc = 1;
	double *s = new double[n];          //牛顿方向，公式中的sk
	double *r = new double[n];          //公式中的r
	double *g = new double[n];          //梯度向量

	// calculate gradient norm at w=0 for stopping condition. 计算梯度向量在w=0时的停止条件
	double *w0 = new double[n];         
	for (i=0; i<n; i++)                 //w初值为0，由于不带约束条件，这是初始可行解
		w0[i] = 0;
	fun_obj->fun(w0);                   //计算初始时的目标函数值
	fun_obj->grad(w0, g);               //计算初始时的梯度值，g是返回的梯度值
	double gnorm0 = dnrm2_(&n, g, &inc);   
	delete [] w0;

	f = fun_obj->fun(w);
	fun_obj->grad(w, g);                  //计算初始条件下的梯度值
	delta = dnrm2_(&n, g, &inc);          //delta
	double gnorm = delta;

	if (gnorm <= eps*gnorm0)              //如果梯度已经接近0，则说明达到最优点，直接返回
		search = 0;

	iter = 1;                             //迭代次数的初始值

	double *w_new = new double[n];        //迭代后w向量的新值
	bool reach_boundary;
	while (iter <= max_iter && search)    //开始迭代，外层循环
	{
		
		cg_iter = trcg(delta, g, s, r, &reach_boundary);//首先用共轭梯度法搜索近似牛顿方向，s为返回的牛顿方向，同时还返回了r，这个向量在后面会用上

		memcpy(w_new, w, sizeof(double)*n);    //将w值拷贝到w_new中
		daxpy_(&n, &one, s, &inc, w_new, &inc);  //计算wk+1=wk+sk，即w_new=one*w_new+s

		gs = ddot_(&n, g, &inc, s, &inc);        //计算（梯度f（wk））T，ddot_是向量内积函数，gs是内积后的结果
		prered = -0.5*(gs-ddot_(&n, s, &inc, r, &inc));  
		fnew = fun_obj->fun(w_new);               //计算新的函数值f(wk+s)

		// Compute the actual reduction.      计算实际减小值
		actred = f - fnew;

		// On the first iteration, adjust the initial step bound.  计算||s||
		snorm = dnrm2_(&n, s, &inc);
		if (iter == 1)
			delta = min(delta, snorm);

		// Compute prediction alpha*snorm of the step.   
		if (fnew - f - gs <= 0)
			alpha = sigma3;
		else
			alpha = max(sigma1, -0.5*(gs/(fnew - f - gs)));

		// Update the trust region bound according to the ratio of actual to predicted reduction. 更新可信域边界，分三种情况
		if (actred < eta0*prered)                                           //pk<n0
			delta = min(max(alpha, sigma1)*snorm, sigma2*delta);
		else if (actred < eta1*prered)                                       //pk属于(n1,n2)
			delta = max(sigma1*delta, min(alpha*snorm, sigma2*delta));
		else if (actred < eta2*prered)                                       //pk<=n1
			delta = max(sigma1*delta, min(alpha*snorm, sigma3*delta));
		else                                                                 //pk>=n2
		{
			if (reach_boundary)
				delta = sigma3*delta;
			else
				delta = max(delta, min(alpha*snorm, sigma3*delta));
		}

		info("iter %2d act %5.3e pre %5.3e delta %5.3e f %5.3e |g| %5.3e CG %3d\n", iter, actred, prered, delta, f, gnorm, cg_iter);

		//更新w的值
		if (actred > eta0*prered)                                         //pk>n0
		{
			iter++;
			//更新w的值
			memcpy(w, w_new, sizeof(double)*n);
			f = fnew;                   //更新函数的值
			fun_obj->grad(w, g);        //重新计算梯度

			gnorm = dnrm2_(&n, g, &inc);
			if (gnorm <= eps*gnorm0)     //梯度模小于指定阈值，迭代结束
				break;
		}
		if (f < -1.0e+32)
		{
			info("WARNING: f < -1.0e+32\n");
			break;
		}
		if (prered <= 0)
		{
			info("WARNING: prered <= 0\n");
			break;
		}
		if (fabs(actred) <= 1.0e-12*fabs(f) &&
		    fabs(prered) <= 1.0e-12*fabs(f))
		{
			info("WARNING: actred and prered too small\n");
			break;
		}
	}

	delete[] g;
	delete[] r;
	delete[] w_new;
	delete[] s;
}

int TRON::trcg(double delta, double *g, double *s, double *r, bool *reach_boundary)  // 实现共轭梯度法搜索近似牛顿方向，g为梯度，s和r是公式中的相应向量
{
	int i, inc = 1;
	int n = fun_obj->get_nr_variable();
	double one = 1;
	double *d = new double[n];
	double *Hd = new double[n];
	double rTr, rnewTrnew, alpha, beta, cgtol;

	//对s，g，d赋初值
	*reach_boundary = false;
	for (i=0; i<n; i++)
	{
		s[i] = 0;
		r[i] = -g[i];
		d[i] = r[i];
	}
	cgtol = eps_cg*dnrm2_(&n, g, &inc);

	int cg_iter = 0;
	rTr = ddot_(&n, r, &inc, r, &inc);
	while (1)
	{
		if (dnrm2_(&n, r, &inc) <= cgtol)
			break;
		cg_iter++;
		fun_obj->Hv(d, Hd);

		alpha = rTr/ddot_(&n, d, &inc, Hd, &inc);
		daxpy_(&n, &alpha, d, &inc, s, &inc);
		if (dnrm2_(&n, s, &inc) > delta)
		{
			//求解一元二次方程
			info("cg reaches trust region boundary\n");
			*reach_boundary = true;
			alpha = -alpha;
			daxpy_(&n, &alpha, d, &inc, s, &inc);

			double std = ddot_(&n, s, &inc, d, &inc);
			double sts = ddot_(&n, s, &inc, s, &inc);
			double dtd = ddot_(&n, d, &inc, d, &inc);
			double dsq = delta*delta;
			double rad = sqrt(std*std + dtd*(dsq-sts));
			if (std >= 0)
				alpha = (dsq - sts)/(std + rad);
			else
				alpha = (rad - std)/dtd;
			daxpy_(&n, &alpha, d, &inc, s, &inc);
			alpha = -alpha;
			daxpy_(&n, &alpha, Hd, &inc, r, &inc);
			break;
		}
		alpha = -alpha;
		daxpy_(&n, &alpha, Hd, &inc, r, &inc);
		rnewTrnew = ddot_(&n, r, &inc, r, &inc);
		beta = rnewTrnew/rTr;
		dscal_(&n, &beta, d, &inc);
		daxpy_(&n, &one, r, &inc, d, &inc);
		rTr = rnewTrnew;
	}

	delete[] d;
	delete[] Hd;

	return(cg_iter);
}

double TRON::norm_inf(int n, double *x)
{
	double dmax = fabs(x[0]);
	for (int i=1; i<n; i++)
		if (fabs(x[i]) >= dmax)
			dmax = fabs(x[i]);
	return(dmax);
}

void TRON::set_print_string(void (*print_string) (const char *buf))
{
	tron_print_string = print_string;
}
