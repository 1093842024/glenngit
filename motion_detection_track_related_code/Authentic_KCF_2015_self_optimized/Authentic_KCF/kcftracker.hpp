/*

Tracker based on Kernelized Correlation Filter (KCF) [1] and Circulant Structure with Kernels (CSK) [2].
CSK is implemented by using raw gray level features, since it is a single-channel filter.
KCF is implemented by using HOG features (the default), since it extends CSK to multiple channels.

[1] J. F. Henriques, R. Caseiro, P. Martins, J. Batista,
"High-Speed Tracking with Kernelized Correlation Filters", TPAMI 2015.

[2] J. F. Henriques, R. Caseiro, P. Martins, J. Batista,
"Exploiting the Circulant Structure of Tracking-by-detection with Kernels", ECCV 2012.

Authors: Joao Faro, Christian Bailer, Joao F. Henriques
Contacts: joaopfaro@gmail.com, Christian.Bailer@dfki.de, henriques@isr.uc.pt
Institute of Systems and Robotics - University of Coimbra / Department Augmented Vision DFKI


Constructor parameters, all boolean:
    hog: use HOG features (default), otherwise use raw pixels
    fixed_window: fix window size (default), otherwise use ROI size (slower but more accurate)
    multiscale: use multi-scale tracking (default; cannot be used with fixed_window = true)

Default values are set for all properties of the tracker depending on the above choices.
Their values can be customized further before calling init():
    interp_factor: linear interpolation factor for adaptation
    sigma: gaussian kernel bandwidth
    lambda: regularization
    cell_size: HOG cell size
    padding: horizontal area surrounding the target, relative to its size
    output_sigma_factor: bandwidth of gaussian target
    template_size: template size in pixels, 0 to use ROI size
    scale_step: scale step for multi-scale estimation, 1 to disable it
    scale_weight: to downweight detection scores of other scales for added stability

For speed, the value (template_size/cell_size) should be a power of 2 or a product of small prime numbers.

Inputs to init():
   image is the initial frame.
   roi is a cv::Rect with the target positions in the initial frame

Inputs to update():
   image is the current frame.

Outputs of update():
   cv::Rect with target positions for the current frame


By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
 */

#pragma once

#include "tracker.h"
#include "track.h"

#ifndef _OPENCV_KCFTRACKER_HPP_
#define _OPENCV_KCFTRACKER_HPP_
#endif

class KCFTracker : public Tracker
{
public:
    // Constructor  构造函数
    KCFTracker(bool hog = true, bool fixed_window = true, bool multiscale = true, bool lab = true);

    // Initialize tracker  初始化跟踪器
    virtual void init(const cv::Rect &roi, cv::Mat image);
    
    // Update position based on the new frame  基于新的一帧图像进行位置更新
    virtual cv::Rect update(cv::Mat image);

    float interp_factor; // linear interpolation factor for adaptation   用于模型自适应的线性插值因子
    float sigma; // gaussian kernel bandwidth    高斯核的sigma
    float lambda; // regularization      正则化参数lambda
    int cell_size; // HOG cell size      HOG梯度直方图的cell尺寸
    int cell_sizeQ; // cell size^2, to avoid repeated operations    cell尺寸的平方，避免重复操作
    float padding; // extra area surrounding the target             目标周围的额外区域
    float output_sigma_factor; // bandwidth of gaussian target      高斯目标的带宽
    int template_size; // template size                             模板尺寸
    float scale_step; // scale step for multi-scale estimation      对尺度估计的步长
    float scale_weight;  // to downweight detection scores of other scales for added stability    为增加稳定性，降低其他尺度检测评分的权重

protected:
    // Detect object in the current frame.     在当前帧中检测目标
    cv::Point2f detect(cv::Mat z, cv::Mat x, float &peak_value);

    // train tracker with a single image        用一张单独图片训练跟踪器
    void train(cv::Mat x, float train_interp_factor);

	//对于输入特征图像X和Y（两者都必须是MxN维，且是周期性的，并经过cosine窗口预处理）之间的所有相对偏移，估计（对应的）带宽为SIGMA的高斯核（标签值）
    // Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
    cv::Mat gaussianCorrelation(cv::Mat x1, cv::Mat x2);

    // Create Gaussian Peak. Function called only in the first frame.  创造高斯峰。该函数只在第一帧被调用
    cv::Mat createGaussianPeak(int sizey, int sizex);

    // Obtain sub-window from image, with replication-padding and extract features   从图像中获取子窗口，进行复制填充和特征提取 
    cv::Mat getFeatures(const cv::Mat & image, bool inithann, float scale_adjust = 1.0f);

    // Initialize Hanning window. Function called only in the first frame. 初始化hanning窗（余弦平方窗）。该函数只在第一帧调用。  该函数用于特征提取
    void createHanningMats();

    // Calculate sub-pixel peak for one dimension    计算一维亚像素峰值   该函数用于detect函数中
    float subPixelPeak(float left, float center, float right);

    cv::Mat _alphaf;
    cv::Mat _prob;    //对应特征图尺寸的高斯标签峰
    cv::Mat _tmpl;    //获得的特征图
    cv::Mat _num;
    cv::Mat _den;
    cv::Mat _labCentroids;

	

private:
    int size_patch[3];   //分别存放特征图的尺寸以及维度信息
    cv::Mat hann;        //hanning窗口，尺寸由特征图的尺寸决定
    cv::Size _tmpl_sz;   //按模板大小转换以后的区域尺寸大小
    float _scale;        //padded尺寸与模板尺寸的比值
    int _gaussian_size;
    bool _hogfeatures;   
    bool _labfeatures;

    KalmanTracker kalman;  //跟踪目标位置的卡拉曼滤波器 added by gelin 2017-8-24
	std::vector<float> goal;          //最近十次分数；
	int lowgoaltimes;      //低分的次数
	
};
