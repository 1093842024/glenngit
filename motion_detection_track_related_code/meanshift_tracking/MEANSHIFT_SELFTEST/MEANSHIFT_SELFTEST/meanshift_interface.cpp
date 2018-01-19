#include "meanshift_interface.h"
#include "MeanShift.h"
#include <stdlib.h>

TrackHandle CreateTracker(int histnum,float gamma,float scale_factor)
{
	MeanshiftTracker* tracker=new MeanshiftTracker;
	return (TrackHandle)tracker;
}

int InitTracker(TrackHandle th, BBox Roi, unsigned char* colorRGB, const int width, const int height,const float deltaT=0.1)
{
	MeanshiftTracker* tracker=(MeanshiftTracker*)th;
	int x,y,w,h;
	x=Roi.x+Roi.w/2;
	y=Roi.y+Roi.h/2;
	w=Roi.w/2;
	h=Roi.h/2;
	tracker->Initial_MeanShift_tracker(x,y,w,h,colorRGB,width,height,deltaT);
	return 0;
}

BBox UpdateTracker(TrackHandle th,unsigned char* colorRGB, const int width, const int height,float &Goal,BBox ROIin)
{
	MeanshiftTracker* tracker=(MeanshiftTracker*)th;
	int xi,yi,wi,hi,xo,yo,wo,ho;
	xi=ROIin.x+ROIin.w/2;
	yi=ROIin.y+ROIin.h/2;
	wi=ROIin.w/2;
	hi=ROIin.h/2;
	Goal=tracker->MeanShift_tracker(xi,yi,wi,hi,colorRGB,width,height,xo,yo,wo,ho);
	BBox ROIout;
	ROIout.x=xo-wo;
	ROIout.y=yo-ho;
	ROIout.w=wo*2;
	ROIout.h=ho*2;
	return ROIout;
}

bool ReleaseTracker(TrackHandle pth)
{
	if(pth!=NULL) delete pth;
	return true;
}
