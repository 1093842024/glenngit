#ifndef _LIBLINEAR_H
#define _LIBLINEAR_H

#define LIBLINEAR_VERSION 211

#ifdef __cplusplus
extern "C" {
#endif

extern int liblinear_version;

struct feature_node
{
	int index;                            //������ţ������������еڼ�������  ������Ŵ�1��ʼ
	double value;                         //����ֵ
};

struct problem                            //��������
{
	int l, n;                             //l��ʾѵ�����ݵ�������n��ʾ����������ά�������bia>=0����Ҳ����bia���������
	double *y;                            //Ŀ��ֵ�����ǩֵ��������
	struct feature_node **x;              //x��һ��˫��ָ�루ָ����������x[i][j]�����i�����������Ĳ�Ϊ����������ݣ�������ţ�����ֵ�������������Ϊ����Ӧ����������ϡ��������������洢�ͼ����϶�������
	                                      //���bias>=0,x���Ҫ��һ��Ԫ�أ���Ϊx��bias
	double bias;            /* < 0 if no bias term */ //���>=0�������ÿ���������ݵĺ����һ������
};

/*
�ṹ�� problem����,���������ѵ�����ݣ�
   LABEL       ATTR1   ATTR2   ATTR3   ATTR4   ATTR5
    -----       -----   -----   -----   -----   -----
    1           0       0.1     0.2     0       0
    2           0       0.1     0.3    -1.2     0
    1           0.4     0       0       0       0
    2           0       0.1     0       1.4     0.5
    3          -0.1    -0.2     0.1     1.1     0.1
    and bias = 1, then the components of problem are:

    l = 5
    n = 6
    y -> 1 2 1 2 3
    x -> [ ] -> (2,0.1) (3,0.2) (6,1) (-1,?)
         [ ] -> (2,0.1) (3,0.3) (4,-1.2) (6,1) (-1,?)
         [ ] -> (1,0.4) (6,1) (-1,?)
         [ ] -> (2,0.1) (4,1.4) (5,0.5) (6,1) (-1,?)
         [ ] -> (1,-0.1) (2,-0.2) (3,0.1) (4,1.1) (5,0.1) (6,1) (-1,?)
*/



enum { L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL, L2R_L2LOSS_SVR = 11, L2R_L2LOSS_SVR_DUAL, L2R_L1LOSS_SVR_DUAL }; /* solver_type */

struct parameter
{
	int solver_type;                               //��������ͣ����е����ͼ������enum��Ĭ��Ϊ L2R_L2LOSS_SVC_DUAL

	/* these are for training only */
	double eps;	        /* stopping criteria */    //ֹͣ��������
	double C;                                      //Լ�������ͷ�����
	int nr_weight;                         //weight_label��weight������Ԫ�ظ�����Ȩ������Ĵ�С�����������ı��κ���ĳͷ��������Ϊ0
	int *weight_label;                    //Ȩ�ر�ǩ���飬ÿ��weight[i]��Ӧһ��weight_label[i]����ζ����weight_label[i]�ĳͷ���weight[i]������
	double* weight;                       //Ȩ������
	double p;                            //֧�������ع���ʧ��������
	double *init_sol;
};                                     //Ϊ�˱����������Ӧ���ڵ���train()�����Ժ󣬵���check_parameter�����
/*
 solver_type can be one of L2R_LR, L2R_L2LOSS_SVC_DUAL, L2R_L2LOSS_SVC, L2R_L1LOSS_SVC_DUAL, MCSVM_CS, L1R_L2LOSS_SVC, L1R_LR, L2R_LR_DUAL, L2R_L2LOSS_SVR, L2R_L2LOSS_SVR_DUAL, L2R_L1LOSS_SVR_DUAL.
  for classification
    L2R_LR                L2-regularized logistic regression (primal)
    L2R_L2LOSS_SVC_DUAL   L2-regularized L2-loss support vector classification (dual)
    L2R_L2LOSS_SVC        L2-regularized L2-loss support vector classification (primal)
    L2R_L1LOSS_SVC_DUAL   L2-regularized L1-loss support vector classification (dual)
    MCSVM_CS              support vector classification by Crammer and Singer
    L1R_L2LOSS_SVC        L1-regularized L2-loss support vector classification
    L1R_LR                L1-regularized logistic regression
    L2R_LR_DUAL           L2-regularized logistic regression (dual)
  for regression
    L2R_L2LOSS_SVR        L2-regularized L2-loss support vector regression (primal)
    L2R_L2LOSS_SVR_DUAL   L2-regularized L2-loss support vector regression (dual)
    L2R_L1LOSS_SVR_DUAL   L2-regularized L1-loss support vector regression (dual)
*/

struct model
{
	struct parameter param;                      //ģ�Ͳ���
	int nr_class;		/* number of classes */  //�����������ر�Ķ����ڻع����⣬�ò���Ϊ2
	int nr_feature;                              //����������
	double *w;                                   //������Ȩ�أ����ĳߴ�Ϊ nr_feature*nr_class�����nr_class=2����Ϊnr_feature�����bias>=0,��w��һ����nr_feature+1��*nr_class������
	int *label;		/* label of each class */    //ÿ��ı�ǩֵ
	double bias;
};

struct model* train(const struct problem *prob, const struct parameter *param);  
//���ݸ��������ݼ��Ͳ��������첢����һ�����Է����ع�ģ��

void cross_validation(const struct problem *prob, const struct parameter *param, int nr_fold, double *target);
//�ú������н�����֤�����ݱ��ָ��nr_fold�顣�ڸ��������£�ÿһ�鶼ʹ�� ����������ѵ����ģ�� ������֤�� ��֤�����е�Ԥ���ǩ����target��������

void find_parameter_C(const struct problem *prob, const struct parameter *param, int nr_fold, double start_C, double max_C, double *best_C, double *best_rate);
//�˺������ƽ�����֤����cross_validation�������뽻����֤������Ҫ�ض���C������ͬ���ǣ��˺���ͨ����C=start_C��2*start_C��4*start_C...�����½��ж�ν�����֤���ҵ���߽�����֤׼ȷ�ʵ����Ų���C

double predict_values(const struct model *model_, const struct feature_node *x, double* dec_values);
//�ú���������dec_values�и���nr_w����ֵ
//���ڻع�������������Ϊ2ʱ��nr_w=1��һ�����⣺���ڶ����SVM��-s 4�������ֻ�����࣬nr_w=2��
//�������������nr_wΪ��������

//���ڶ�������⣬���õ���һ�Զ�ķ�����ԣ�-s 0,1,2,3,5,6,7�� �Լ� ����Զ���Ĳ���(-s 4) ���ص��Ǿ�����߾���ֵ����

double predict(const struct model *model_, const struct feature_node *x);
//���ڷ���ģ�ͣ��ú�����ĳ������x����Ԥ�����ͣ����ڻع�ģ�ͣ��ú������㷨ĳ�������Ļع�ֵ

double predict_probability(const struct model *model_, const struct feature_node *x, double* prob_estimates);
//�ú���������nr_class��������������ʹ���ֵ��������prob_estimates�С�nr_class�ܹ�ͨ������get_nr_class�л�ȡ����߸��ʵ��ཫ�᷵�أ�Ŀǰֻ֧���߼��ع�ĸ������

int save_model(const char *model_file_name, const struct model *model_);
//��ģ����Ϣ�洢��һ���ļ��У�����0����ɹ��洢
struct model *load_model(const char *model_file_name);
//���ļ��ж���ģ����Ϣ

int get_nr_feature(const struct model *model_);
//����ģ�͵�����ά�ȵĴ�С
int get_nr_class(const struct model *model_);
//����ģ�͵���������ع�ģ�ͷ���2
void get_labels(const struct model *model_, int* label);
//��ģ���еı�ǩ���뵽label�У����ڻع�ģ�ͱ�ǩ�ǲ����
double get_decfun_coef(const struct model *model_, int feat_idx, int label_idx);
//����index=feat_idx�������Լ�label index=lable_idx���࣬����ϵ��
double get_decfun_bias(const struct model *model_, int label_idx);
//���ڸ���label_idx���෵�ض�Ӧ��bias��С

void free_model_content(struct model *model_ptr);
//�ͷ�һ��ģ�ͽ����ռ�õ��ڴ�
void free_and_destroy_model(struct model **model_ptr_ptr);
//�ͷ�ģ��ռ�õ��ڴ沢�ݻ�ģ�ͽ��
void destroy_param(struct parameter *param);
//�ͷ�һ��������ռ�õ��ڴ�

const char *check_parameter(const struct problem *prob, const struct parameter *param);
//�������Ƿ��ڽ������ķ�Χ�ڣ��ú���Ӧ���ڵ���train�Լ�cross_validation֮ǰ���ã����������Ч�򷵻�NULL������᷵��һ��������Ϣ
int check_probability_model(const struct model *model);
//���������ģ��֧�ָ���������򷵻�1�����򷵻�0
int check_regression_model(const struct model *model);
//���������ģ����һ���ع�ģ�ͣ��򷵻�1�����򷵻�0
void set_print_string_function(void (*print_func) (const char*));
//�û������Զ����Լ��������ʽͨ��print_func��ʹ��set_print_string_function��NULL��ΪĬ�����

#ifdef __cplusplus
}
#endif

#endif /* _LIBLINEAR_H */

