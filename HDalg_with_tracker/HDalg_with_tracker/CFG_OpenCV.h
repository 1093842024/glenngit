#ifndef _CFG_OPENCV_H_
#define _CFG_OPENCV_H_

//在c版本的代码中只能用opencv1.0,因为c语法不支持namespace、类等语法
#ifndef __cplusplus
    #define  USE_OPENCV_1_0_0
#endif


#ifdef USE_OPENCV_1_0_0
#include "cv.h"
#include "cxcore.h"
#include "cvaux.h"
#include "cxmisc.h"
#include "highgui.h"
	#if defined _WIN32 || defined _WIN64
		#ifdef _DEBUG
		#pragma comment(lib, "cvd.lib")
		#pragma comment(lib, "cxcored.lib")
		#pragma comment(lib, "cvauxd.lib")
		#pragma comment(lib, "highguid.lib")
		#else
		#pragma comment(lib, "cv.lib")
		#pragma comment(lib, "cxcore.lib")
		#pragma comment(lib, "cvaux.lib")
		#pragma comment(lib, "highgui.lib")
		#endif
	#endif
#else
	#if defined _WIN32 || defined _WIN64
		#ifdef _DEBUG
			#define lnkLIB(name) name "d"
		#else
			#define lnkLIB(name) name
		#endif
		#include <opencv2/opencv.hpp> 
		#define CV_VERSION_ID CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
		#define cvLIB(name) lnkLIB("opencv_" name CV_VERSION_ID)
		#pragma comment( lib, cvLIB("core"))
		#pragma comment( lib, cvLIB("imgproc"))
		#pragma comment( lib, cvLIB("highgui"))
		#pragma comment( lib, cvLIB("contrib"))
//		#pragma comment( lib, cvLIB("objdetect"))
	#endif
#endif



#ifndef USE_OPENCV_1_0_0
using namespace cv;
#endif

#endif //end of _CFG_OPENCV_H_