#include "QRdecoder.h"
#include "zbar.h"  
using namespace zbar;  //添加zbar名称空间   

struct CMAT{
	unsigned char* data;
	int cols;
	int rows;
};


int OtsuAlgThreshold(unsigned char* img,int cols,int rows)  
{  
    CMAT image;
	image.data=img;
	image.cols=cols;
	image.rows=rows;

    int T=0; //Otsu算法阈值  
    double varValue=0; //类间方差中间值保存  
    double w0=0; //前景像素点数所占比例  
    double w1=0; //背景像素点数所占比例  
    double u0=0; //前景平均灰度  
    double u1=0; //背景平均灰度  
    double Histogram[256]={0}; //灰度直方图，下标是灰度值，保存内容是灰度值对应的像素点总数  
    unsigned char *data=image.data;  
    double totalNum=image.rows*image.cols; //像素总数  
    //计算灰度直方图分布，Histogram数组下标是灰度值，保存内容是灰度值对应像素点数  
    for(int i=0;i<image.rows;i++)   //为表述清晰，并没有把rows和cols单独提出来  
    {  
        for(int j=0;j<image.cols;j++)  
        {  
            Histogram[data[i*image.cols+j]]++;  
        }  
    }  
    for(int i=0;i<255;i++)  
    {  
        //每次遍历之前初始化各变量  
        w1=0;       u1=0;       w0=0;       u0=0;  
        //***********背景各分量值计算**************************  
        for(int j=0;j<=i;j++) //背景部分各值计算  
        {  
            w1+=Histogram[j];  //背景部分像素点总数  
            u1+=j*Histogram[j]; //背景部分像素总灰度和  
        }  
        if(w1==0) //背景部分像素点数为0时退出  
        {  
            break;  
        }  
        u1=u1/w1; //背景像素平均灰度  
        w1=w1/totalNum; // 背景部分像素点数所占比例  
        //***********背景各分量值计算**************************  
  
        //***********前景各分量值计算**************************  
        for(int k=i+1;k<255;k++)  
        {  
            w0+=Histogram[k];  //前景部分像素点总数  
            u0+=k*Histogram[k]; //前景部分像素总灰度和  
        }  
        if(w0==0) //前景部分像素点数为0时退出  
        {  
            break;  
        }  
        u0=u0/w0; //前景像素平均灰度  
        w0=w0/totalNum; // 前景部分像素点数所占比例  
        //***********前景各分量值计算**************************  
  
        //***********类间方差计算******************************  
        double varValueI=w0*w1*(u1-u0)*(u1-u0); //当前类间方差计算  
        if(varValue<varValueI)  
        {  
            varValue=varValueI;  
            T=i;  
        }  
    }  
    return T;  
}  

void BINARY_IMAGE(unsigned char*img,int cols,int rows,int thredshod)
{
	//dst=new unsigned char[cols*rows];
	for(int i=0;i<rows;i++)
		for(int j=0;j<cols;j++)
		{
			if(img[i*cols+j]>thredshod) img[i*cols+j]=255;
			else img[i*cols+j]=0;
		}
}

void BINARY_IMAGE_TEST(unsigned char*img,int cols,int rows,int thredshod,unsigned char* dst)
{
	//dst=new unsigned char[cols*rows];
	for(int i=0;i<rows;i++)
		for(int j=0;j<cols;j++)
		{
			if(img[i*cols+j]>thredshod) dst[i*cols+j]=255;
			else dst[i*cols+j]=0;
		}
}


void Erosion(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//二值图像膨胀
{
	//res==new unsigned char[nWidth*nHeight];
	int pos,up,upleft,upright,left,right,downleft,down,downright;
	for (int i = 1; i < nHeight - 1; i++)
	{
		for (int j = 1; j < nWidth - 1; j++)
		{
			pos = i*nWidth + j;
			up = pos - nWidth;
			upleft = pos - nWidth - 1;
			upright = pos - nWidth + 1;
			left = pos - 1;
			right = pos + 1;
			downleft = pos + nWidth - 1;
			down = pos + nWidth;
			downright = pos + nWidth + 1;

			(image[pos] || image[up] || image[upleft] || image[upright] || image[left] || image[right] || image[downleft] || image[down] || image[downright]) ? res[pos] = 255 : res[pos] = 0;
		}
	}
	int tmppos;
	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
		    tmppos = i*nWidth + j;
			res[tmppos] = image[tmppos];
		}
	}
	int tmppos1,tmppos2;
	for (int i = 1; i < nHeight - 1; i++)
	{

		tmppos1 = i*nWidth;
		res[tmppos1] = image[tmppos1];

		tmppos2 = (i + 1)*nWidth - 1;
		res[tmppos2] = image[tmppos2];
	}

	for (int i = nHeight - 1; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			tmppos = i*nWidth + j;
			res[tmppos] = image[tmppos];
		}
	}
}

void Dilation(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//二值图像腐蚀
{
	//res==new unsigned char[nWidth*nHeight];
	int pos,up,upleft,upright,left,right,downleft,down,downright;
	for (int i = 1; i < nHeight - 1; i++)
	{
		for (int j = 1; j < nWidth - 1; j++)
		{
			pos = i*nWidth + j;
			up = pos - nWidth;
			upleft = pos - nWidth - 1;
			upright = pos - nWidth + 1;
			left = pos - 1;
			right = pos + 1;
			downleft = pos + nWidth - 1;
			down = pos + nWidth;
			downright = pos + nWidth + 1;

			(image[pos] && image[up] && image[upleft] && image[upright] && image[left] && image[right] && image[downleft] && image[down] && image[downright]) ? res[pos] = 255 : res[pos] = 0;
		}
	}
	int tmppos;
	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			tmppos = i*nWidth + j;
			res[tmppos] = image[tmppos];
		}
	}
	int tmppos1,tmppos2;
	for (int i = 1; i < nHeight - 1; i++)
	{

		tmppos1 = i*nWidth;
		res[tmppos1] = image[tmppos1];

		tmppos2 = (i + 1)*nWidth - 1;
		res[tmppos2] = image[tmppos2];
	}

	for (int i = nHeight - 1; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			tmppos = i*nWidth + j;
			res[tmppos] = image[tmppos];
		}
	}
}

void Closeoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//形态学闭操作
{
	unsigned char* transition = new unsigned char[nHeight*nWidth];
	Erosion(image, transition, nHeight, nWidth);
	Dilation(transition, res, nHeight, nWidth);
	delete[]transition;
}

void Openoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//形态学开操作
{
	unsigned char* transition = new unsigned char[nHeight*nWidth];
	Dilation(image, transition, nHeight, nWidth);
	Erosion(transition, res, nHeight, nWidth);
	delete[]transition;
}


string QRDecoder(unsigned char* img,int cols,int rows)
{
	string result;
	ImageScanner scanner;
	const void *raw = img;
	// configure the reader
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
	// wrap image data
	Image image(cols, rows, "Y800", raw, cols * rows);
	// scan the image for barcodes
	int n = scanner.scan(image);
	// extract results
	result = image.symbol_begin()->get_data();
	image.set_data(NULL, 0);
	return result;
}


string QRDecoder_improve1(unsigned char* img,int cols,int rows,int n) //img为传入灰度图片，不会对其原始数据做修改  
{
	string result=QRDecoder(img,cols,rows);
	static int times=1;
	if(result.empty()&&times>n)
	{
		printf("lib way failed!\n");
		times=0;
		int thre=OtsuAlgThreshold(img,cols,rows);
		unsigned char* dst=new unsigned char[cols*rows];    //dst为不同阈值下，二值化的图像数据，用于再次识别
		unsigned char* dst1=new unsigned char[cols*rows];   //dst1位不同阈值的二值化图像再次进行开运算的图像数据，用于加强识别
		while(result.empty() && thre<255)
		{
			printf("trying thredshold:%d\n",thre);
			BINARY_IMAGE_TEST(img,cols,rows,thre,dst);
			result = QRDecoder(dst,cols,rows);
			if(result.empty())
			{
				Openoperations(dst,dst1,rows,cols);
				result = QRDecoder(dst1,cols,rows);
			}
			thre += 15;//阈值步长设为10，步长越大，识别率越低，速度越快
		}
		delete dst;
		delete dst1;
	}
	times++;
	return result;
}


string QRDecoder_improve2(unsigned char* img,int cols,int rows,int n)
{
	string result=QRDecoder(img,cols,rows);
	static int times=1;
	if(result.empty()&&times>n)
	{
		printf("lib way failed!\n");
		times=0;
		int thre=OtsuAlgThreshold(img,cols,rows);
		int thre2=thre;

		unsigned char* dst=new unsigned char[cols*rows];
		unsigned char* dst1=new unsigned char[cols*rows];
		while(result.empty() && thre<255)
		{
			printf("trying thredshold:%d\n",thre);
			BINARY_IMAGE_TEST(img,cols,rows,thre,dst);
			result = QRDecoder(dst,cols,rows);
			if(result.empty())
			{
				Openoperations(dst,dst1,rows,cols);
				result = QRDecoder(dst1,cols,rows);
			}
			thre += 15;//阈值步长设为10，步长越大，识别率越低，速度越快
		}

		thre2 -= 15;
		while(result.empty() && thre2>0)
		{
			printf("trying thredshold:%d\n",thre2);
			BINARY_IMAGE_TEST(img,cols,rows,thre2,dst);
			result = QRDecoder(dst,cols,rows);
			if(result.empty())
			{
				Openoperations(dst,dst1,rows,cols);
				result = QRDecoder(dst1,cols,rows);
			}
			thre2 -= 15;//阈值步长设为10，步长越大，识别率越低，速度越快
		}
		delete dst;
		delete dst1;
	}
	times++;
	return result;
}


string QRDecoder_improve(unsigned char* img,int cols,int rows,int n,int m) //img为传入灰度图片，不会对其原始数据做修改  
{
	string result=QRDecoder(img,cols,rows);
	static int times=1;
	if(result.empty()&&times>n)
	{
		printf("lib way failed!\n");
		times=0;
		int trytimes=1;
		int thre=OtsuAlgThreshold(img,cols,rows);
		unsigned char* dst=new unsigned char[cols*rows];    //dst为不同阈值下，二值化的图像数据，用于再次识别
		unsigned char* dst1=new unsigned char[cols*rows];   //dst1位不同阈值的二值化图像再次进行开运算的图像数据，用于加强识别
		while(result.empty() && thre<255 && trytimes<m)
		{
			printf("trying thredshold:%d\n",thre);
			BINARY_IMAGE_TEST(img,cols,rows,thre,dst);
			result = QRDecoder(dst,cols,rows);
			if(result.empty())
			{
				Openoperations(dst,dst1,rows,cols);
				result = QRDecoder(dst1,cols,rows);
			}
			thre += 15;//阈值步长设为10，步长越大，识别率越低，速度越快
			trytimes++;
		}
		delete dst;
		delete dst1;
	}
	times++;
	return result;
}



string QRDecoder_improve_ROI(unsigned char* img,int cols,int rows,int n,int m)
{
	string result=QRDecoder(img,cols,rows);
	static int times=1;
	if(result.empty()&&times>n)
	{
		printf("lib way failed!\n");
		times=0;
		int trytimes=1;
		int thre=OtsuAlgThreshold(img,cols,rows);
		unsigned char* dst=new unsigned char[cols*rows];    //dst为不同阈值下，二值化的图像数据，用于再次识别
		unsigned char* dst1=new unsigned char[cols*rows];   //dst1位不同阈值的二值化图像再次进行开运算的图像数据，用于加强识别
		while(result.empty() && thre<255 && trytimes<m)
		{
			printf("trying thredshold:%d\n",thre);
			BINARY_IMAGE_TEST(img,cols,rows,thre,dst);
			result = QRDecoder(dst,cols,rows);
			if(result.empty())
			{
				Openoperations(dst,dst1,rows,cols);
				result = QRDecoder(dst1,cols,rows);
			}
			thre += 15;//阈值步长设为10，步长越大，识别率越低，速度越快
			trytimes++;
		}
		delete dst;
		delete dst1;
	}
	times++;
	return result;
}




