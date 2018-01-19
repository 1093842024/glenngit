#ifndef _MEANSHIFT_INTERFACE_H
#define _MEANSHIFT_INTERFACE_H

struct BBox
{
	int x, y, w, h;
};

typedef void* TrackHandle;
TrackHandle CreateTracker(int histnum=8,float gamma=0.5,float scale_factor=0.1);
int InitTracker(TrackHandle th, BBox Roi, unsigned char* colorRGB, const int width, const int height,const float deltaT);
BBox UpdateTracker(TrackHandle th,unsigned char* colorRGB, const int width, const int height,float &Goal,BBox ROIin);
bool ReleaseTracker(TrackHandle pth);



#endif