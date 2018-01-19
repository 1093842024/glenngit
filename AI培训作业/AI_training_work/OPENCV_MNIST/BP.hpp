#ifndef _BP_HPP_
#define _BP_HPP_

namespace ANN {

#define num_node_input_BP	784 //�����ڵ���  ͼ���СΪ28*28=784
#define width_image_BP		28 //��һ��ͼ���
#define height_image_BP		28 //��һ��ͼ���
#define num_node_hidden_BP	120 //������ڵ���
#define num_node_output_BP	10 //�����ڵ���   ����Ľ���� 10��
#define alpha_learning_BP	0.8 //�������������ѧϰ��
#define beta_learning_BP	0.6 //�������������ѧϰ��
#define patterns_train_BP	60000 //ѵ��ģʽ����(����)   ���ݼ�����60000��ͼ����������
#define patterns_test_BP	10000 //�y��ģʽ����(����)   ���Լ�����10000��ͼ����������
#define iterations_BP		10000 //���ѵ������
#define accuracy_rate_BP	0.965 //Ҫ��ﵽ��׼ȷ��

	class BP {
	public:
		BP();
		~BP();

		void init(); //��ʼ��������ռ�
		bool train(); //ѵ��
		int predict(const int* data, int width, int height); //Ԥ�y
		bool readModelFile(const char* name); //��ȡ��ѵ���õ�BP model

	protected:
		void release(); //�ͷ�����Ŀռ�
		bool saveModelFile(const char* name); //��ѵ���õ�model������������������Ľڵ�����Ȩֵ����ֵ
		bool initWeightThreshold(); //��ʼ��������[-1, 1]֮������С��
		bool getSrcData(); //��ȡMNIST����
		void calcHiddenLayer(const int* data); //�������������
		void calcOutputLayer(); //������������
		void calcAdjuctOutputLayer(const int* data); //���������У�����
		void calcAdjuctHiddenLayer(); //����������У�����
		float calcActivationFunction(float x); //���㼤���������S�κ���
		void updateWeightThresholdOutputLayer(); //�����������������Ȩֵ����ֵ
		void updateWeightThresholdHiddenLayer(const int* data); //�����������������Ȩֵ����ֵ
		float test(); //ѵ����һ�μ���һ��׼ȷ��

	private:
		float weight1[num_node_input_BP][num_node_hidden_BP]; //�����������������Ȩֵ
		float weight2[num_node_hidden_BP][num_node_output_BP]; //�����������������Ȩֵ
		float threshold1[num_node_hidden_BP]; //��������ֵ
		float threshold2[num_node_output_BP]; //�������ֵ
		float output_hiddenLayer[num_node_hidden_BP]; //˳���������������ֵ
		float output_outputLayer[num_node_output_BP]; //˳��������������ֵ
		float adjust_error_outputLayer[num_node_output_BP]; //�洫���������У�����
		float adjust_error_hiddenLayer[num_node_hidden_BP]; //�洫����������У�����

		int* data_input_train; //ԭʼ��׼�������ݣ�ѵ��
		int* data_output_train; //ԭʼ��׼���������ѵ��
		int* data_input_test; //ԭʼ��׼�������ݣ��y��
		int* data_output_test; //ԭʼ��׼����������y��
	};

}

#endif //_BP_HPP_