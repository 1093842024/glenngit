#include "zbar.h"        
#include "cv.h"        
#include "highgui.h"        
#include <iostream>        
#include "QRdecoder.h"
#include <fstream>

using namespace std;        
using namespace zbar;  //添加zbar名称空间      
using namespace cv;        

string ZbarDecoder(Mat img);
string GetQRInBinImg(Mat binImg);

#define LIBTESTPIC
//#define LIBTESTVIDEO
//#define PIC
//#define VIDEO




int main(int argc,char*argv[])      
{  

#ifdef LIBTESTPIC
	//读入图像的格式大小
	string imagename="640480.jpg";
	Mat image = imread(imagename);    //即使图像是灰色的，读出来还是三通道的图像
	if(!image.data)  {  
		cout<<"请确认图片"<<endl;  system("pause");  return 0;  
	}  
	/*Mat change;
	Size dsize1=Size(640,480);
	resize(image,change,dsize1);
	imwrite("640480.jpg",change);
	imshow("ca",change);
	waitKey(0);*/


	int width = image.cols;        
	int height = image.rows; 
	
	printf("%d,%d,%d\n",width,height,image.channels());
	/*uchar *raw = (uchar *)image.data;*/

	//创建识别的图像和显示的图像
	Mat imageGray;        
	cvtColor(image,imageGray,CV_RGB2GRAY); 
	Mat frame;
	double scale=1;      
	Size dsize=Size(imageGray.cols*scale,imageGray.rows*scale);
	frame=Mat(dsize,CV_32S);

	
	string img_path ="QRdecode_sample_vga/distance_20cm/";
	char img_name[128];
	int img_idx = 1;
	int suc0=0,suc1=0;
	double t0=0,t1=0;

	while(1)
	{
		//写入图片路径
		
        FILE* pfile = NULL; char path[120] = {0x0};
	    //sprintf_s(img_name, "%02d.yuv", img_idx++);
		sprintf_s(img_name, "gray_picture%d.yuv", img_idx++);
	    string img_full_name = img_path + + img_name;
	    sprintf(path,img_full_name.c_str());
	    /*pfile = fopen(path, "w+"); if(pfile == NULL) perror("open file");
	    cout<<fwrite(raw, 1, width*height, pfile)<<endl;
	    cout<<"approach 3:"<<QRDecoder_improve1(raw,width,height)<<endl;
	    fflush(pfile);
	    fclose(pfile);*/
		
		//读取文件内容
        uchar* gray = NULL;  gray = (uchar*)malloc(width*height); if(gray == NULL) perror("malloc");
	    pfile = fopen(path, "rb"); 
	    if(pfile == NULL) break;
	//cout<<
		fread(gray,1, width*height, pfile);
	//	<<endl;

        printf("***************FRAME %d\n",img_idx-1);
		//显示文件内容为图片
		for(int i=0;i<width*height;i++)
			imageGray.data[i]=gray[i];
		resize(imageGray,frame,dsize);
		imshow("pic",frame);
		waitKey(100);

		//进行识别
		double t=(double)cvGetTickCount();
		//string result0=QRDecoder(gray,width,height);
		string result0=QRDecoder(frame.data,frame.cols,frame.rows);
		t0+=(double)cvGetTickCount()-t;
		t=(double)cvGetTickCount();
		//string result1=QRDecoder_improve(gray,width,height,0);
		string result1=QRDecoder_improve(frame.data,frame.cols,frame.rows,0,2);
		t1+=(double)cvGetTickCount()-t;
	    cout<<"approach 0:"<<result0<<endl;
	    cout<<"approach 1:"<<result1<<endl;
        fclose(pfile);
	    free(gray);
		if(!result0.empty()) suc0++;
		if(!result1.empty()) suc1++;
		waitKey(10);
	}
	img_idx--;img_idx--;
	printf("suc0:%.1f%%,suc1:%.1f%%\n",suc0*100.0/img_idx,suc1*100.0/img_idx);
	printf("aver time0:%.1fms,avertime1:%.1fms\n",t0/ ((double)cvGetTickFrequency() * 1000*img_idx),t1/ ((double)cvGetTickFrequency() * 1000*img_idx));

	/*
	Mat binImg;
	int thre = threshold(imageGray, binImg, 0, 255, cv::THRESH_OTSU); 
	int thre1=OtsuAlgThreshold(raw,width,height);
	cout<<"opencv ostu:"<<thre<<",my ostu:"<<thre1<<endl;
	
    Mat openImg;
	threshold(imageGray, binImg, thre, 255, cv::THRESH_BINARY);
	imshow("opencv binary",binImg);
	waitKey(100);
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(binImg, openImg, MORPH_OPEN, element);
	imshow("opencv open",openImg);

	BINARY_IMAGE_TEST(raw,width,height,thre1,binImg.data);
	imshow("my binary",binImg);
	waitKey(100);
	Openoperations(binImg.data,openImg.data,height,width);
	imshow("my open",openImg);
	waitKey(0);
	*/

	


#endif

#ifdef LIBTESTVIDEO
	VideoCapture cap( 
		//"video1/90_5.0.mp4"
		//"video1/90_6.0.mp4"
		//"video1/90_8.9.mp4"
		//"video/120_5.0.mp4"
		//"video1/120_5.5.mp4"
		//"video1/120_8.9.mp4"
		//"video1/180_5.0.mp4"
		//"video1/180_5.5.mp4"
		//"video1/180_8.9.mp4"
		"video/test.mp4"
		);
	if (!cap.isOpened()){
		printf("video open failed!"); 
		cv::waitKey(0);
		return -1;
	}
	Mat frameorigin;
	Mat frame;
	Mat imageGray;  
	int framenum=1;
	int successnum=1;
	int successnum1=1;
	string resultlib;
	int k=0;
	double scale=1;      //缩小图像尺寸会提高识别速度，但会降低识别精度

	cap >>frameorigin;
	if (frameorigin.empty()) { return -1;}
	Size dsize=Size(frameorigin.cols*scale,frameorigin.rows*scale);
	frame=Mat(dsize,CV_32S);

	while (1)
	{
		k++;
		cap >>frameorigin;
		if (frameorigin.empty()) { break;}
		if(k%5!=0) continue;
		resize(frameorigin,frame,dsize);

		printf("\n**********FRAME:%d,success:%d,success1:%d,successrate:%.1f%%,successrate1:%.1f%%\n"
			,framenum,successnum,successnum1,successnum*100.0/framenum,successnum1*100.0/framenum);
		framenum++;
		successnum++;
		successnum1++;
		cvtColor(frame,imageGray,CV_RGB2GRAY); 
		int width = imageGray.cols;        
		int height = imageGray.rows;        
		uchar *raw = (uchar *)imageGray.data;  

		resultlib=QRDecoder(raw,width,height);
		if(!resultlib.empty())
		{
             cout<<"way 0: "<<resultlib<<endl;
		}
		else
		{
			cout<<"way 0 failed! "<<endl;
			successnum--;
		}

		resultlib.erase();
		resultlib=QRDecoder_improve1(raw,width,height,0);
		if(!resultlib.empty())
		{
			cout<<"way 3: "<<resultlib<<endl;
		}
		else
		{
			cout<<"way 3 failed! "<<endl;
			successnum1--;
		}
		 uchar c=waitKey(10);
		 if(c==' ')waitKey(0);
		
		
		imshow("video",frame);
		waitKey(10);
        //if(framenum==1117) waitKey(0);
	}
#endif



#ifdef PIC
	Mat image = imread("code8.jpg");    
	if(!image.data)  
	{  
		cout<<"请确认图片"<<endl;  
		system("pause");  
		return 0;  
	}  
	Mat imageGray;        
	cvtColor(image,imageGray,CV_RGB2GRAY); 
	int width = imageGray.cols;        
	int height = imageGray.rows;        
	uchar *raw = (uchar *)imageGray.data;  

	ImageScanner scanner;                               //图像扫描类创建
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);  //配置扫描类
	Image imageZbar(width, height, "Y800", raw, width * height);    //交换图像数据给imageZbar         
	scanner.scan(imageZbar);                                        //扫描条码      
	Image::SymbolIterator symbol = imageZbar.symbol_begin();        //提取结果 symbol
	if(imageZbar.symbol_begin()==imageZbar.symbol_end())    
	{    
		cout<<"查询条码失败，请检查图片！"<<endl;    
	}    
	for(;symbol != imageZbar.symbol_end();++symbol)      
	{        
		cout<<"类型："<<endl<<symbol->get_type_name()<<endl<<endl;      
		cout<<"条码："<<endl<<symbol->get_data()<<endl<<endl;   
		if (symbol->get_location_size() == 4) {
			//rectangle(frame, Rect(symbol->get_location_x(i), symbol->get_location_y(i), 10, 10), Scalar(0, 255, 0));
			line(image, Point(symbol->get_location_x(0), symbol->get_location_y(0)), Point(symbol->get_location_x(1), symbol->get_location_y(1)), Scalar(0, 255, 0), 2, 8, 0);
			line(image, Point(symbol->get_location_x(1), symbol->get_location_y(1)), Point(symbol->get_location_x(2), symbol->get_location_y(2)), Scalar(0, 255, 0), 2, 8, 0);
			line(image, Point(symbol->get_location_x(2), symbol->get_location_y(2)), Point(symbol->get_location_x(3), symbol->get_location_y(3)), Scalar(0, 255, 0), 2, 8, 0);
			line(image, Point(symbol->get_location_x(3), symbol->get_location_y(3)), Point(symbol->get_location_x(0), symbol->get_location_y(0)), Scalar(0, 255, 0), 2, 8, 0);
		}
	}        
	imshow("Source Image",image); 
	imageZbar.set_data(NULL,0);    //清空数据
	waitKey(500);    
	 
	/************************************************************************/
	/* 对于方法二和方法三，方法二一般情况识别所用的次数会比方法三少，但是方法三对于极个别的情况识别概率会高于方法三，因为方法二试过了所有的阈值，因此对于最差的情况可以借鉴方法三                                                                     */
	/* 因此识别策略可以设计如下：
	/*        首先用方法一，直接输入灰度图像，让zbar进行识别
	/*        如果方法一无法识别，然后用方法二，向下减少阈值，让zbar进行识别
	/*        如果方法二无法识别，最后利用方法二生成的阈值，向上增加，让zbar进行识别
	/************************************************************************/
	Mat binImg;
	//在otsu二值结果的基础上，不断增加阈值，用于识别模糊图像
	int thre = threshold(imageGray, binImg, 0, 255, cv::THRESH_OTSU); 
	cout<<"opencv ostu:"<<thre<<",my ostu:"<<OtsuAlgThreshold(raw,width,height)<<endl;
	string result; int num=0;
	while(result.empty() && thre>=0&&thre<255)
	{
		printf("trying thredshold:%d\n",thre);
		threshold(imageGray, binImg, thre, 255, cv::THRESH_BINARY);
		imshow("gray ostu",binImg);
		result = GetQRInBinImg(binImg);
		thre -= 10;//阈值步长设为20，步长越大，识别率越低，速度越快
		num++;
	}
	cout<<"方法二条码："<<" times:"<<num<<endl<<result<<endl<<endl;  
	waitKey(500);  
	/*threshold( src_gray, dst, threshold_value, max_BINARY_value,threshold_type );
	  函数 threshold<> 会接受到5个参数：
                 src_gray: 输入的灰度图像的地址。
                 dst: 输出图像的地址。
                 threshold_value: 进行阈值操作时阈值的大小。
                 max_BINARY_value: 设定的最大灰度值（该参数运用在二进制与反二进制阈值操作中）。
                 threshold_type: 阈值的类型。从上面提到的5种中选择出的结果。*/
	
	int thre1=0;
	string result1;int num1=0;
	while(result1.empty() && thre1>=0&&thre1<255)
	{
		printf("trying thredshold:%d\n",thre1);
		threshold(imageGray, binImg, thre1, 255, cv::THRESH_BINARY);
        imshow("gray ostu1",binImg);
		result1 = GetQRInBinImg(binImg);
		thre1 += 10;//阈值步长设为20，步长越大，识别率越低，速度越快
		num1++;
	}
	cout<<"方法三条码："<<" times:"<<num1<<endl<<result1<<endl<<endl;  
	waitKey(500); 
#endif

#ifdef VIDEO
	VideoCapture cap( 
		//"video/90_5.0.mp4"
		//"video/90_5.5.mp4"
		//"video/90_8.9.mp4"
		"video/120_5.0.mp4"
		//"video/120_5.5.mp4"
		//"video/120_8.9.mp4"
		//"video/180_5.0.mp4"
		//"video/180_5.5.mp4"
		//"video/180_8.9.mp4"
		);
	if (!cap.isOpened()){
		printf("video open failed!"); 
		cv::waitKey(0);
		return -1;
	}
	Mat frame;
    Mat imageGray;  
	int framenum=1;
	int successnum=1;
	int fail1=0,fail2=0,fail3=0;

	while (1)
	{
		cap >>frame;
		if (frame.empty()) { break;}
        printf("\n**********FRAME:%d,success:%d%%,successrate:%.1f%%,fail1:%.1f%%,fail2:%.1f%%\n",framenum,successnum,successnum*100.0/framenum,fail1,fail2);
		framenum++;
		successnum++;
		cvtColor(frame,imageGray,CV_RGB2GRAY); 
		int width = imageGray.cols;        
		int height = imageGray.rows;        
		uchar *raw = (uchar *)imageGray.data;  

		ImageScanner scanner;                               //图像扫描类创建
		scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);  //配置扫描类
		Image imageZbar(width, height, "Y800", raw, width * height);    //交换图像数据给imageZbar         
		scanner.scan(imageZbar);                                        //扫描条码      
		Image::SymbolIterator symbol = imageZbar.symbol_begin();        //提取结果 symbol
		if(imageZbar.symbol_begin()==imageZbar.symbol_end())    
		{    
			cout<<"查询条码失败，请检查图片！"<<endl;    
		}    
		for(;symbol != imageZbar.symbol_end();++symbol)      
		{        
			cout<<"类型：  "<<symbol->get_type_name()<<"   ";      
			cout<<"条码：  "<<symbol->get_data()<<endl;   
			if (symbol->get_location_size() == 4) {
				//rectangle(frame, Rect(symbol->get_location_x(i), symbol->get_location_y(i), 10, 10), Scalar(0, 255, 0));
				line(frame, Point(symbol->get_location_x(0), symbol->get_location_y(0)), Point(symbol->get_location_x(1), symbol->get_location_y(1)), Scalar(0, 255, 0), 2, 8, 0);
				line(frame, Point(symbol->get_location_x(1), symbol->get_location_y(1)), Point(symbol->get_location_x(2), symbol->get_location_y(2)), Scalar(0, 255, 0), 2, 8, 0);
				line(frame, Point(symbol->get_location_x(2), symbol->get_location_y(2)), Point(symbol->get_location_x(3), symbol->get_location_y(3)), Scalar(0, 255, 0), 2, 8, 0);
				line(frame, Point(symbol->get_location_x(3), symbol->get_location_y(3)), Point(symbol->get_location_x(0), symbol->get_location_y(0)), Scalar(0, 255, 0), 2, 8, 0);
			}
		}        
		imshow("video",frame); 
		imageZbar.set_data(NULL,0);    //清空数据
		waitKey(10);
		//waitKey(0);

		if(imageZbar.symbol_begin()==imageZbar.symbol_end())
		{
			printf("方法一失效，尝试方法二！\n");
			fail1++;
			Mat binImg;
			//在otsu二值结果的基础上，不断增加阈值，用于识别模糊图像
			int thre = threshold(imageGray, binImg, 0, 255, cv::THRESH_OTSU); 
			int thre1=thre;
			string result; int num=0;
			while(result.empty() && thre<255)
			{
				printf("trying thredshold:%d\n",thre);
				threshold(imageGray, binImg, thre, 255, cv::THRESH_BINARY);
				imshow("approach2",binImg);
				result = GetQRInBinImg(binImg);
				thre += 10;//阈值步长设为20，步长越大，识别率越低，速度越快
				num++;
			}
			 waitKey(10); 

			if(!result.empty())
			{
                cout<<"方法二条码："<<" times:"<<num<<endl<<result<<endl<<endl;  
			    waitKey(10);
				destroyWindow("approach2");
			}else{
				fail2++;
				destroyWindow("approach2");
				printf("方法二失效，次数：%d！\n",num);
				printf("尝试方法三！\n");
				string result1;int num1=0;thre1 -= 10;
				while(result1.empty() &&thre1>=0)
				{
					printf("trying thredshold:%d\n",thre1);
					threshold(imageGray, binImg, thre1, 255, cv::THRESH_BINARY);
					imshow("approach3",binImg);
					result1 = GetQRInBinImg(binImg);
					thre1 -= 10;//阈值步长设为20，步长越大，识别率越低，速度越快
					num1++;
				}
				 waitKey(10); 

				if(!result1.empty())
				{
                    cout<<"方法三条码："<<" times:"<<num1<<endl<<result1<<endl<<endl;  
				    waitKey(10); 
					destroyWindow("approach3");
				}else{
					fail3++;
					destroyWindow("approach3");
					printf("方法三失效，次数：%d！\n",num1);
					printf("所有方法失效，请检查图片！\n");
					successnum--;
				}
			}
		}
		imshow("video",frame);
	}
	printf("all sucessrate:%.1f%%,1 failrate:%.1f%%, 2 failrate:%.1f%%,3 failrate:%.1f%%\n",successnum*100.0/framenum
		,fail1*100.0/framenum,fail2*100.0/framenum,fail3*100.0/framenum);

#endif

	system("pause");
	return 0;    
}  


string ZbarDecoder(Mat img)
{
	string result;
	ImageScanner scanner;
	const void *raw = (&img)->data;
	// configure the reader
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
	// wrap image data
	Image image(img.cols, img.rows, "Y800", raw, img.cols * img.rows);
	// scan the image for barcodes
	int n = scanner.scan(image);
	// extract results
	result = image.symbol_begin()->get_data();
	image.set_data(NULL, 0);
	return result;
}

//对二值图像进行识别，如果失败则开运算进行二次识别
string GetQRInBinImg(Mat binImg)
{
	string result = ZbarDecoder(binImg);
	if(result.empty())
	{
		Mat openImg;
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		morphologyEx(binImg, openImg, MORPH_OPEN, element);
		result = ZbarDecoder(openImg);
	}
	return result;
}





/*
int main(int argc, char **argv) {
    int cam_idx = 0;

    if (argc == 2) {
        cam_idx = atoi(argv[1]);
    }

    VideoCapture cap(cam_idx);
    if (!cap.isOpened()) {
        cerr << "Could not open camera." << endl;
        exit(EXIT_FAILURE);
    }
    //cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    namedWindow("captured", CV_WINDOW_AUTOSIZE);
    
    // Create a zbar reader
    ImageScanner scanner;
    
    // Configure the reader
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    for (;;) {
        // Capture an OpenCV frame
        cv::Mat frame, frame_grayscale;
        cap >> frame;

        // Convert to grayscale
        cvtColor(frame, frame_grayscale, CV_BGR2GRAY);

        // Obtain image data
        int width = frame_grayscale.cols;
        int height = frame_grayscale.rows;
        uchar *raw = (uchar *)(frame_grayscale.data);

        // Wrap image data
        Image image(width, height, "Y800", raw, width * height);

        // Scan the image for barcodes
        //int n = scanner.scan(image);
        scanner.scan(image);

        // Extract results
        int counter = 0;
        for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
            time_t now;
            tm *current;
            now = time(0);
            current = localtime(&now);

            // do something useful with results
            cout    << "[" << current->tm_hour << ":" << current->tm_min << ":" << setw(2) << setfill('0') << current->tm_sec << "] " << counter << " "
                    << "decoded " << symbol->get_type_name()
                    << " symbol \"" << symbol->get_data() << '"' << endl;

            //cout << "Location: (" << symbol->get_location_x(0) << "," << symbol->get_location_y(0) << ")" << endl;
            //cout << "Size: " << symbol->get_location_size() << endl;
            
            // Draw location of the symbols found
            if (symbol->get_location_size() == 4) {
                //rectangle(frame, Rect(symbol->get_location_x(i), symbol->get_location_y(i), 10, 10), Scalar(0, 255, 0));
                line(frame, Point(symbol->get_location_x(0), symbol->get_location_y(0)), Point(symbol->get_location_x(1), symbol->get_location_y(1)), Scalar(0, 255, 0), 2, 8, 0);
                line(frame, Point(symbol->get_location_x(1), symbol->get_location_y(1)), Point(symbol->get_location_x(2), symbol->get_location_y(2)), Scalar(0, 255, 0), 2, 8, 0);
                line(frame, Point(symbol->get_location_x(2), symbol->get_location_y(2)), Point(symbol->get_location_x(3), symbol->get_location_y(3)), Scalar(0, 255, 0), 2, 8, 0);
                line(frame, Point(symbol->get_location_x(3), symbol->get_location_y(3)), Point(symbol->get_location_x(0), symbol->get_location_y(0)), Scalar(0, 255, 0), 2, 8, 0);
            }
            
            // Get points
            for (Symbol::PointIterator point = symbol.point_begin(); point != symbol.point_end(); ++point) {
                cout << point << endl;
            } 
            counter++;
        }

        // Show captured frame, now with overlays!
        imshow("captured", frame);
                                                                                                                                                          
        // clean up
        image.set_data(NULL, 0);
        
        waitKey(30);
    }

    return 0;
}


*/