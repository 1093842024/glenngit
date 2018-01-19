#include <opencv2/opencv.hpp>  
#include <iostream>  
  
  
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
                          const cv::Mat& OutputMat, int MaxIte) ;
  
  
  
  
/** 
 * @brief NeuralNetworksPredict 
 * @param NeuralNetworks 
 * @param Input 
 * @param Output 
 */  
void NeuralNetworksPredict(const CvANN_MLP& NeuralNetworks, const cv::Mat& Input,  
                           cv::Mat& Output);