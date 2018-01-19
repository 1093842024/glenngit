#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
using namespace std;
using namespace cv;

//拼接视频
/*
int main()
{

    //Mat depth = imread("F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/warp-depth-f000.png");
    //Mat image = imread("F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/warp-img-f000.png");
	VideoCapture cap1("../05_5.avi");
	if (!cap1.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}
	Mat depth;
	cap1>>depth;
	VideoCapture cap2("../05_6.avi");
	if (!cap2.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}
	Mat image;
	cap2>>image;
	double fps=cap2.get(CV_CAP_PROP_FPS);
    int liv_width = depth.cols + image.cols;
    int liv_height = image.rows;
    int liv_nChannels = image.channels();
    VideoWriter videoWriter("5_3.avi",CV_FOURCC('M','J','P','G'),fps,Size(liv_width,liv_height));
    //IplImage *lpv_firstFrame1 = cvLoadImage("F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/final-depth-f000.png");
    //IplImage *lpv_firstFrame2 = cvLoadImage("F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/final-img-f000.png");
        //int liv_width = lpv_firstFrame1->width+lpv_firstFrame2->width;
        //int liv_height = lpv_firstFrame2->height;
        //int liv_nChannels = lpv_firstFrame2->nChannels;

        // 初始化CvVideoWriter
        //CvVideoWriter* videoWriter = cvCreateVideoWriter("final-result.avi",CV_FOURCC('X', 'V', 'I', 'D'),    20, cvSize(liv_width, liv_height),1);
        //IplImage *lpv_imageFrame1;
        //IplImage *lpv_imageFrame2;
   // Mat image1;
   // Mat image2;



        //用于表示处于视频的多少帧
        int i = 0;  
        char imageDir1[256], imageDir2[256];    //略大于读入文件目录的总字符数，防止因为'\0'而出现栈溢出的情况
        //sprintf(imageDir1, "F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/final-depth-f%03d.png", i);
        sprintf(imageDir2, "F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/final-image-f%03d.png", i);//
        //image1 = imread(imageDir1);
        //image2 = imread(imageDir2);
        //while ((lpv_imageFrame1 = cvLoadImage(imageDir1))&&(lpv_imageFrame2=cvLoadImage(imageDir2)))
        //while (image1.empty()&&image2.empty())
        while (!(depth.empty() && image.empty()))
        {

            Rect rectd = Rect(0, 0, depth.cols, depth.rows);
            Rect recti = Rect(depth.cols, 0, image.cols, image.rows);

            Mat dstImage;
            dstImage.create(Size(liv_width,liv_height),image.type());


            //image1.copyTo(Mat(dstImage, rectd));
            //image2.copyTo(Mat(dstImage, recti));
            depth.copyTo(Mat(dstImage, rectd));
            image.copyTo(Mat(dstImage, recti));
            videoWriter << dstImage;
            i++;
			cap1>>depth;
			cap2>>image;
            //sprintf(imageDir1, "F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/warp-depth-f%03d.png", i);
            //sprintf(imageDir2, "F:/3DImageProcess/MSProjection/MSProjection/MSProjection/results/cam7/warp-img-f%03d.png", i);
           // depth = imread(imageDir1);
            //image = imread(imageDir2);


 


        }

 

    return 0;
}
*/