#include "VibeModel.h"
#include "HDcommon.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
//#include <fstream>

//#define FOR_DEBUG_USE

#ifdef FOR_DEBUG_USE
#include "CFG_OpenCV.h"
#endif

#define MIN_FG_PTS 12
//#define EPS_DOUBLE 0.0000001
#define FG_UDRATE 25
#define BG_UDRATE 5

#define MIN(a,b) (a) < (b) ? (a):(b)
#define MAX(a,b) (a) > (b) ? (a):(b)

int table_BlockSizexj[IMG_WIDTH];
void CalcBlobSizeTable(int w,int blocksize)
{
    for (int i=0;i<w;i++)
    {
        table_BlockSizexj[i] = i*blocksize;
    }
}
// 注意：在Update中已经使用位移方式取代j*depth，因而当前代码仅仅对应于depth=16的情况
VibeParam* CreateBgParam()
{
    VibeParam* ptr = new VibeParam;
    ptr->w = ptr->h = 0;                 //模型宽高
    ptr->depth = 14;                     //模型大小  默认16
    ptr->blockD = 5;                     //邻域？大小 默认8？9
    ptr->nTh = 2;                        //个数阈值  默认2
    ptr->Radius = 10;                    //距离半径  默认20
    ptr->pModel = NULL;
    ptr->pFgImg = NULL;
    ptr->pHistoryImage = NULL;              //两点历史值
    ptr->lastHistoryImageSwapped = 0;       //用于交换模型点
    ptr->frameCnt = 0;
    ptr->jump = NULL;                       //由于模型更新时，跳过若干点

    ptr->mindiff	= 10;                   //帧差所需参数
    ptr->prevImg	= NULL;
    ptr->cntImg		= NULL;

    ptr->randorDep = 0;                     //随机数，更新模型下表

    ptr->alarmCntTh = MIN_FG_PTS;
    ptr->alarmPercentage = 0.0005;         //前景点数达到这个比例会触发报警，灵敏度参数之一
    ptr->justInitial = false;             //上一帧是否用于初始化
    ptr->Sensitive = 0;                   //灵敏度参数

    ptr->roi_x = ptr->roi_y = ptr->roi_w = ptr->roi_h = -1;            //检测区域xywh 

    //ptr->last_is_fore = true;
    //int index = 0;
    //while( index < IMG_WIDTH )
    //{
	   // ptr->last_is_fore_vec[index++] = true;
    //}
    return ptr;
}

void setSens(VibeParam* ptr, unsigned int m_uMDSensitive)
{
    if (ptr->Sensitive != m_uMDSensitive)
    {
#ifdef FOR_DEBUG_USE
        printf("Sensitive changed =========== %d to %d\n",ptr->Sensitive,m_uMDSensitive);
#endif
        ptr->Sensitive = m_uMDSensitive;
        switch (ptr->Sensitive)     // 灵敏度设置，分0-3共4个档，灵敏度依次降低。
        {
        case 0:
            ptr->Radius = 10;
            ptr->alarmPercentage = 0.0005;  // 前景比例尽量小
			ptr->mindiff	= 10;
            //ptr->alarmPercentage = 0.006;
            //ptr->alarmFilterTh = 2.5;
            break;
        case 1:
            ptr->Radius = 12;
            ptr->alarmPercentage = 0.001;
			ptr->mindiff	= 12;
            //ptr->alarmPercentage = 0.008;
            //ptr->alarmFilterTh = 2.5;
            break;
        case 2:
            ptr->Radius = 15;
            ptr->alarmPercentage = 0.001;
			ptr->mindiff	= 15;
            //ptr->alarmPercentage = 0.01;
            //ptr->alarmFilterTh = 3.0;
            break;
        case 3:
            ptr->Radius = 20;
            ptr->alarmPercentage = 0.003;
			ptr->mindiff	= 15;
            //ptr->alarmPercentage = 0.012;
            //ptr->alarmFilterTh = 3.0;
            break;
        default:
            break;
        }
    }
}

bool InitBackModel( const uchar* ptr_gray, const int width, const int height, const int step, unsigned int m_uMDSensitive, VibeParam* phd )
{

    if ( phd != NULL && phd->pModel != NULL )
    {
        delete[] phd->pModel;
    }
    if ( phd != NULL && phd->pFgImg != NULL)
    {
        delete[] phd->pFgImg;
    }
    if ( phd != NULL && phd->pHistoryImage != NULL )
    {
        delete[] phd->pHistoryImage;
    }
    if ( phd != NULL && phd->jump != NULL )
        delete[] phd->jump;
    if ( phd != NULL && phd->prevImg != NULL )
    {
        delete[] phd->prevImg;
    }
    if ( phd != NULL && phd->cntImg != NULL )
    {
        delete[] phd->cntImg;
    }


    setSens(phd,m_uMDSensitive);    //设置灵敏度
    //ROI参数检查
    if (phd->roi_x<0||phd->roi_y<0||phd->roi_w<=0||phd->roi_h<=0)
    {
        phd->roi_x = 0;
        phd->roi_y = 0;
        phd->roi_w = width;
        phd->roi_h = height;
    }

    int x_max = MIN(width,phd->roi_x+phd->roi_w);
    int y_max = MIN(height,phd->roi_y+phd->roi_h);
    phd->roi_x = MIN(phd->roi_x,width-1);
    phd->roi_y = MIN(phd->roi_y,height-1);
    phd->roi_w = x_max - phd->roi_x;
    phd->roi_h = y_max - phd->roi_y;
    phd->blockD = (phd->roi_w + phd->roi_h + IMG_WIDTH - 1)/IMG_WIDTH;    //？？
    if (phd->blockD == 0)
    {
        phd->blockD = 1;
    }
    CalcBlobSizeTable(IMG_WIDTH,phd->blockD);
    phd->w = (phd->roi_w + phd->blockD/2)/phd->blockD;
    phd->h = (phd->roi_h + phd->blockD/2)/phd->blockD;
    phd->pModel = new uchar[phd->w*phd->h*phd->depth];      //模型的数据 由模型大小w*h以及每个点的样本数量*depth构成
    phd->pHistoryImage = new uchar[phd->w*phd->h*2];        //两点历史值
    phd->pFgImg = new uchar[phd->w*phd->h];                 //前景图像
    phd->prevImg = new uchar[phd->w*phd->h];                //之前的前景图像
    phd->cntImg  = new uchar[phd->w*phd->h];                //
    phd->jump = new int[2*phd->w+1];                        //
    memset(phd->cntImg,0,sizeof(uchar)*phd->w*phd->h);       //将cntimg的前w*h个值设为0
    phd->alarmCntTh = MAX(MIN_FG_PTS,int(phd->w*phd->h*phd->alarmPercentage));   //警告点数取20 或者 模型的一定比例点数 的最大者
	//phd->alarmCntTh =10;         //该参数决定了检测的灵敏度，也可以通过外层的sensitive参数进行设置

    for (int i=0; i<2*phd->w+1; i++)
    {
        phd->jump[i] = (rand() % 32) + 1;
    }

    int blockR = phd->blockD/2;
    int gWS = step, mWS = phd->depth*phd->w;
    const uchar* ptrImg = ptr_gray + (phd->roi_y+blockR)*gWS + phd->roi_x + blockR;
    uchar* ptrM = phd->pModel;
    uchar* ptrPrev = phd->prevImg;
    uchar* ptrHist1 = phd->pHistoryImage;
    uchar* ptrHist2 = phd->pHistoryImage+phd->w*phd->h;

    memset(phd->pFgImg,0,phd->w*phd->h);
    for ( int i = 0; i < phd->h; i ++ )
    {
        for ( int j = 0; j < phd->w; j ++ )
        {
            uchar ucdata = ptrImg[j*phd->blockD];
            ptrPrev[j] = ucdata;
            ptrHist1[j] = ucdata;
            ptrHist2[j] = ucdata;
            memset(ptrM+j*phd->depth,ucdata,sizeof(uchar)*phd->depth);
            //for ( int k = 0; k < phd->depth; k ++ )
            //{
            //  ptr1[k+j*phd->depth] = ptr0[j*phd->blockD];
            //}
        }
        ptrHist1 += phd->w;
        ptrHist2 += phd->w;
        ptrPrev += phd->w;
        ptrM += mWS;
        ptrImg += gWS*phd->blockD;
    }
    phd->lastHistoryImageSwapped = 0;
    phd->justInitial = true;
    return true;
}

//VIBE前景检测
int UpdateBgModel( const uchar* ptr_gray, const int width, const int height, const int step, unsigned int m_uMDSensitive, VibeParam *phd/*Handle*/ )
{
	if ( phd->frameCnt < IGNORED_LEN )
	{
		phd->frameCnt ++;
		return FRAME_IGNORED;
	}
	if ( phd->frameCnt == IGNORED_LEN)
	{
		printf("init vibe background!\n");

		phd->frameCnt ++;
		InitBackModel( ptr_gray, width, height, step, m_uMDSensitive,phd );      //初始化背景
		return INIT_MODULE;
	}

    memset( phd->pFgImg, 0, sizeof(uchar)*phd->w*phd->h);
    // Calculate the foreGround and update model                                   计算前景更新背景
    int blockR = phd->blockD/2;
    int mWS = phd->w*phd->depth, gWS = step, depth = phd->depth;
    //int blockSize = 2*phd->blockR + 1;
    int blockStep = gWS*phd->blockD;
    const uchar* pG = ptr_gray + (phd->roi_y+blockR+phd->blockD*(phd->h-1))*gWS + phd->roi_x + blockR ;
    uchar* pPrev = phd->prevImg + (phd->h-1)*phd->w ;
    uchar* pCnt = phd->cntImg + (phd->h-1)*phd->w;
    uchar* pM = phd->pModel + (phd->h-1)*mWS;
    uchar* pHist1 = phd->pHistoryImage + (phd->h-1)*phd->w;
    uchar* pHist2 = phd->pHistoryImage + (phd->h*2-1)*phd->w;
    uchar* pFg = phd->pFgImg + (phd->h-1)*phd->w;
    uchar* swappingImageBuffer = pHist1 + (phd->lastHistoryImageSwapped) * phd->w * phd->h;
    phd->lastHistoryImageSwapped = (phd->lastHistoryImageSwapped + 1) % 2;
    int cnt, Gcolor, prevColor;

#ifdef FOR_DEBUG_USE
	IplImage* maskImg = cvCreateImage( cvSize(phd->w*phd->blockD,phd->h*phd->blockD), IPL_DEPTH_8U, 1 );
	uchar* ptr = (uchar*)maskImg->imageData+ (phd->h-1)*phd->blockD*maskImg->widthStep;
#endif

    for (int ii = 0; ii < phd->w; ii++)	//--change to phd->w
    {
        phd->last_is_fore_vec[ii] = false;
    }

    setSens(phd,m_uMDSensitive);
    int totalCnt = 0;
    for ( int i = phd->h - 1; i >= 0 ; i -- )
    {
        phd->last_is_fore = true;
        for ( int j = phd->w - 1; j >= 0; j -- )
        {
            // calc fg
            int index_j = *(table_BlockSizexj+j);
            cnt = 0, Gcolor = int(*(pG + index_j ));
            prevColor = int(*(pPrev + j ));
            *(pPrev + j ) = uchar(Gcolor);
            if (abs(Gcolor-prevColor)>phd->mindiff)
            {
                *( pCnt + j ) = 20;
            }
            else if (*(pCnt + j) > 0)
            {
                *( pCnt + j ) -= 1;
            }
            //				ptr[j] = 0;
#ifdef FOR_DEBUG_USE
            for (int ii=0; ii<phd->blockD; ii++)
            {
                for (int jj=0; jj<phd->blockD; jj++)
                {
                    ptr[ii*maskImg->widthStep+j*phd->blockD+jj] = 0;
                }
            }
#endif
            if ( *( pCnt + j ) == 0 )
            {
                if ( phd->last_is_fore && phd->last_is_fore_vec[j] )
                {
                    *(pFg+j) = 255;
                    totalCnt++;
#ifdef FOR_DEBUG_USE
                    for (int ii=0; ii<phd->blockD-1; ii++)
                    {
                        for (int jj=0; jj<phd->blockD-1; jj++)
                        {
                            ptr[ii*maskImg->widthStep+j*phd->blockD+jj] = 255;
                        }
                    }
#endif
                }
                phd->last_is_fore = false;
                phd->last_is_fore_vec[j] = false;
            }
            else
            {
                cnt += ( abs( Gcolor - *(pHist1+j)) <= phd->Radius );
                cnt += ( abs( Gcolor - *(pHist2+j)) <= phd->Radius );
                int index_m = 0;
                uchar *pmsub = pM + j*phd->depth;
                while (cnt < 2 && index_m < phd->depth)
                {
                    if( abs( Gcolor - *(pmsub + index_m)) <= phd->Radius )
                    {
                        cnt++;
                        uchar tmp = *(pmsub + index_m);
                        *(pmsub + index_m) = *(swappingImageBuffer + j);
                        *(swappingImageBuffer + j) = tmp;
                    }
                    index_m ++;
                }

                if ( cnt >= phd->nTh ) // when it's backGround pt
                {
                    //	ptr[j] = 0;
                    if ( phd->last_is_fore && phd->last_is_fore_vec[j] )
                    {
                        *(pFg+j) = 255;
                        totalCnt++;
#ifdef FOR_DEBUG_USE
                        for (int ii=0; ii<phd->blockD-1; ii++)
                        {
                            for (int jj=0; jj<phd->blockD-1; jj++)
                            {
                                ptr[ii*maskImg->widthStep+j*phd->blockD+jj] = 255;
                            }
                        }
#endif
                    }
                    phd->last_is_fore = false;
                    phd->last_is_fore_vec[j] = false;
                }
                else
                {
                    if ( phd->last_is_fore || phd->last_is_fore_vec[j] )
                    {
                        *(pFg+j) = 255;
                        totalCnt++;
#ifdef FOR_DEBUG_USE
                        for (int ii=0; ii<phd->blockD-1; ii++)
                        {
                            for (int jj=0; jj<phd->blockD-1; jj++)
                            {
                                ptr[ii*maskImg->widthStep+j*phd->blockD+jj] = 255;
                            }
                        }
#endif
                    }
                    phd->last_is_fore = true;
                    phd->last_is_fore_vec[j] = true;
                }
            }
        }
#ifdef FOR_DEBUG_USE
        ptr -= maskImg->widthStep*phd->blockD;
#endif
        pM -= mWS;
        pG -= blockStep;
        pCnt -= phd->w;
        pPrev -= phd->w;
        pHist1 -= phd->w;
        pHist2 -= phd->w;
        pFg -= phd->w;
        swappingImageBuffer -= phd->w;
    }

    // 背景模型更新
    int indX,shift;
    for (int y=0; y<phd->h; y++)
    {
        shift = rand()%phd->w;
        indX = phd->jump[shift] - 1;
        while( indX < phd->w )
        {
            int index = indX + y * phd->w;
            if (phd->pFgImg[index] == 0)
            {
                uchar val = phd->prevImg[index];
                phd->randorDep = (phd->randorDep+1)%(depth+2);
                if (phd->randorDep < depth)
                {
                    phd->pModel[index*phd->depth + phd->randorDep] = val;
                }else{
                    phd->pHistoryImage[(phd->randorDep-depth)*phd->w*phd->h + index] = val;
                }
            }
            ++shift;
            indX += phd->jump[shift];
        }
    }
    //phd->randorBG = ( phd->randorBG + rand() ) % BG_UPDATE_CNT;
    //phd->randorFG = ( phd->randorFG + rand() ) % FG_UPDATE_CNT;
    phd->randorDep = ( phd->randorDep + rand() ) % (depth+2);

#ifdef FOR_DEBUG_USE
    //CvSize sz;
    //sz.width = 640;
    //sz.height =360;
    //IplImage *showImg = cvCreateImage(sz,8,1); 
    //cvResize(maskImg,showImg,CV_INTER_CUBIC);
    cvShowImage( "mask", maskImg );
    cvWaitKey( 1 );
    cvReleaseImage( &maskImg );
    //cvReleaseImage( &showImg );
#endif

	// fore region is too large and the background has changed for much,
	// so we need to re-init the background
	//int colCnt = 0, rowCnt = 0;
	int wholeArea = (phd->w-2)*(phd->h-2);
#ifdef FOR_DEBUG_USE
	//printf("%f\n",1.0*totalCnt/wholeArea);
#endif



	// we have just init the model but current fore pts number is large
	// or fore pts num is too large
	if ((phd->justInitial && totalCnt > 0.25*wholeArea) || totalCnt > 0.45*wholeArea)
	{
//		InitBackModel( ptr_gray, width, height, step, sensitive, phd );
		printf("foreground change too much,return initing background model!\n");
		phd->frameCnt = 0;
		return FRAME_IGNORED;
	}
	else
	{
		phd->justInitial = false;
		if ( totalCnt > phd->alarmCntTh )
		{
#ifdef FOR_DEBUG_USE
			printf("enough points!\n");
#endif
			return ENOUGH_POINTS;
		}
		else	// fore pts num is not large enough
		{
#ifdef FOR_DEBUG_USE
			printf("not enough points!\n");
#endif 
			return NOT_ENOUTH_POINTS;
		}
	}

}
bool ReleaseBgModel( VibeParam** pvp/*Handle*/ )
{
//	VibeParam** pvp = (VibeParam**)pvpHandle;
	if ( pvp!= NULL && *pvp != NULL &&
		(*pvp)->pModel != NULL )
		delete[] (pvp[0]->pModel);
    if ( pvp!= NULL && *pvp != NULL &&
        (*pvp)->pHistoryImage != NULL )
        delete[] (pvp[0]->pHistoryImage);
    if ( pvp!= NULL && *pvp != NULL &&
        (*pvp)->pFgImg != NULL )
        delete[] (pvp[0]->pFgImg);
    if ( pvp!= NULL && *pvp != NULL &&
        (*pvp)->prevImg != NULL )
        delete[] (pvp[0]->prevImg);
    if ( pvp!= NULL && *pvp != NULL &&
        (*pvp)->cntImg != NULL )
        delete[] (pvp[0]->cntImg );
    if ( pvp!= NULL && *pvp != NULL &&
        (*pvp)->jump != NULL )
        delete[] (pvp[0]->jump);
	if ( pvp!=NULL && *pvp!= NULL )
		delete *pvp;
	*pvp = NULL;
	return true;
}
//打印当前时间，测试时使用
//#include <time.h>
//time_t now;
//struct tm *timenow;
//time(&now);
//timenow = localtime(&now);
//printf( "%s", asctime(timenow) );
