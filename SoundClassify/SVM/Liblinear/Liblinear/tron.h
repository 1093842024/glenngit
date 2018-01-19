#ifndef _TRON_H
#define _TRON_H

class function                             //����ʧ������Ŀ�꺯����������࣬��Щֵ������ţ�ٷ��������½����������㷨�����
{                                         //��function������һЩ�࣬����ʵ�ָ�����ʧ����������I2r_Ir_fun��I2r_I2_svc_fun��I2r_I2_svr_fun
public:
	virtual double fun(double *w) = 0 ;              //������ʧ����ֵ��wΪȨ������
	virtual void grad(double *w, double *g) = 0 ;    //�����ݶ�ֵ��wΪȨ�����飬gΪ���ص��ݶ�ֵ
	virtual void Hv(double *s, double *Hs) = 0 ;     //����Hessian�����������ĳ˻���sΪ����������HsΪ���ص�Hessian������s�ĳ˻�

	virtual int get_nr_variable(void) = 0 ;          //��ȡ����������ά������������
	virtual ~function(void){}
};                                            //��Щ������麯����û��ʵ�֣������ɺ����������ʵ��

class TRON                 //������ţ�ٷ���ʵ���� Ϊ L2����Logi�ع� �� L2����֧�������� ѵ���ṩ֧��
{
public:
	//������ǻ���function��ָ�룬����ʹ������������ʧ����
	TRON(const function *fun_obj, double eps = 0.1, double eps_cg = 0.1, int max_iter = 1000);
	~TRON();

	void tron(double *w);                                                        //������ţ�ٷ��������壬wΪ���ص�Ȩ�ز���      
	void set_print_string(void (*i_print) (const char *buf));

private:
	int trcg(double delta, double *g, double *s, double *r, bool *reach_boundary);  //�����ݶȷ���������ţ�ٷ��򣬱�tron����
	double norm_inf(int n, double *x);

	double eps;                //������������
	double eps_cg;             
	int max_iter;              //����������
	function *fun_obj;         //Ŀ�꺯��
	void info(const char *fmt,...);
	void (*tron_print_string)(const char *buf);
};
#endif
