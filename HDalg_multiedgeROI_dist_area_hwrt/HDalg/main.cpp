// HDalg.cpp : Defines the entry point for the console application.
//
#include "HDalg.h"
#include "CFG_OpenCV.h"
#include <conio.h>
#include <iostream>
using namespace cv;
using std::vector;
using namespace std;

int pnpoly2 (int nvert, int *vertx, int *verty, int testx, int testy) 
{
	int i, j, c = 0;
	for (i = 0, j = nvert-1; i < nvert; j = i++) 
	{
		if ( ( (verty[i]>testy) != (verty[j]>testy) ) &&(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
			c = !c;
	}
	return c;
}

int main(int argc, char* argv[])
{

    HumanDetHandle phdHandle = CreateBackModel();
    VideoCapture cap( 
        //"E:\\²âÊÔ\\ÒÆ¶¯Õì²â\\×Ô²âÂ¼Ïñ20141021\\¾àÀë²âÊÔ.264"
        //"D:\\360Downloads\\172.18.32.140-00-134407.264"
        //"C:\\Users\\glennge\\Desktop\\9-Algorithm\\5.mp4"
		//"car.avi"
		"test3.mp4"
		//"C:\\Users\\glennge\\Desktop\\9-Algorithm\\Æû³µÊÓÆµ\\30-1.mp4"
		//"C:\\Users\\glennge\\Desktop\\9-Algorithm\\Æû³µÊÓÆµ\\test3.mp4"
	   //"C:\\Users\\glennge\\Desktop\\9-Algorithm\\20171102.mp4"
	   //"C:\\Users\\glennge\\Desktop\\9-Algorithm\\corridor.avi"
       //"E:\\²âÊÔ\\ÒÆ¶¯Õì²â\\²âÊÔ×éÂ¼Ïñ\\´°Íâ»Æ»è.264"
	   //"C:\Users\glennge\Desktop\9-Algorithm\Motion Detection\test20180113"
    );
	int framenum=(int) cap.get(CV_CAP_PROP_FRAME_COUNT); 
	double fps=cap.get(CV_CAP_PROP_FPS)/1;
	int delay=1000/fps;

    Mat frame;
    cap>>frame;
	printf("cols:%d,rows:%d\n",frame.cols,frame.rows);
    Mat gray;
    cvtColor( frame, gray, COLOR_BGR2GRAY );
    int frm_num = 0, half_num = 0, jmp_cnt = 0;
    double t0 = cvGetTickCount();

    //cv::Rect det_roi = cv::Rect(0,0,frame.cols/2,frame.rows);
    cv::Rect det_roi = cv::Rect(0,0,frame.cols,frame.rows);
    //SetROI(phdHandle,det_roi.x,det_roi.y,det_roi.width,det_roi.height,0);
	int nvert=8;
	int *vertx=new int[nvert];
	int *verty=new int[nvert];
	float kkk=0.5;
	Point p;
	/*p.x=150; p.y=41;       vertx[0]=p.x;verty[0]=p.y;
	p.x=250; p.y=51;       vertx[1]=p.x;verty[1]=p.y;
	p.x=80; p.y=210;      vertx[2]=p.x;verty[2]=p.y;
	p.x=250; p.y=200;    vertx[3]=p.x;verty[3]=p.y;
	p.x=30; p.y=170;      vertx[4]=p.x;verty[4]=p.y;
	p.x=280; p.y=155;      vertx[5]=p.x;verty[5]=p.y;
	p.x=55; p.y=120;      vertx[6]=p.x;verty[6]=p.y;
	p.x=270; p.y=100;     vertx[7]=p.x;verty[7]=p.y;*/  //30 41 250 169

	p.x=335; p.y=464;       vertx[0]=p.x*kkk;verty[0]=p.y*kkk;
	p.x=26; p.y=447;       vertx[1]=p.x*kkk;verty[1]=p.y*kkk;
	p.x=12; p.y=320;      vertx[2]=p.x*kkk;verty[2]=p.y*kkk;
	p.x=1; p.y=187;    vertx[3]=p.x*kkk;verty[3]=p.y*kkk;
	p.x=8; p.y=9;      vertx[4]=p.x*kkk;verty[4]=p.y*kkk;    //180 170
	p.x=173; p.y=13;      vertx[5]=p.x*kkk;verty[5]=p.y*kkk;    //6¸öµã 80 11 200 199
	p.x=337; p.y=18;      vertx[6]=p.x*kkk;verty[6]=p.y*kkk;
	p.x=335; p.y=248;     vertx[7]=p.x*kkk;verty[7]=p.y*kkk;    //8¸öµã 55 11 225 199

	for(int i=0;i<nvert;i++)
	{
		circle(frame,Point(vertx[i],verty[i]),3,Scalar(10,255,128));
		if(i<nvert-1) line(frame,Point(vertx[i],verty[i]),Point(vertx[i+1],verty[i+1]),Scalar(0,255,255)); 
		else line(frame,Point(vertx[i],verty[i]),Point(vertx[0],verty[0]),Scalar(0,255,255)); 
	}
	printf("test 1:%d\n",pnpoly2(8,vertx,verty,20,20));
	printf("test 2:%d\n",pnpoly2(8,vertx,verty,210,10));

	det_roi = cv::Rect(1*kkk,9*kkk,336*kkk,455*kkk);
	SetROI_MultiEdge(phdHandle,vertx,verty,nvert,0);

	//det_roi = cv::Rect(frame.cols*0.25,frame.rows*0.25,frame.cols*0.5,frame.rows*0.5);
	//SetROI(phdHandle,det_roi.x,det_roi.y,det_roi.width,det_roi.height,0);

	imshow( "original", frame );
	waitKey(0);

	double tstart=getTickCount();
	double totalTime = 0.0;
	//SetFilterParamDist(phdHandle,-1,-1);
    while ( 1)
    {
		//printf("NEW FRAME!******************\n");
        cap >>frame;
        if (frame.empty())
        {
            break;
        }
       /* jmp_cnt = (jmp_cnt+1)%2;
        if (jmp_cnt)
        {
            continue;
        }*/
		for(int i=0;i<nvert;i++)
		{
             circle(frame,Point(vertx[i],verty[i]),3,Scalar(10,255,128));
             if(i<nvert-1) line(frame,Point(vertx[i],verty[i]),Point(vertx[i+1],verty[i+1]),Scalar(0,255,255)); 
			 else line(frame,Point(vertx[i],verty[i]),Point(vertx[0],verty[0]),Scalar(0,255,255)); 
		}

        cvtColor( frame, gray, COLOR_BGR2GRAY );
        double t = getTickCount();
        int ret = UpdateAndHD(phdHandle, (unsigned char*)gray.data, gray.cols, gray.rows, gray.step, 2/*, &out_objs*/);
        t = getTickCount() - t;
		printf("frame time%gms\n",t/(cvGetTickFrequency()*1000.0));
        totalTime += t;


        if (3 == ret)
        {
            putText( frame, "MD", cv::Point( 100,100 ), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0,255,0), 0 );
		}
        ReturnObjs * out_objs = getObjs(phdHandle);

        int tl_x,tl_y,br_x,br_y; int pause=0; 

		for (int kk=0; kk<out_objs->num; kk++)
        {
            float fwidth = sqrt(out_objs->obj[kk].area*out_objs->obj[kk].AreaK*det_roi.height/(out_objs->obj[kk].hvsw*det_roi.width));
            float fheight = fwidth*out_objs->obj[kk].hvsw*det_roi.width/det_roi.height;
            tl_x = det_roi.x + int(det_roi.width*(out_objs->obj[kk].cx - fwidth/2));
            tl_y = det_roi.y + int(det_roi.height*(out_objs->obj[kk].cy - fheight/2));
            br_x = det_roi.x + int(det_roi.width*(out_objs->obj[kk].cx + fwidth/2));
            br_y = det_roi.y + int(det_roi.height*(out_objs->obj[kk].cy + fheight/2));
		
            if (out_objs->obj[kk].bflag)
            {
				//if(out_objs->obj[kk].hvsw>2.0&&out_objs->obj[kk].hvsw<4.0&&out_objs->obj[kk].area>0.03){
                char percent[64];
				if(3 == ret)
				{
                sprintf(percent,"%0.1f%% %0.1f ",out_objs->obj[kk].area*100,out_objs->obj[kk].hvsw/*,out_objs->obj[kk].move_dist/*out_objs->obj[kk].xspeed*/);
                putText( frame, percent, Point( tl_x,tl_y ), FONT_HERSHEY_SIMPLEX,1.0, Scalar(0,255,0),2 );
                rectangle(frame,Point(tl_x,tl_y),Point(br_x,br_y),Scalar(0,255,0),2);
				//printf("startx\y:%d, %d, endx\y:%d, %d\n",tl_x,tl_y,br_x,br_y);
				//printf("motion detected!\n");
				printf("green: object move distance vec in latest 2s / 60 frame is %f\n",out_objs->obj[kk].move_dist);
				//printf("motiondirection:    %d,     %d\n",out_objs->obj[kk].motiondirectx,out_objs->obj[kk].motiondirecty);
				//printf("green: object move distance mod in latest 2s / 60 frame is %f\n",out_objs->obj[kk].move_dist_mod);
				pause=1;
				}

            }else{
			   char percent[64];
               //sprintf(percent,"%0.1f%% %0.1f %0.2f",out_objs->obj[kk].area*100,out_objs->obj[kk].hvsw,out_objs->obj[kk].move_dist/*out_objs->obj[kk].xspeed*/);
               //putText( frame, percent, Point( tl_x,tl_y ), FONT_HERSHEY_SIMPLEX,1.0, Scalar(0,0,255),2 );
               rectangle(frame,Point(tl_x,tl_y),Point(br_x,br_y),Scalar(0,0,255),2);
			   //printf("%d %d %d %d\n", tl_x,tl_y,br_x,br_y);
			   //printf("motion matched!\n");
			   //printf("red: object move distance vec in latest 2s / 60 frame is %f\n",out_objs->obj[kk].move_dist);
			   //printf("red: object move distance mod in latest 2s / 60 frame is %f\n",out_objs->obj[kk].move_dist_mod);
			  // pause=1;
            }
		 
        }
	
        //Mat showImg;
        //resize(frame,showImg,Size(640,360));
        imshow( "original", frame );
        if(pause==1) waitKey(0);

		
		if(kbhit()){
		  int k=getch();
		  if(k==' ')
		  {
			while(1)
			{
			 if(kbhit()&&getch()==' ') break;
			}
		  }
		}


        char c = waitKey( delay );
        if ( c == 27 )
            break;

    }
    printf( "Total time: %gms\n", totalTime/(cvGetTickFrequency()*1000.0) );
	printf("vedio time:%gms\n",(getTickCount()-tstart)/(cvGetTickFrequency()*1000.0));
    ReleaseBackModel(&phdHandle);


    system("pause");
    return 0;
}