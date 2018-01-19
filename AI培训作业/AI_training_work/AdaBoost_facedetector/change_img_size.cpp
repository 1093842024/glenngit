// change_img_size.cpp : 定义控制台应用程序的入口点。

#include "change_img_size.h"
#include <io.h>
#define DST_IMG_WIDTH 24        //需要调整图片后的尺寸宽度
#define SRC_IMG_HEIGH 24        //需要调整图片后的尺寸高度

int changeimgsize_pos()
{
	Mat src_img;
	int i;
	string src_img_name="yale/",dst_img_name="yale_small_size/";//源文件和目的文件的文件夹名字
	char chari[5];
	for(i=1;i<=165;i++)
	{

			itoa(i,chari,10);//将变量转换成字符型，此处的chari是字符数组首地址，但是如果定义为char *chari="";则会出现错误，why？
			
			src_img_name+="s";
			src_img_name+=chari;
			src_img_name+=".bmp";

			src_img=imread(src_img_name,1);
			imshow("face",src_img);
			waitKey(0);
			Mat dst_img_rsize(DST_IMG_WIDTH,SRC_IMG_HEIGH,src_img.type());
			resize(src_img,dst_img_rsize,dst_img_rsize.size(),0,0,INTER_LINEAR);

			dst_img_name+="s";
			dst_img_name+=chari;
			dst_img_name+="n.bmp";

			imwrite(dst_img_name,dst_img_rsize);
			src_img_name="yale/",dst_img_name="yale_small_size/";//每次循环后要重新清0字符数组内的内容，目的文件夹一定要事先建立，否则无效果

		
	}    
	return 0;
}


int changeimgsize_neg()
{
	Mat src_img;
	string src_img_name="neg/",dst_img_name="neg_img/";//源文件和目的文件的文件夹名字

	string path="neg";
	vector<string> filename;
	getFiles(path,filename);

	for(vector<string>::iterator it=filename.begin();it!=filename.end();it++)
	{
		cout<<(*it).c_str()<<endl;
		
		src_img_name+=(*it);

		src_img=imread(src_img_name,1);
		imshow("face",src_img);
		waitKey(0);
		Mat dst_img_rsize(DST_IMG_WIDTH,SRC_IMG_HEIGH,src_img.type());
		resize(src_img,dst_img_rsize,dst_img_rsize.size(),0,0,INTER_LINEAR);

        dst_img_name+="n";
		dst_img_name+=(*it);

		imwrite(dst_img_name,dst_img_rsize);
		src_img_name="neg/",dst_img_name="neg_img/";//每次循环后要重新清0字符数组内的内容，目的文件夹一定要事先建立，否则无效果


	}    
	return 0;
}

void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息，声明一个存储文件信息的结构体  
	struct _finddata_t fileinfo;
	string p;//字符串，存放路径
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)//若查找成功，则进入
	{
		do
		{
			//如果是目录,迭代之（即文件夹内还有文件夹）  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				/*
				//文件名不等于"."&&文件名不等于".."
				//.表示当前目录
				//..表示当前目录的父目录
				//判断时，两者都要忽略，不然就无限递归跳不出去了！
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
					*/
			}
			//如果不是,加入列表  
			else
			{
				//files.push_back(p.assign(path).append("\\").append(fileinfo.name));  //包含了路径信息的文件名
				files.push_back(fileinfo.name);      //只包含纯文件名
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		//_findclose函数结束查找
		_findclose(hFile);
	}
}







//从背景图片中随机抽取图像块，多用于生成负样本
#define kImageBlockWidth                50        //图像块大小
#define kImageBlockHeight                50        
#define kLoopTimes                        5    //期望样本数

int createimg()
{
	Mat src_img;
	string src_img_name="neg/";//源文件和目的文件的文件夹名字

	string path="neg";
	vector<string> filename;
	getFiles(path,filename);
	int filek=0;
	for(vector<string>::iterator it=filename.begin();it!=filename.end();it++)
	{
		cout<<(*it).c_str()<<endl;

		src_img_name+=(*it);

	int originX = 0, originY = 0;
	int width_limited = 0, height_limited = 0;
	int width = 0, height = 0;
	IplImage *bgImage = cvLoadImage(src_img_name.c_str());
	IplImage *blockImage = cvCreateImage(cvSize(kImageBlockWidth, kImageBlockHeight), bgImage->depth, bgImage->nChannels);
	width = bgImage->width;
	height = bgImage->height;
	width_limited = width - kImageBlockWidth;
	height_limited = height - kImageBlockHeight;
	cout<<width_limited<<"   "<<height_limited;
	for (int i = 0; i < kLoopTimes; i++)
	{
		originX = rand() % width_limited;
		originY = rand() % height_limited;
		cvZero(blockImage);
		CvPoint2D32f center_block = cvPoint2D32f(originX + kImageBlockWidth / 2, originY + kImageBlockHeight / 2);
		cvGetRectSubPix(bgImage, blockImage, center_block);
		char saveFileName[100] = {'\0'};
		sprintf(saveFileName, "neg\\(%d).bmp", i + filek+1);
		cvSaveImage(saveFileName, blockImage);
	}

	  cvReleaseImage(&bgImage);
	  cvReleaseImage(&blockImage);
	  filek=filek+kLoopTimes;
	  src_img_name="neg/";
	}

	system("pause");
	return 0;
}



