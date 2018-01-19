#include "QRdecoder.h"
#include "zbar.h"  
using namespace zbar;  //���zbar���ƿռ�   

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

    int T=0; //Otsu�㷨��ֵ  
    double varValue=0; //��䷽���м�ֵ����  
    double w0=0; //ǰ�����ص�����ռ����  
    double w1=0; //�������ص�����ռ����  
    double u0=0; //ǰ��ƽ���Ҷ�  
    double u1=0; //����ƽ���Ҷ�  
    double Histogram[256]={0}; //�Ҷ�ֱ��ͼ���±��ǻҶ�ֵ�����������ǻҶ�ֵ��Ӧ�����ص�����  
    unsigned char *data=image.data;  
    double totalNum=image.rows*image.cols; //��������  
    //����Ҷ�ֱ��ͼ�ֲ���Histogram�����±��ǻҶ�ֵ�����������ǻҶ�ֵ��Ӧ���ص���  
    for(int i=0;i<image.rows;i++)   //Ϊ������������û�а�rows��cols���������  
    {  
        for(int j=0;j<image.cols;j++)  
        {  
            Histogram[data[i*image.cols+j]]++;  
        }  
    }  
    for(int i=0;i<255;i++)  
    {  
        //ÿ�α���֮ǰ��ʼ��������  
        w1=0;       u1=0;       w0=0;       u0=0;  
        //***********����������ֵ����**************************  
        for(int j=0;j<=i;j++) //�������ָ�ֵ����  
        {  
            w1+=Histogram[j];  //�����������ص�����  
            u1+=j*Histogram[j]; //�������������ܻҶȺ�  
        }  
        if(w1==0) //�����������ص���Ϊ0ʱ�˳�  
        {  
            break;  
        }  
        u1=u1/w1; //��������ƽ���Ҷ�  
        w1=w1/totalNum; // �����������ص�����ռ����  
        //***********����������ֵ����**************************  
  
        //***********ǰ��������ֵ����**************************  
        for(int k=i+1;k<255;k++)  
        {  
            w0+=Histogram[k];  //ǰ���������ص�����  
            u0+=k*Histogram[k]; //ǰ�����������ܻҶȺ�  
        }  
        if(w0==0) //ǰ���������ص���Ϊ0ʱ�˳�  
        {  
            break;  
        }  
        u0=u0/w0; //ǰ������ƽ���Ҷ�  
        w0=w0/totalNum; // ǰ���������ص�����ռ����  
        //***********ǰ��������ֵ����**************************  
  
        //***********��䷽�����******************************  
        double varValueI=w0*w1*(u1-u0)*(u1-u0); //��ǰ��䷽�����  
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


void Erosion(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//��ֵͼ������
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

void Dilation(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//��ֵͼ��ʴ
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

void Closeoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//��̬ѧ�ղ���
{
	unsigned char* transition = new unsigned char[nHeight*nWidth];
	Erosion(image, transition, nHeight, nWidth);
	Dilation(transition, res, nHeight, nWidth);
	delete[]transition;
}

void Openoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth)//��̬ѧ������
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


string QRDecoder_improve1(unsigned char* img,int cols,int rows,int n) //imgΪ����Ҷ�ͼƬ���������ԭʼ�������޸�  
{
	string result=QRDecoder(img,cols,rows);
	static int times=1;
	if(result.empty()&&times>n)
	{
		printf("lib way failed!\n");
		times=0;
		int thre=OtsuAlgThreshold(img,cols,rows);
		unsigned char* dst=new unsigned char[cols*rows];    //dstΪ��ͬ��ֵ�£���ֵ����ͼ�����ݣ������ٴ�ʶ��
		unsigned char* dst1=new unsigned char[cols*rows];   //dst1λ��ͬ��ֵ�Ķ�ֵ��ͼ���ٴν��п������ͼ�����ݣ����ڼ�ǿʶ��
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
			thre += 15;//��ֵ������Ϊ10������Խ��ʶ����Խ�ͣ��ٶ�Խ��
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
			thre += 15;//��ֵ������Ϊ10������Խ��ʶ����Խ�ͣ��ٶ�Խ��
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
			thre2 -= 15;//��ֵ������Ϊ10������Խ��ʶ����Խ�ͣ��ٶ�Խ��
		}
		delete dst;
		delete dst1;
	}
	times++;
	return result;
}


string QRDecoder_improve(unsigned char* img,int cols,int rows,int n,int m) //imgΪ����Ҷ�ͼƬ���������ԭʼ�������޸�  
{
	string result=QRDecoder(img,cols,rows);
	static int times=1;
	if(result.empty()&&times>n)
	{
		printf("lib way failed!\n");
		times=0;
		int trytimes=1;
		int thre=OtsuAlgThreshold(img,cols,rows);
		unsigned char* dst=new unsigned char[cols*rows];    //dstΪ��ͬ��ֵ�£���ֵ����ͼ�����ݣ������ٴ�ʶ��
		unsigned char* dst1=new unsigned char[cols*rows];   //dst1λ��ͬ��ֵ�Ķ�ֵ��ͼ���ٴν��п������ͼ�����ݣ����ڼ�ǿʶ��
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
			thre += 15;//��ֵ������Ϊ10������Խ��ʶ����Խ�ͣ��ٶ�Խ��
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
		unsigned char* dst=new unsigned char[cols*rows];    //dstΪ��ͬ��ֵ�£���ֵ����ͼ�����ݣ������ٴ�ʶ��
		unsigned char* dst1=new unsigned char[cols*rows];   //dst1λ��ͬ��ֵ�Ķ�ֵ��ͼ���ٴν��п������ͼ�����ݣ����ڼ�ǿʶ��
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
			thre += 15;//��ֵ������Ϊ10������Խ��ʶ����Խ�ͣ��ٶ�Խ��
			trytimes++;
		}
		delete dst;
		delete dst1;
	}
	times++;
	return result;
}




