#ifndef _TRON_H
#define _TRON_H

class function                             //是损失函数（目标函数）的虚基类，这些值会用于牛顿法、坐标下降法和其他算法的求解
{                                         //从function派生出一些类，用于实现各种损失函数，包括I2r_Ir_fun，I2r_I2_svc_fun，I2r_I2_svr_fun
public:
	virtual double fun(double *w) = 0 ;              //计算损失函数值，w为权重数组
	virtual void grad(double *w, double *g) = 0 ;    //计算梯度值，w为权重数组，g为返回的梯度值
	virtual void Hv(double *s, double *Hs) = 0 ;     //计算Hessian矩阵与向量的乘积，s为传入向量，Hs为返回的Hessian与向量s的乘积

	virtual int get_nr_variable(void) = 0 ;          //获取特征向量的维数，即变量数
	virtual ~function(void){}
};                                            //这些抽象的虚函数并没有实现，它们由后面的派生类实现

class TRON                 //可信域牛顿法的实现类 为 L2正则化Logi回归 和 L2正则化支持向量机 训练提供支持
{
public:
	//传入的是基类function的指针，可以使用上面两种损失函数
	TRON(const function *fun_obj, double eps = 0.1, double eps_cg = 0.1, int max_iter = 1000);
	~TRON();

	void tron(double *w);                                                        //可信域牛顿法求解的主体，w为返回的权重参数      
	void set_print_string(void (*i_print) (const char *buf));

private:
	int trcg(double delta, double *g, double *s, double *r, bool *reach_boundary);  //共轭梯度法搜索近似牛顿方向，被tron调用
	double norm_inf(int n, double *x);

	double eps;                //迭代收敛精度
	double eps_cg;             
	int max_iter;              //最大迭代次数
	function *fun_obj;         //目标函数
	void info(const char *fmt,...);
	void (*tron_print_string)(const char *buf);
};
#endif
