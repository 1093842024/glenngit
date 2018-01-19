#ifndef __MORPH_H__
#define __MORPH_H__

#include <iostream>
#include <vector>
#include "HDcommon.h"
using std::vector;

bool IsAdjacent(HdRect r1, HdRect r2);
void Merge (HdRect *r1, HdRect r2);
int fastFindContourRects(const uchar *src, int width, int height, int widthStride, vector<HdRect> &blobList, HdSize operateSz,double ratio);
#endif