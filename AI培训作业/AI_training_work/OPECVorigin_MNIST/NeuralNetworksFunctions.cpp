#include <opencv2/opencv.hpp>  
#include <iostream>  
  
#include "NeuralNetworksFunctions.h"  
  
/** 
 * @brief NeuralNetworksTraing Training the neural networks 
 * @param NeuralNetworks   The neural network 
 * @param InputMat         Floating-point matrix of input vectors, one vector 
 *                         per row. 
 * @param OutputMat        Floating-point matrix of the corresponding output 
 *                         vectors, one vector per row. 
 * @param MaxIte           The number of the iteration 
 * 
 * 
 * @author sheng 
 * @version 1.0.0 
 * @date  2014-04-17 
 * 
 * @histroy     <author>      <date>      <version>      <description> 
 *               sheng      2014-04-08      1.0.0      build the function 
 * 
 */  
  
void NeuralNetworksTraing(CvANN_MLP& NeuralNetworks, const cv::Mat& InputMat,  
                          const cv::Mat& OutputMat, int MaxIte)  
{  
    // Network architecture  
    std::vector<int> LayerSizes;  
    LayerSizes.push_back(InputMat.cols);    // input layer  
    LayerSizes.push_back(500);             // hidden layer has 1000 neurons  
    LayerSizes.push_back(OutputMat.cols);   // output layer  
  
  
    // Activate function  
    int ActivateFunc = CvANN_MLP::SIGMOID_SYM;  
    double Alpha = 1;  
    double Beta = 1;  
  
  
    // create the network  
    NeuralNetworks.create(cv::Mat(LayerSizes), ActivateFunc, Alpha, Beta);  
  
  
  
    // Training Params  
    CvANN_MLP_TrainParams TrainParams;  
    TrainParams.train_method = CvANN_MLP_TrainParams::BACKPROP;  
    TrainParams.bp_dw_scale = 0.001;  
    TrainParams.bp_moment_scale = 0.001;  
  
    // iteration number  
    CvTermCriteria TermCrlt;  
    TermCrlt.type = CV_TERMCRIT_ITER | CV_TERMCRIT_EPS;  
    TermCrlt.epsilon = 0.00001f;  
    TermCrlt.max_iter = MaxIte;  
    TrainParams.term_crit = TermCrlt;  
  
  
    // Training the networks  
    NeuralNetworks.train(InputMat, OutputMat, cv::Mat(), cv::Mat(), TrainParams);  
  
}  
  
void DecodeOutputVector(cv::Mat OutputVector, cv::Mat Output, int cols)
{
	int max;
	for(int j=0;j<OutputVector.rows;j++)
	{
		int max=OutputVector.data[j*10];
		Output.data[j]=0;
         for(int i=1;i<cols;i++)
	    {
	        if(OutputVector.data[j*10+i]>max)
		    {
				max=OutputVector.data[i];
				Output.data[j]=i;
		    }
	    }
	}
}
  
  
/** 
 * @brief NeuralNetworksPredict 
 * @param NeuralNetworks 
 * @param Input 
 * @param Output 
 */  
void NeuralNetworksPredict(const CvANN_MLP& NeuralNetworks, const cv::Mat& Input,  
                           cv::Mat& Output)  
{  
    // Neural network predict  
    cv::Mat OutputVector;  
    NeuralNetworks.predict(Input, OutputVector); 

	printf("predict output.rows:%d,predict output.cols:%d\n",OutputVector.rows,OutputVector.cols);
  
    // change the output vector  
    DecodeOutputVector(OutputVector, Output, OutputVector.cols);  
  
  
}  