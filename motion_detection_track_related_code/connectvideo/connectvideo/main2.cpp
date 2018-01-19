#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
using namespace std;
using namespace cv;

#define RECORDVIDEO

int main(int argc, char *argv[])  
{  
 
    VideoCapture cap("../5.avi");
	if (!cap.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}
	Mat frameorigin;
	cap>>frameorigin;

    cv::namedWindow("original image");
	cv::imshow("original image", frameorigin);
	waitKey(0);   //不加这句，无法显示出图像，因为图像还未加载就进行了下一句

	float scale=0.7;
	Mat frame(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
	cv::resize(frameorigin,frame,frame.size());

#ifdef RECORDVIDEO
	double fps=cap.get(CV_CAP_PROP_FPS);
	cv::Size size=frame.size();
	int count=cap.get(CV_CAP_PROP_FRAME_COUNT);
	cv::VideoWriter writevideo("../5.1.avi",CV_FOURCC('M','J','P','G'), fps, size);
#endif
	
 
 //读取和显示  
 while(1)  
 {  
	 cap>>frameorigin;
	 if (frameorigin.empty()) break;
	 // stop the program if no more images
	 if (frameorigin.rows == 0 || frameorigin.cols == 0)
		 break;

	 Mat frame(frameorigin.rows*scale,frameorigin.cols*scale,frameorigin.type());
	 cv::resize(frameorigin,frame,frame.size());

  imshow("tracker", frame);
  printf("changing!\n");

#ifdef RECORDVIDEO
  writevideo<<frame;
#endif

  if (waitKey(1) == 27)   //不加这句，无法显示出图像，因为图像还未加载就进行了下一句
	  break;
  
 } 



 return 0;  
} 




/*
void average(const Mat &img, Point_<int> a, Point_<int> b, Vec3b &p)
{

    const Vec3b *pix;
    Vec3i temp;
    for (int i = a.x; i <= b.x; i++){
        pix = img.ptr<Vec3b>(i);
        for (int j = a.y; j <= b.y; j++){
            temp[0] += pix[j][0];
            temp[1] += pix[j][1];
            temp[2] += pix[j][2];
        }
    }

    int count = (b.x - a.x + 1) * (b.y - a.y + 1);
    p[0] = temp[0] / count;
    p[1] = temp[1] / count;
    p[2] = temp[2] / count;
}


void scalePartAverage(const Mat &src, Mat &dst, double xRatio, double yRatio)
{
    int rows = static_cast<int>(src.rows * xRatio);
    int cols = static_cast<int>(src.cols * yRatio);

    dst.create(rows, cols, src.type());

    int lastRow = 0;
    int lastCol = 0;

    Vec3b *p;
    for (int i = 0; i < rows; i++) {
        p = dst.ptr<Vec3b>(i);
        int row = static_cast<int>((i + 1) / xRatio + 0.5) - 1;

        for (int j = 0; j < cols; j++) {
            int col = static_cast<int>((j + 1) / yRatio + 0.5) - 1;

            Vec3b pix;
            average(src, Point_<int>(lastRow, lastCol), Point_<int>(row, col), pix);
            p[j] = pix;

            lastCol = col + 1; //下一个子块左上角的列坐标，行坐标不变
        }
        lastCol = 0; //子块的左上角列坐标，从0开始
        lastRow = row + 1; //子块的左上角行坐标
    }
}

int main()
{
	VideoCapture cap("1.avi");
	if (!cap.isOpened()){
		printf("video open failed!"); 
 	    cv::waitKey(0);
	    return -1;
	}
	Mat frame;
	cap >> frame;

	double fps=cap.get(CV_CAP_PROP_FPS);
	int w=static_cast<int>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
	int h= static_cast<int>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
	cv::Size size=Size(w/3,h/3);
	int count=cap.get(CV_CAP_PROP_FRAME_COUNT);
	cv::VideoWriter writevideo("../1.avi",CV_FOURCC('M','J','P','G'), fps, size);

	Mat frame2;
	scalePartAverage(frame, frame2, 3, 3);
	writevideo<<frame2;


	for (;;) {
		cap >> frame;
		if (frame.empty()) break;

		// stop the program if no more images
		if (frame.rows == 0 || frame.cols == 0)
			break;

	

	    scalePartAverage(frame, frame2, 3, 3);
	    writevideo<<frame2;

		// show image with the tracked object
		imshow("tracker", frame);

		writevideo<<frame;

		//! [visualization]
		//quit on ESC button
		if (waitKey(1) == 27)
			break;
	}

	//ReleaseTracker(&tracker);
	return EXIT_SUCCESS;






}
*/





