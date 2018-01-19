// Meanshift_and_Tracing.cpp : �������̨Ӧ�ó������ڵ㡣
// Meanshift_and_Tracing.cpp : �������̨Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#define  u_char unsigned char
#define  DIST 0.5
#define  NUM 20
#define  R_bin 8
#define  G_bin 8
#define  B_bin 8 //��ͨ�����ֵΪ256/32��ÿһ��bin���32������0-31��32-63��64-127...224-255��RGB�ܼ�512��bin
#define  R_shift 5
#define  G_shift 5
#define  B_shift 5
#define  bins 512

//ȫ�ֱ���
bool gettarget = false;
bool pause = true;
bool is_tracking = false;
CvRect drawing_box;
CvPoint lastpoint; //��һ֡����������
CvPoint predictpoint, measurepoint;//Ԥ�������Լ���ǰ֡������
CvMat* measurement;
CvMat* realposition;
const CvMat* prediction;
IplImage *current;
double *hist1, *hist2, *hist_none1, *hist_none2;
double qu1, qu2;
double BTcharyya1 = 0, BTcharyya2 = 0;
double *K;		//�˺���															//Ȩֵ����
double C = 0.0;	//��һϵ��	
double h;// �˴���
int max_num;

void init_target(double *hist1, double *K, IplImage *current); //��ʼ��
void MeanShift_Tracking(IplImage *current);//��ֵƫ�Ƹ���
void onMouse(int event, int x, int y, int flags, void *param);//ѡȡĿ��
void CalColorBar(IplImage *current, int *q_temp, double *hist, double *hist_none);//������ɫ�ֲ�
void GetCurentState(CvKalman* kalman, CvPoint point1, CvPoint point2);//��ȡ
CvKalman* InitializeKalman(CvKalman* kalman);//kalman��ʼ��

int _tmain(int argc, _TCHAR* argv[])
{
  CvCapture *capture = cvCreateFileCapture("motinas_emilio_webcam_turning.avi");
  current = cvQueryFrame(capture);


  int nframe = 0;//����
  cvNamedWindow("Meanshift", 1);
  cvSetMouseCallback("Meanshift", onMouse, 0);

  //��ʼ��kalman
  CvKalman* kalman = 0;
  kalman = InitializeKalman(kalman);
  measurement = cvCreateMat(2, 1, CV_32FC1);//Z(k)
  realposition = cvCreateMat(4, 1, CV_32FC1);//real X(k)

  while (!gettarget)
  {

    cvShowImage("Meanshift", current);
    if (cvWaitKey(20) == 'q')
      return 1;
  }
  //Remove callback
  cvSetMouseCallback("Meanshift", NULL, NULL);

  while (1)
  {		//��ó�ʼkalman����ֵ
    lastpoint = predictpoint = cvPoint(drawing_box.x + drawing_box.width / 2,
      drawing_box.y + drawing_box.height / 2);
    GetCurentState(kalman, lastpoint, predictpoint);//input curent state

    //Ԥ��Ŀ����λ��(x,y,vx,vy), 
    prediction = cvKalmanPredict(kalman, 0);//predicton=kalman->state_post 
    predictpoint = cvPoint((int)prediction->data.fl[0], (int)prediction->data.fl[1]);
    //Ԥ����ľ��ο�
    drawing_box = cvRect(predictpoint.x - drawing_box.width / 2, predictpoint.y - drawing_box.height / 2,
      drawing_box.width, drawing_box.height);
    // drawing_box=my_ChangeRect(cvRect(0,0,frame->width,frame->height),track_window);

    if (is_tracking)
    {
      MeanShift_Tracking(current);
    }


    //ʵ�ʵ��������� 
    measurepoint = cvPoint(drawing_box.x + drawing_box.width / 2,
      drawing_box.y + drawing_box.height / 2);
    realposition->data.fl[0] = measurepoint.x;
    realposition->data.fl[1] = measurepoint.y;
    realposition->data.fl[2] = measurepoint.x - lastpoint.x;
    realposition->data.fl[3] = measurepoint.y - lastpoint.y;
    lastpoint = measurepoint;//keep the current real position
    //ʵ�������measurementֻ�ǵ�ǰ��x��y
    cvMatMulAdd(kalman->measurement_matrix/*2x4*/, realposition/*4x1*/,/*measurementstate*/ 0, measurement);
    cvKalmanCorrect(kalman, measurement);

    int c = cvWaitKey(1);
    //��ͣ
    if (c == 'p')
    {
      pause = true;

    }
    while (pause){
      if (cvWaitKey(0) == 'p')
        pause = false;
    }
    cvShowImage("Meanshift", current);
    current = cvQueryFrame(capture); //ץȡһ֡
	if(!current)
		break;
  }

  cvReleaseCapture(&capture);
  cvDestroyWindow("Meanshift");
  return 0;
}

void init_target(double *hist1, double *K, IplImage *current)
{
  IplImage *pic_hist = 0;
  double  dist;
  int i, j;
  int q_r, q_g, q_b;
  int *q_temp;

  h = pow(((double)drawing_box.width) / 2, 2) + pow(((double)drawing_box.height) / 2, 2);			//�˺������
  pic_hist = cvCreateImage(cvSize(300, 200), IPL_DEPTH_8U, 3);     //����ֱ��ͼͼ��
  hist_none1 = (double *)malloc(sizeof(double)*bins);
  q_temp = (int *)malloc(sizeof(int)*drawing_box.height*drawing_box.width);
  //��ʼ��Ȩֵ�����Ŀ��ֱ��ͼ
  for (i = 0; i < drawing_box.width*drawing_box.height; i++)
  {
    K[i] = 0.0;
  }

  for (i = 0; i<bins; i++)
  {
    hist1[i] = 0.0;
    hist_none1[i] = 0.0;
  }

  for (i = 0; i < drawing_box.height; i++)
  {
    for (j = 0; j < drawing_box.width; j++)
    {
      dist = pow(i - (double)drawing_box.height / 2, 2) + pow(j - (double)drawing_box.width / 2, 2);
      K[i * drawing_box.width + j] = 1 - dist / h; //Epannechnikov�˺���1-x^2
      C += K[i * drawing_box.width + j];//��һ��ϵ��
    }
  }


  CalColorBar(current, q_temp, hist1, hist_none1);


  //����Ŀ��ֱ��ͼ
  double temp_max = 0.0, tempnone_max = 0.0;
  for (i = 0; i < bins; i++)			//��ֱ��ͼ���ֵ��Ϊ�˹�һ��
  {
    hist1[i] = hist1[i] / C;       //��һ����ɫ�ֲ�qu
    hist_none1[i] = hist_none1[i] / (drawing_box.height*drawing_box.width);
    BTcharyya1 += sqrt(hist1[i]);
    if (temp_max < hist1[i])
    {
      temp_max = hist1[i];
    }
    if (tempnone_max<hist_none1[i])
    {
      tempnone_max = hist_none1[i];
      max_num = i;
    }
  }
  qu1 = hist_none1[max_num];//��ɫ�ֲ�������

  //��ֱ��ͼ
  CvPoint p1, p2;
  double bin_width = (double)pic_hist->width / bins;
  double bin_unith = (double)pic_hist->height / temp_max;

  for (i = 0; i < bins; i++)
  {
    p1.x = i * bin_width;
    p1.y = pic_hist->height;
    p2.x = (i + 1)*bin_width;
    p2.y = pic_hist->height - hist1[i] * bin_unith;
    cvRectangle(pic_hist, p1, p2, cvScalar(0, 255, 0), -1, 8, 0);
  }
  cvSaveImage("hist1.jpg", pic_hist);
  cvReleaseImage(&pic_hist);


}

void MeanShift_Tracking(IplImage *current)
{
  int num = 0, i = 0, j = 0;
  double *w = 0;
  double sum_w = 0, x1 = 0, x2 = 0, y1 = 2.0, y2 = 2.0, r = 0.0;
  int *q_temp;
  IplImage *pic_hist = 0;

  pic_hist = cvCreateImage(cvSize(300, 200), IPL_DEPTH_8U, 3);     //����ֱ��ͼͼ��
  hist2 = (double *)malloc(sizeof(double)*bins);//����ֱ��ͼ�ڴ�
  hist_none2 = (double *)malloc(sizeof(double)*bins);
  w = (double *)malloc(sizeof(double)*bins);
  q_temp = (int *)malloc(sizeof(int)*drawing_box.height*drawing_box.width);

  while ((pow(y1, 2) + pow(y2, 2))>1 && (num < NUM))
  {
    num++;
    memset(q_temp, 0, sizeof(int)*drawing_box.height*drawing_box.width);
    for (i = 0; i<bins; i++)
    {
      w[i] = 0.0;
      hist2[i] = 0.0;
      hist_none2[i] = 0.0;
    }

    CalColorBar(current, q_temp, hist2, hist_none2);


    //����Ŀ��ֱ��ͼ
    double temp_max = 0.0;

    for (i = 0; i<bins; i++)			//��ֱ��ͼ���ֵ��Ϊ�˹�һ��
    {
      hist2[i] = hist2[i] / C; //��һ��ֱ��ͼ
      hist_none2[i] = hist_none2[i] / (drawing_box.height*drawing_box.width);
      if (temp_max < hist2[i])
      {
        temp_max = hist2[i];
      }

    }
    qu2 = hist_none2[max_num];



    //��ֱ��ͼ
    CvPoint p1, p2;
    double bin_width = (double)pic_hist->width / (bins);
    double bin_unith = (double)pic_hist->height / temp_max;

    for (i = 0; i < bins; i++)
    {
      p1.x = i * bin_width;
      p1.y = pic_hist->height;
      p2.x = (i + 1)*bin_width;
      p2.y = pic_hist->height - hist2[i] * bin_unith;
      cvRectangle(pic_hist, p1, p2, cvScalar(0, 255, 0), -1, 8, 0);
    }
    cvSaveImage("hist2.jpg", pic_hist);

    for (i = 0; i < bins; i++)
    {
      if (hist2[i] != 0)
      {
        w[i] = sqrt(hist1[i] / hist2[i]);//Ȩ��
      }
      else
      {
        w[i] = 0;
      }
      BTcharyya2 += sqrt(hist1[i] * hist2[i]);
    }

    sum_w = 0.0;
    x1 = 0.0;
    x2 = 0.0;

    for (i = 0; i < drawing_box.height; i++)
    {
      for (j = 0; j < drawing_box.width; j++)
      {
        //printf("%d\n",q_temp[i * t_w + j]);
        sum_w = sum_w + w[q_temp[i * drawing_box.width + j]];
        x1 = x1 + w[q_temp[i * drawing_box.width + j]] * (i - drawing_box.height / 2);
        x2 = x2 + w[q_temp[i * drawing_box.width + j]] * (j - drawing_box.width / 2);
      }
    }
    y1 = x1 / sum_w;
    y2 = x2 / sum_w;



    //���ڶ˵�λ�ø��£���Ч������λ�ø��£�����������ͬ��
    drawing_box.x += y2;
    drawing_box.y += y1;

    h = pow(((double)drawing_box.width) / 2, 2) + pow(((double)drawing_box.height) / 2, 2);

    //���ڴ�С����
    if ((qu2 - qu1)>0.01)
    {
      r = sqrt(h*1.1);//���󴰿�
      //	drawing_box.width=r/2;
      //drawing_box.height=r/2*1.2;
    }



    else if ((qu1 - qu2)>0.1)
    {
      r = sqrt(h*0.9);//��С����
      // drawing_box.width=drawing_box.width-1;
      // drawing_box.height=drawing_box.height-1;
    }

  }
  free(hist2);
  free(w);
  free(q_temp);
  //��ʾ���ٽ��
  //cvCircle(current,cvPoint(drawing_box.x+drawing_box.width/2,drawing_box.y+drawing_box.height/2),r,CV_RGB(0,0,255),2);
  cvRectangle(current, cvPoint(drawing_box.x, drawing_box.y), cvPoint(drawing_box.x + drawing_box.width, drawing_box.y + drawing_box.height), CV_RGB(255, 0, 0), 2);
  cvShowImage("Meanshift", current);
  printf("%d\n", num);
  //cvSaveImage("result.jpg",current);
  cvReleaseImage(&pic_hist);
}

void CalColorBar(IplImage *current, int *q_temp, double *hist, double *hist_none)
{
  int t_w = 0, t_h = 0, t_x = 0, t_y = 0, i, j;
  int q_r, q_g, q_b;//ɫ�ʷ���

  t_w = drawing_box.width;//�����
  t_h = drawing_box.height;//��
  t_x = drawing_box.x;//
  t_y = drawing_box.y;//


  for (i = t_y; i < t_h + t_y; i++)
  {
    for (j = t_x; j < t_w + t_x; j++)
    {
      //rgb��ɫ�ռ�����Ϊ16*16*16 bins
      q_r = ((u_char)current->imageData[i * current->widthStep + j * 3 + 2]) >> R_shift;
      q_g = ((u_char)current->imageData[i * current->widthStep + j * 3 + 1]) >> G_shift;
      q_b = ((u_char)current->imageData[i * current->widthStep + j * 3 + 0]) >> B_shift;
      q_temp[(i - t_y) *t_w + j - t_x] = q_r * R_bin*G_bin + q_g * G_bin + q_b;
      hist[q_temp[(i - t_y) *t_w + j - t_x]] = hist[q_temp[(i - t_y) *t_w + j - t_x]] + K[(i - t_y) * t_w + j - t_x];
      hist_none[q_temp[(i - t_y) *t_w + j - t_x]] = hist_none[q_temp[(i - t_y) *t_w + j - t_x]] + 1;//����Ȩ�ķֲ�
    }
  }

}


void onMouse(int event, int x, int y, int flags, void *param)
{
  if (pause)
  {
    switch (event)
    {
    case CV_EVENT_LBUTTONDOWN:
      //the left up point of the rect
      drawing_box.x = x;
      drawing_box.y = y;
      break;
    case CV_EVENT_LBUTTONUP:
      //finish drawing the rect (use color green for finish)
      drawing_box.width = x - drawing_box.x;
      drawing_box.height = y - drawing_box.y;
      cvRectangle(current, cvPoint(drawing_box.x, drawing_box.y), cvPoint(drawing_box.x + drawing_box.width, drawing_box.y + drawing_box.height), CV_RGB(255, 0, 0), 2);
      cvShowImage("Meanshift", current);

      //Ŀ���ʼ��
      hist1 = (double *)malloc(sizeof(double)*bins);
      K = (double *)malloc(sizeof(double)*drawing_box.height*drawing_box.width);
      init_target(hist1, K, current);
      is_tracking = true;
      gettarget = true;
      break;
    }
    return;
  }
}

//**************************************
//kalman��ʼ��
CvKalman* InitializeKalman(CvKalman* kalman)
{

  const float A[] = { 1, 0, 1, 0,
    0, 1, 0, 1,
    0, 0, 1, 0,
    0, 0, 0, 1 };
  kalman = cvCreateKalman(4, 2, 0);
  memcpy(kalman->transition_matrix->data.fl, A, sizeof(A));//A
  cvSetIdentity(kalman->measurement_matrix, cvScalarAll(1));//H
  cvSetIdentity(kalman->process_noise_cov, cvScalarAll(1e-5));//Q w ��
  cvSetIdentity(kalman->measurement_noise_cov, cvScalarAll(1e-1));//R v
  cvSetIdentity(kalman->error_cov_post, cvScalarAll(1));//P
  return kalman;
}
//***************************************

//point1��һ֡���ģ�point2��ǰ֡���ģ���x,y,vx,vy����kalman->state_post
void GetCurentState(CvKalman* kalman, CvPoint point1, CvPoint point2)
{
  float input[4] = { point2.x, point2.y, point2.x - point1.x, point2.y - point1.y };//currentstate
  memcpy(kalman->state_post->data.fl, input, sizeof(input));
}
//****************************************



