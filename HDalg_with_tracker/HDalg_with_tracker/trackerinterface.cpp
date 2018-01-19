#include "kcftracker.hpp"
#include "trackerinterface.h"

TrackHandle CreateTracker(int template_size, float padding_size,float scale_size)
{
    KCFTracker* tracker = new KCFTracker(template_size,padding_size,scale_size);
    return (TrackHandle)tracker;
}

int InitTracker(TrackHandle th, BBox roi, unsigned char* gray, const int width, const int height, const int step)
{
    KCFTracker* tracker = (KCFTracker*)th;
    CMat img = CMat(height,width,MAT_8UC1,(void*)gray,step);
    //img.data = gray;
    CRect box(roi.x,roi.y,roi.w,roi.h);
    //cv::imshow("image", img );
    //cv::waitKey(0);
    tracker->init( box, img );
    
    return 0;
}

BBox UpdateTracker(TrackHandle th,unsigned char* gray, const int width, const int height,const int step,int &times,float &Goal)
{
    KCFTracker* tracker = (KCFTracker*)th;
    CMat img = CMat(height,width,MAT_8UC1,(void*)gray,step);
    //img.data = gray;
    CRect bb = tracker->update(img,times,Goal);
    BBox box;
    box.x = bb.x;
    box.y = bb.y;
    box.w = bb.width;
    box.h = bb.height;

    return box;
}

bool ReleaseTracker(TrackHandle pth)   //该函数进行了修改 之前形参是(TrackHandle* pth) 但释放指针ptracker时出现了错误 
{
   /*KCFTracker** ptracker = (KCFTracker**)pth;
	
    if (ptracker!=NULL && *ptracker != NULL)
    {
        //delete *ptracker;
        *ptracker = NULL;
        ptracker = NULL;
    }*/

	if(pth!=NULL) delete pth;

    return true;
}
