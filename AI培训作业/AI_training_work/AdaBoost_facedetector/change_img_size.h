#ifndef _H_CHANGE
#define _H_CHANGE

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <stdio.h>
using namespace cv;
using namespace std;

int changeimgsize_pos();
int changeimgsize_neg();
void getFiles( string path, vector<string>& files) ;
int createimg();

#endif