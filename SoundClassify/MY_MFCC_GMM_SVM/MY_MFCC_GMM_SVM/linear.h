#ifndef _LIBLINEAR_H
#define _LIBLINEAR_H

#define LIBLINEAR_VERSION 211

#ifdef __cplusplus
extern "C" {
#endif

extern int liblinear_version;

struct feature_node
{
	int index;                            //特征编号，即特征向量中第几个特征  特征标号从1开始
	double value;                         //特征值
};

struct problem                            //定义问题
{
	int l, n;                             //l表示训练数据的数量，n表示特征的数量维数（如果bia>=0，则也包含bia这个特征）
	double *y;                            //目标值（或标签值）的向量
	struct feature_node **x;              //x是一个双重指针（指针向量），x[i][j]代表第i个特征向量的不为零的特征数据（特征编号，特征值），这样设计是为了适应特征向量是稀疏向量的情况，存储和计算上都有优势
	                                      //如果bias>=0,x最后要加一个元素，变为x；bias
	double bias;            /* < 0 if no bias term */ //如果>=0，则假设每组特征数据的后面多一个特征
};

/*
结构体 problem例子,如果有以下训练数据：
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
	int solver_type;                               //求解器类型，所有的类型见上面的enum，默认为 L2R_L2LOSS_SVC_DUAL

	/* these are for training only */
	double eps;	        /* stopping criteria */    //停止迭代条件
	double C;                                      //约束条件惩罚因子
	int nr_weight;                         //weight_label和weight向量的元素个数（权重数组的大小），如果不想改变任何类的惩罚项，则设置为0
	int *weight_label;                    //权重标签数组，每个weight[i]对应一个weight_label[i]，意味着类weight_label[i]的惩罚有weight[i]来调节
	double* weight;                       //权重数组
	double p;                            //支持向量回归损失的灵敏度
	double *init_sol;
};                                     //为了避免参数错误，应该在调用train()函数以后，调用check_parameter来检查
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
	struct parameter param;                      //模型参数
	int nr_class;		/* number of classes */  //类别的数量，特别的而对于回归问题，该参数为2
	int nr_feature;                              //特征的数量
	double *w;                                   //特征的权重，它的尺寸为 nr_feature*nr_class，如果nr_class=2，则为nr_feature；如果bias>=0,则w是一个（nr_feature+1）*nr_class的向量
	int *label;		/* label of each class */    //每类的标签值
	double bias;
};

struct model* train(const struct problem *prob, const struct parameter *param);  
//根据给定的数据集和参数，构造并返回一个线性分类或回归模型

void cross_validation(const struct problem *prob, const struct parameter *param, int nr_fold, double *target);
//该函数进行交叉验证，数据被分割成nr_fold组。在给定参数下，每一组都使用 其他组数据训练的模型 进行验证。 验证过程中的预测标签存在target的向量中

void find_parameter_C(const struct problem *prob, const struct parameter *param, int nr_fold, double start_C, double max_C, double *best_C, double *best_rate);
//此函数类似交叉验证函数cross_validation，不过与交叉验证函数需要特定的C参数不同的是，此函数通过在C=start_C，2*start_C，4*start_C...条件下进行多次交叉验证，找到最高交叉验证准确率的最优参数C

double predict_values(const struct model *model_, const struct feature_node *x, double* dec_values);
//该函数在向量dec_values中给定nr_w决定值
//对于回归问题或者类别数为2时，nr_w=1；一个例外：对于多分类SVM（-s 4），如果只有两类，nr_w=2；
//对于其他情况，nr_w为类别的数量

//对于多分类问题，采用的是一对多的分类策略（-s 0,1,2,3,5,6,7） 以及 多类对多类的策略(-s 4) 返回的是具有最高决定值的类

double predict(const struct model *model_, const struct feature_node *x);
//对于分类模型，该函数对某个样本x返回预测类型；对于回归模型，该函数计算法某个样本的回归值

double predict_probability(const struct model *model_, const struct feature_node *x, double* prob_estimates);
//该函数给出了nr_class（类别数量）概率估计值，在向量prob_estimates中。nr_class能够通过函数get_nr_class中获取，最高概率的类将会返回，目前只支持逻辑回归的概率输出

int save_model(const char *model_file_name, const struct model *model_);
//将模型信息存储到一个文件中，返回0代表成功存储
struct model *load_model(const char *model_file_name);
//从文件中读入模型信息

int get_nr_feature(const struct model *model_);
//返回模型的样本维度的大小
int get_nr_class(const struct model *model_);
//返回模型的类别数，回归模型返回2
void get_labels(const struct model *model_, int* label);
//将模型中的标签输入到label中，对于回归模型标签是不变的
double get_decfun_coef(const struct model *model_, int feat_idx, int label_idx);
//对于index=feat_idx的特征以及label index=lable_idx的类，返回系数
double get_decfun_bias(const struct model *model_, int label_idx);
//对于给定label_idx的类返回对应的bias大小

void free_model_content(struct model *model_ptr);
//释放一个模型结果所占用的内存
void free_and_destroy_model(struct model **model_ptr_ptr);
//释放模型占用的内存并摧毁模型结果
void destroy_param(struct parameter *param);
//释放一个参数集占用的内存

const char *check_parameter(const struct problem *prob, const struct parameter *param);
//检查参数是否在解决问题的范围内，该函数应该在调用train以及cross_validation之前调用，如果参数有效则返回NULL，否则会返回一个错误消息
int check_probability_model(const struct model *model);
//如果被检查的模型支持概率输出，则返回1，否则返回0
int check_regression_model(const struct model *model);
//如果被检查的模型是一个回归模型，则返回1，否则返回0
void set_print_string_function(void (*print_func) (const char*));
//用户可以自定义自己的输出格式通过print_func，使用set_print_string_function（NULL）为默认输出

#ifdef __cplusplus
}
#endif

#endif /* _LIBLINEAR_H */

