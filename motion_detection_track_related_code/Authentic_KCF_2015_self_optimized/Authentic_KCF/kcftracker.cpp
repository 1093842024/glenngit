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
    padding: area surrounding the target, relative to its size
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

#ifndef _KCFTRACKER_HEADERS
#include "kcftracker.hpp"
#include "ffttools.hpp"
#include "recttools.hpp"
#include "fhog.hpp"
#include "labdata.hpp"
#endif

#define KALMAN
#define OPTIMIZATION1     //该步在源码基础上可以加速15-20ms，提高50%的速度，但跟丢概率稍有升高
#define OPTIMIZATION2       //该步在源码基础上可以加速3-4ms，在优化1基础上可以加速2ms左右

// Constructor
KCFTracker::KCFTracker(bool hog, bool fixed_window, bool multiscale, bool lab)
{

    // Parameters equal in all cases      全局参数：正则化参数lambda和模板范围参数2.5
    lambda = 0.0001;

#ifdef OPTIMIZATION1
	padding=2.2;
#else
    padding = 2.5;
#endif
    //output_sigma_factor = 0.1;
    output_sigma_factor = 0.125;

    if (hog) {    // HOG    使用HOG特征
        // VOT                   
        interp_factor = 0.012;    //     线性插值因子     
        sigma = 0.6;              //
        // TPAMI
        //interp_factor = 0.02;
        //sigma = 0.5;             
        cell_size = 4;          //HOG特征的cell尺寸
        _hogfeatures = true;

        if (lab) {             //在使用HOG特征的同时，使用Lab特征。lab颜色空间的特征   l亮度[0 100]   a 红色到绿色的范围[127 -128]   b黄色到蓝色的范围[127 -128]
            interp_factor = 0.005;      //插值因子
            sigma = 0.4; 
            //output_sigma_factor = 0.025;
            output_sigma_factor = 0.1;     //输出sigma因子
            _labfeatures = true;
            _labCentroids = cv::Mat(nClusters, 3, CV_32FC1, &data);
            cell_sizeQ = cell_size*cell_size;
        }
        else{
            _labfeatures = false;
        }
    }
    else {   // RAW     否则只用像素特征
        interp_factor = 0.075;    //插值因子
        sigma = 0.2;              
        cell_size = 1;
        _hogfeatures = false;

        if (lab) {
            printf("Lab features are only used with HOG features.\n");
            _labfeatures = false;
        }
    }


    if (multiscale) { // multiscale    优先判断是否开启多尺度功能
#ifdef OPTIMIZATION1
		template_size=64;
        scale_step = 1.07;             //多尺度的变化步长
#else
		template_size = 96;            //模板尺寸初始化为96   该参数越小（相同标记目标大小，_scale越大），会导致最后padded区域越大（标记区域大小*padding+增_scale*cell_size*2）反之最后padded区域稍小
		scale_step = 1.05;             //多尺度的变化步长
#endif
		//template_size = 64; template size in pixels, 0 to use ROI size   For speed, the value (template_size/cell_size) should be a power of 2 or a product of small prime numbers.
        
        scale_weight = 0.95;           //多尺度权重的变化参数
        if (!fixed_window) {           //如果fixedwindow设置为false 则把它修改设置为true，使逻辑关系正确（因为多尺度功能肯定是非限定窗口的）
            //printf("Multiscale does not support non-fixed window.\n");   多尺度不支持非固定窗口
            fixed_window = true;
        }
    }
    else if (fixed_window) {  // fit correction without multiscale   如果fixed_window为true，但是又不采用多尺度功能
        template_size = 96;                  //采用固定模板尺寸96，变化步长为1（即不变尺寸）
        //template_size = 100;
        scale_step = 1;
    }
    else {                  //如果即不采用多尺度multiscale， fixed_window又为false， 则模板尺寸固定为1
        template_size = 1;
        scale_step = 1;
    }
}

// Initialize tracker 
void KCFTracker::init(const cv::Rect &roi, cv::Mat image)
{
    _roi = roi;        //track父类中的保护成员_roi
    assert(roi.width >= 0 && roi.height >= 0);//assert宏定义函数作用是如果它的条件返回错误，则终止程序执行。如果其值为假（即为0），则它先向stderr打印一条错误信息，然后调用abort来终止程序执行。
    _tmpl = getFeatures(image, 1);            //获取初始图像特征，初始化特征图_tmpl
    _prob = createGaussianPeak(size_patch[0], size_patch[1]);    //创建高斯峰值，高斯标签的范围为特征图的范围，初始化为_prob
    _alphaf = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));   //初始化_alphaf参数的维度大小
    //_num = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
    //_den = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
    train(_tmpl, 1.0); // train with initial frame       利用初始特征图像的特征图_tmpl进行训练分类器（跟踪器）

#ifdef KALMAN
	Object klobject;                        ////added by gelin 2017-8-24
	klobject.x=roi.x;
	klobject.y=roi.y;
	klobject.vx=0;
	klobject.vy=0;
	kalman.Init(klobject);                ////end added
#endif
 }
// Update position based on the new frame
cv::Rect KCFTracker::update(cv::Mat image)
{
#ifdef KALMAN
	//double time_profile_counter1 = (double)cvGetTickCount();
	 kalman.Predict();                                 ////对roi的x y位置进行预测更正  added by gelin 2017-8-24
	 Object klobject=kalman.GetObject();
	 _roi.x=klobject.x;
	 _roi.y=klobject.y;                          
	// printf("before kcf:x %f y %f w %f h %f \n",_roi.x,_roi.y,_roi.width,_roi.height);         
	// time_profile_counter1 = (double)cvGetTickCount() - time_profile_counter1;
#endif                                                ////修改结束 ////end added

    if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 1;
    if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 1;
    if (_roi.x >= image.cols - 1) _roi.x = image.cols - 2;
    if (_roi.y >= image.rows - 1) _roi.y = image.rows - 2;

    float cx = _roi.x + _roi.width / 2.0f;
    float cy = _roi.y + _roi.height / 2.0f;

    float peak_value;
    //计算下一帧分类器响应最大值相对原来roi的位置偏移量res.x和res.y  
	int Choosesize=0;
	cv::Mat map1,map2,map3;
#ifdef OPTIMIZATION2
	map1=getFeatures(image, 0, 1.0f);
	cv::Point2f res = detect(_tmpl, map1, peak_value);
	Choosesize=1;
#else
	cv::Point2f res = detect(_tmpl, getFeatures(image, 0, 1.0f), peak_value); //对目前目标区域_roi获得featuremap，然后利用原来的特征图矩阵_tmpl进行detect 计算最优区域偏移位置及最高分
#endif
    if (scale_step != 1) {
        // Test at a smaller _scale
        float new_peak_value;

#ifdef OPTIMIZATION2
		map2=getFeatures(image, 0, 1.0f / scale_step);
		cv::Point2f new_res = detect(_tmpl, map2, new_peak_value);	
#else
        cv::Point2f new_res = detect(_tmpl, getFeatures(image, 0, 1.0f / scale_step), new_peak_value);  //对小尺度（1/scale_step倍）计算结果和最高得分
#endif
        if (scale_weight * new_peak_value > peak_value) {    //如果得分更高，则将结果、最高得分以及尺度信息进行更新
            Choosesize=2;
            res = new_res;
            peak_value = new_peak_value;
            _scale /= scale_step;
            _roi.width /= scale_step;
            _roi.height /= scale_step;
        }

#ifdef OPTIMIZATION2
		map3=getFeatures(image, 0, scale_step);
		new_res = detect(_tmpl, map3, new_peak_value);	
#else
		//double time_profile_counter = (double)cvGetTickCount();
        // Test at a bigger _scale
        new_res = detect(_tmpl, getFeatures(image, 0, scale_step), new_peak_value);  //对大尺度（scale_step）计算结果和最高得分
		//time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
	   // std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
#endif
        if (scale_weight * new_peak_value > peak_value) {    //如果得分更高，则将结果、最高得分以及尺度信息进行更新
			Choosesize=3;
            res = new_res;
            peak_value = new_peak_value;
            _scale *= scale_step;
            _roi.width *= scale_step;
            _roi.height *= scale_step;
        }
    }

    // Adjust by cell size and _scale     //根据上面更新的尺度信息，重新得到目标区域的尺度尺寸
    _roi.x = cx - _roi.width / 2.0f + ((float) res.x * cell_size * _scale);      
    _roi.y = cy - _roi.height / 2.0f + ((float) res.y * cell_size * _scale);

#ifdef KALMAN
	//double time_profile_counter2 = (double)cvGetTickCount();
	KLRect updateklobj;                                          ////added by gelin 2017-8-24
	updateklobj.x=_roi.x;
	updateklobj.y=_roi.y;
	//printf("kcf:x %f y %f w %f h %f \n",_roi.x,_roi.y,_roi.width,_roi.height);      
	kalman.Update(updateklobj);                  
	
	klobject=kalman.GetObject();
	_roi.x=klobject.x;
	_roi.y=klobject.y;
	//printf("just:x %f y %f w %f h %f \n",_roi.x,_roi.y,_roi.width,_roi.height); 
	// printf("just:vx %0.1f vy %0.1f \n",klobject.vx,klobject.vy);       
	//time_profile_counter2 = (double)cvGetTickCount() - time_profile_counter2;
	//std::cout << "kalman time used          " << (time_profile_counter1+time_profile_counter2) / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
#endif                                                            ////修改结束

    if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;   //调整目标区域尺寸，避免超出整个图像的范围
    if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
    if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
    if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

    assert(_roi.width >= 0 && _roi.height >= 0);   //确定目标区域存在

	
#ifdef OPTIMIZATION2
	cv::Mat x;
	if(Choosesize==1) x=map1;
	else if (Choosesize==2) x=map2;
	else if(Choosesize==3) x=map3;

#else
    //double time_profile_counter = (double)cvGetTickCount();
    cv::Mat x = getFeatures(image, 0);
	//time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
	//std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
#endif	

	

	if(goal.size()<10) goal.push_back(peak_value);
	else{
		goal.erase(goal.begin());
		goal.push_back(peak_value);
	}

	lowgoaltimes=0;
	for(std::vector<float>::iterator it=goal.begin();it!=goal.end();it++)
	{
	    if((*it)<0.2) lowgoaltimes++;
	}
	
	if(lowgoaltimes>6) printf("target missing!\n");


	//double time_profi;e_counter = (double)cvGetTickCount();
    train(x, interp_factor);
	//time_profile_counter = (double)cvGetTickCount() - time_profile_counter;
	//std::cout << "time used " << time_profile_counter / ((double)cvGetTickFrequency() * 1000) << "ms" << std::endl;
	
    return _roi;
}


// Detect object in the current frame.
cv::Point2f KCFTracker::detect(cv::Mat z, cv::Mat x, float &peak_value)
{
    using namespace FFTTools;

    cv::Mat k = gaussianCorrelation(x, z);
    cv::Mat res = (real(fftd(complexMultiplication(_alphaf, fftd(k)), true)));

    //minMaxLoc only accepts doubles for the peak, and integer points for the coordinates
    cv::Point2i pi;
    double pv;
    cv::minMaxLoc(res, NULL, &pv, NULL, &pi);
    peak_value = (float) pv;

    //subpixel peak estimation, coordinates will be non-integer
    cv::Point2f p((float)pi.x, (float)pi.y);

    if (pi.x > 0 && pi.x < res.cols-1) {
        p.x += subPixelPeak(res.at<float>(pi.y, pi.x-1), peak_value, res.at<float>(pi.y, pi.x+1));
    }

    if (pi.y > 0 && pi.y < res.rows-1) {
        p.y += subPixelPeak(res.at<float>(pi.y-1, pi.x), peak_value, res.at<float>(pi.y+1, pi.x));
    }

    p.x -= (res.cols) / 2;
    p.y -= (res.rows) / 2;

    return p;
}

// train tracker with a single image
void KCFTracker::train(cv::Mat x, float train_interp_factor)
{
    using namespace FFTTools;

    cv::Mat k = gaussianCorrelation(x, x);    //估计输入特征图的高斯核循环矩阵
    cv::Mat alphaf = complexDivision(_prob, (fftd(k) + lambda));    //根据参数更新公式计算分类器参数a=y/（kxx+lambda） 其中_prob为y，fftd(k)为kxx，lambda即为lambda
    
    _tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor) * x; //对目标的特征图进行插值更新
    _alphaf = (1 - train_interp_factor) * _alphaf + (train_interp_factor) * alphaf;  //对跟踪检测分类器的对偶参数进行插值更新


    /*cv::Mat kf = fftd(gaussianCorrelation(x, x));
    cv::Mat num = complexMultiplication(kf, _prob);
    cv::Mat den = complexMultiplication(kf, kf + lambda);
    
    _tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor) * x;
    _num = (1 - train_interp_factor) * _num + (train_interp_factor) * num;
    _den = (1 - train_interp_factor) * _den + (train_interp_factor) * den;

    _alphaf = complexDivision(_num, _den);*/

}

// Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
cv::Mat KCFTracker::gaussianCorrelation(cv::Mat x1, cv::Mat x2)
{
    using namespace FFTTools;
    cv::Mat c = cv::Mat( cv::Size(size_patch[1], size_patch[0]), CV_32F, cv::Scalar(0) );
    // HOG features
    if (_hogfeatures) {
        cv::Mat caux;
        cv::Mat x1aux;
        cv::Mat x2aux;
        for (int i = 0; i < size_patch[2]; i++) {
            x1aux = x1.row(i);   // Procedure do deal with cv::Mat multichannel bug
            x1aux = x1aux.reshape(1, size_patch[0]);
            x2aux = x2.row(i).reshape(1, size_patch[0]);
            cv::mulSpectrums(fftd(x1aux), fftd(x2aux), caux, 0, true); 
            caux = fftd(caux, true);
            rearrange(caux);
            caux.convertTo(caux,CV_32F);
            c = c + real(caux);
        }
    }
    // Gray features
    else {
        cv::mulSpectrums(fftd(x1), fftd(x2), c, 0, true);
        c = fftd(c, true);
        rearrange(c);
        c = real(c);
    }
    cv::Mat d; 
    cv::max(( (cv::sum(x1.mul(x1))[0] + cv::sum(x2.mul(x2))[0])- 2. * c) / (size_patch[0]*size_patch[1]*size_patch[2]) , 0, d);

    cv::Mat k;
    cv::exp((-d / (sigma * sigma)), k);
    return k;
}

// Create Gaussian Peak. Function called only in the first frame.  创建高斯峰（标签）的位置，只在第一帧调用
cv::Mat KCFTracker::createGaussianPeak(int sizey, int sizex)
{
    cv::Mat_<float> res(sizey, sizex);

    int syh = (sizey) / 2;
    int sxh = (sizex) / 2;

    float output_sigma = std::sqrt((float) sizex * sizey) / padding * output_sigma_factor;
    float mult = -0.5 / (output_sigma * output_sigma);

    for (int i = 0; i < sizey; i++)
        for (int j = 0; j < sizex; j++)
        {
            int ih = i - syh;
            int jh = j - sxh;
            res(i, j) = std::exp(mult * (float) (ih * ih + jh * jh));
        }
    return FFTTools::fftd(res);
}

// Obtain sub-window from image, with replication-padding and extract features
cv::Mat KCFTracker::getFeatures(const cv::Mat & image, bool inithann, float scale_adjust)
{
    cv::Rect extracted_roi;

    float cx = _roi.x + _roi.width / 2;
    float cy = _roi.y + _roi.height / 2;

	//首先进行输入图像区域的大小计算
    if (inithann) {    //为true（1）  开启hanning 窗口
        int padded_w = _roi.width * padding;     //目标区域的padding倍（padded_w，padded_h）大小作为训练输入padded
        int padded_h = _roi.height * padding;
        
        if (template_size > 1) {  // Fit largest dimension to the given template size   如果模板尺寸大于1（使用多尺度或限定窗口情况），将最大维度设定为给定的模板尺寸（即将padded区域的最大尺寸（宽或高）变换到模板尺寸）
            if (padded_w >= padded_h)  //fit to width   如果padded区域的宽大于等于高 
                _scale = padded_w / (float) template_size;     //将_scale参数按照padded_w来用 模板尺寸 归一化
            else
                _scale = padded_h / (float) template_size;     //否则将_scale参数按照padded_h来用 模板尺寸 归一化

            _tmpl_sz.width = padded_w / _scale;           //然后利用_scale参数归一化_tmpl_sz的尺寸（此时宽或高的最大尺寸为模板尺寸）
            _tmpl_sz.height = padded_h / _scale;
        }
        else {  //No template size given, use ROI size  无给定的模板尺寸，使用ROI尺寸  
            _tmpl_sz.width = padded_w;                   //将_tmpl_sz尺寸直接设定为padded的尺寸
            _tmpl_sz.height = padded_h;
            _scale = 1;
            // original code from paper:
            /*if (sqrt(padded_w * padded_h) >= 100) {   //Normal size
                _tmpl_sz.width = padded_w;
                _tmpl_sz.height = padded_h;
                _scale = 1;
            }
            else {   //ROI is too big, track at half size
                _tmpl_sz.width = padded_w / 2;
                _tmpl_sz.height = padded_h / 2;
                _scale = 2;
            }*/
        }

        if (_hogfeatures) {                          //如果创建kcf时使用了hog特征
            // Round to cell size and also make it even     使选定的_tmpl_sz模板尺寸的长宽各增加两个cell_size，并让总尺寸的长宽都是一个偶数
            _tmpl_sz.width = ( ( (int)(_tmpl_sz.width / (2 * cell_size)) ) * 2 * cell_size ) + cell_size*2;
            _tmpl_sz.height = ( ( (int)(_tmpl_sz.height / (2 * cell_size)) ) * 2 * cell_size ) + cell_size*2;
        }
        else {  //Make number of pixels even (helps with some logic involving half-dimensions) 让总尺寸的长宽都是一个偶数，有利于涉及半维度的一些逻辑
            _tmpl_sz.width = (_tmpl_sz.width / 2) * 2;
            _tmpl_sz.height = (_tmpl_sz.height / 2) * 2;
        }
    }

    extracted_roi.width = scale_adjust * _scale * _tmpl_sz.width;  //将提取特征的区域尺寸变换为标记区域的padding倍尺寸大小  
    extracted_roi.height = scale_adjust * _scale * _tmpl_sz.height;

	//之前的变换主要是影响在于，如果开启hog特征会对最后的extracted_roi尺寸造成影响、如果没有开启，extracted_roi大小基本就是标记区域的padding倍
	//如果开启了hog特征，若初始标记区域的最大尺寸远大与模板尺寸，则_scale会很大，造成后面加上cell_size*2后再变换回去，导致extracted_roi增加的尺寸是_scale*cell_size*2大小

    // center roi with new size
    extracted_roi.x = cx - extracted_roi.width / 2;
    extracted_roi.y = cy - extracted_roi.height / 2;

    cv::Mat FeaturesMap;  
    cv::Mat z = RectTools::subwindow(image, extracted_roi, cv::BORDER_REPLICATE);//提取目标区域与全部图像的公共部分的窗口？
    
    if (z.cols != _tmpl_sz.width || z.rows != _tmpl_sz.height) {   //如果划分的子窗口的 宽 或 高 与（模板大小+cell_size*2）不相等
        cv::resize(z, z, _tmpl_sz);      //将划分的子窗口的大小调整到和_tmpl_sz(即模板大小+cell_size*2)相同
    }   

    // HOG features
    if (_hogfeatures) {
        IplImage z_ipl = z;
        CvLSVMFeatureMapCaskade *map;
        getFeatureMaps(&z_ipl, cell_size, &map);   //获取子窗口图的feature map
        normalizeAndTruncate(map,0.2f);            //对feature map进行规范化和截断
        PCAFeatureMaps(map);                       //对feature map进行PCA算法降维
        size_patch[0] = map->sizeY;               
        size_patch[1] = map->sizeX;
        size_patch[2] = map->numFeatures;

        FeaturesMap = cv::Mat(cv::Size(map->numFeatures,map->sizeX*map->sizeY), CV_32F, map->map);  // Procedure do deal with cv::Mat multichannel bug
        FeaturesMap = FeaturesMap.t();
        freeFeatureMapObject(&map);

        // Lab features
        if (_labfeatures) {
            cv::Mat imgLab;
            cvtColor(z, imgLab, CV_BGR2Lab);
            unsigned char *input = (unsigned char*)(imgLab.data);

            // Sparse output vector
            cv::Mat outputLab = cv::Mat(_labCentroids.rows, size_patch[0]*size_patch[1], CV_32F, float(0));

            int cntCell = 0;
            // Iterate through each cell
            for (int cY = cell_size; cY < z.rows-cell_size; cY+=cell_size){
                for (int cX = cell_size; cX < z.cols-cell_size; cX+=cell_size){
                    // Iterate through each pixel of cell (cX,cY)
                    for(int y = cY; y < cY+cell_size; ++y){
                        for(int x = cX; x < cX+cell_size; ++x){
                            // Lab components for each pixel
                            float l = (float)input[(z.cols * y + x) * 3];
                            float a = (float)input[(z.cols * y + x) * 3 + 1];
                            float b = (float)input[(z.cols * y + x) * 3 + 2];

                            // Iterate trough each centroid
                            float minDist = FLT_MAX;
                            int minIdx = 0;
                            float *inputCentroid = (float*)(_labCentroids.data);
                            for(int k = 0; k < _labCentroids.rows; ++k){
                                float dist = ( (l - inputCentroid[3*k]) * (l - inputCentroid[3*k]) )
                                           + ( (a - inputCentroid[3*k+1]) * (a - inputCentroid[3*k+1]) ) 
                                           + ( (b - inputCentroid[3*k+2]) * (b - inputCentroid[3*k+2]) );
                                if(dist < minDist){
                                    minDist = dist;
                                    minIdx = k;
                                }
                            }
                            // Store result at output
                            outputLab.at<float>(minIdx, cntCell) += 1.0 / cell_sizeQ; 
                            //((float*) outputLab.data)[minIdx * (size_patch[0]*size_patch[1]) + cntCell] += 1.0 / cell_sizeQ; 
                        }
                    }
                    cntCell++;
                }
            }
            // Update size_patch[2] and add features to FeaturesMap
            size_patch[2] += _labCentroids.rows;
            FeaturesMap.push_back(outputLab);
        }
    }
    else {
        FeaturesMap = RectTools::getGrayImage(z);
        FeaturesMap -= (float) 0.5; // In Paper;
        size_patch[0] = z.rows;
        size_patch[1] = z.cols;
        size_patch[2] = 1;  
    }
    
    if (inithann) {
        createHanningMats();
    }
    FeaturesMap = hann.mul(FeaturesMap);//计算hann窗口矩阵与FeatureMap矩阵对应位的乘积
    return FeaturesMap;
}
    
// Initialize Hanning window. Function called only in the first frame.  该函数用于特征提取
void KCFTracker::createHanningMats()
{   
    cv::Mat hann1t = cv::Mat(cv::Size(size_patch[1],1), CV_32F, cv::Scalar(0));
    cv::Mat hann2t = cv::Mat(cv::Size(1,size_patch[0]), CV_32F, cv::Scalar(0)); 

    for (int i = 0; i < hann1t.cols; i++)
        hann1t.at<float > (0, i) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann1t.cols - 1)));
    for (int i = 0; i < hann2t.rows; i++)
        hann2t.at<float > (i, 0) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann2t.rows - 1)));

    cv::Mat hann2d = hann2t * hann1t;
    // HOG features
    if (_hogfeatures) {
        cv::Mat hann1d = hann2d.reshape(1,1); // Procedure do deal with cv::Mat multichannel bug
        
        hann = cv::Mat(cv::Size(size_patch[0]*size_patch[1], size_patch[2]), CV_32F, cv::Scalar(0));
        for (int i = 0; i < size_patch[2]; i++) {
            for (int j = 0; j<size_patch[0]*size_patch[1]; j++) {
                hann.at<float>(i,j) = hann1d.at<float>(0,j);
            }
        }
    }
    // Gray features
    else {
        hann = hann2d;
    }
}

// Calculate sub-pixel peak for one dimension   该函数用于detect函数中
float KCFTracker::subPixelPeak(float left, float center, float right)
{   
    float divisor = 2 * center - right - left;

    if (divisor == 0)
        return 0;
    
    return 0.5 * (right - left) / divisor;
}
