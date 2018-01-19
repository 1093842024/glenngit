#include "Morph.h"
//#define DEBUGUSE
#ifdef DEBUGUSE
#include "CFG_OpenCV.h"
#endif

#define MINOBJSIZE 15
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) > (b) ? (b) : (a))

//判断两个矩形是否相邻、相交或包含
bool IsAdjacent(HdRect r1, HdRect r2)
{
	if (r1.x + r1.width >= r2.x &&
		r1.x <= r2.x + r2.width &&
		r1.y + r1.height >= r2.y &&
		r1.y <= r2.y + r2.height)
		return true;
	else
		return false;
}

//将两个符合IsAdjacent的矩形合并成一个大的矩形
void Merge (HdRect *r1, HdRect r2)
{
	r1->width = MAX(r1->x + r1->width, r2.x + r2.width);
	r1->height = MAX(r1->y + r1->height, r2.y + r2.height);
	r1->x = MIN(r1->x, r2.x);
	r1->y = MIN(r1->y, r2.y);
	r1->width -= r1->x;
	r1->height -= r1->y;
	return;
}

/************************************************************************/
/* 快速遍历前景图像，输出bloblist          */
/* param: src 输入的前景图像          */
/* param: width,height 输入图像的宽高          */
/* param: blobList 输出          */
/* param: operateSz 操作单元大小，以operateSz大小的矩形为基本单元遍历图像*/
/* param: ratio 基本单元内前景点比例达到ratio则该单元为有效的          */
/************************************************************************/
int fastFindContourRects(const uchar *src,int width,int height,int widthStride,vector<HdRect> &blobList, HdSize operateSz, double ratio)
{
	if (!src) return 0;
	//if (blobList == NULL)
	//{
	//	return 0;
	//}
	//vector<HdRect> *seq = blobList;
	//vector<HdRect> *line1 = new vector<HdRect>;
	//vector<HdRect> *line2 = new vector<HdRect>;
	vector<HdRect> &seq = blobList;
	vector<HdRect> *line1 = new vector<HdRect>;  //存储上一个分析的高度为h的行图像中的前景矩形
	vector<HdRect> *line2 = new vector<HdRect>;   //存储当前分析的高度为h的行图像中的前景矩形
	const uchar *p = src;
	size_t step = widthStride;
	int b_width = width;
	int b_height = height;
	int w = operateSz.width, h = operateSz.height;
	double thresh = w*h*255.0*ratio;

	vector<HdRect>::iterator index1 = line1->begin();
	vector<HdRect>::iterator idx2 = line2->begin();

	for (int i = 0; i <= b_height-h; i += h)	//--可以取等号    根据小的操作单元，将前景图像中出现的块进行合并操作，形成初始的bloblist
	{
		line2->clear();
		for (int j = 0; j <= b_width-w; j += w)     //每次对一个高度为h的一行图像进行分析，首先分析是否2*3的小区域满足前景条件
		{                                           //如果满足，则将该矩形与之前的矩形对比，看是否邻近，邻近则合并；不论合不合并，都存放在line2中
			bool c = false;
			double tmp = 0.0;

			for (int ii = 0; ii < h; ii++)
			{
				for(int jj = 0; jj < w; jj++)
				{
					tmp += p[ii*step+j+jj];
					if (tmp >= thresh)
					{
						c = true;
						break;
					}
				}
				if (c) break;
			}

			if (c)
			{
				HdRect curRect = hdRect(j,i,w,h);	//--mark
				if (line2->size() == 0)
					line2->push_back(curRect);
				else
				{
					HdRect *last_elem = &(line2->back());
					if (IsAdjacent(*last_elem,curRect))
						Merge(last_elem,curRect);
					else
						line2->push_back(curRect);
				}
			}
		}

		index1 = line1->begin();
		idx2 = line2->begin();
		while (index1 != line1->end())                //对上一行分析line1的前景矩形与本次行分析line2的前景矩形进行合并处理，将合并的存储到line2中，并删除line1中的对应矩形
		{
			bool l2flag = true;
			for (vector<HdRect>::iterator index2 = idx2; index2 != line2->end(); index2 ++)
			{
				HdRect *elem1 = &(*index1);
				HdRect *elem2 = &(*index2);

				if (IsAdjacent(*elem1, *elem2))
				{
					Merge(elem2, *elem1);
					while(++ index2 != line2->end())
					{
						HdRect next_elem = *index2;
						if (IsAdjacent(next_elem, *elem2))
						{
							Merge(elem2, next_elem);
							index2 = line2->erase(index2);
							if (index2 != line2->begin()) index2--;
						}
						else
						{
							break;
						}
					}
					if(index2 != line2->begin()) 	idx2 = index2 - 1;
					index1 = line1->erase(index1); 
					if(index1 != line1->begin()) index1--;
					l2flag = false;
					break;//---????:? 结束line2，回到line1
				}
			}
			if (l2flag && index1!=line1->end()) index1 ++;
		}

		for (index1 = line1->begin(); index1 != line1->end(); index1 ++)    //将未合并到line2中的line1的矩形，存在bloblist中
		{
			HdRect tmp = *index1;
			if (tmp.height*tmp.width >= MINOBJSIZE)
			{
				seq.push_back(tmp);
			}

		}
		line1->clear();                                  //将line1置为line2的信息，line2清空，进行下一行分析
		{
			vector<HdRect> *tmp = line1;
			line1 = line2;
			line2 = tmp;
		}
		p += h*step;
	}

	//-- 最后一行的单元！
	for (index1 = line1->begin(); index1 != line1->end(); index1 ++)
	{
		HdRect tmp = *index1;
		if (tmp.height*tmp.width >= MINOBJSIZE)
		{
			seq.push_back(tmp);
		}
	}


	//-- 相邻的大块合并                      下面的一段代码对检测到的相邻目标做了合并处理，可以根据需求去掉这一段代码
	vector<HdRect>::iterator seq_1 = seq.begin();
	bool flag = false;
	while(seq_1 != seq.end())
	{
		HdRect *rect1 = &(*seq_1);
		for(vector<HdRect>::iterator seq_2 = seq_1 + 1; seq_2 != seq.end(); seq_2 ++)
		{
			HdRect rect2 = *seq_2;
			if (IsAdjacent(*rect1,rect2))
			{
				Merge(rect1,rect2);
				seq.erase(seq_2);
				flag = true;
				break;
			}
		}
		if(flag) 
		{
			seq_1 = seq.begin();
			flag = false;
		}
		else seq_1 ++;
	}

	delete line1;
	delete line2;
	
	return seq.size();
}


int fastFindContourRects2(const uchar *src,int width,int height,int widthStride,vector<HdRect> &blobList, HdSize operateSz, double ratio)
{
	if (!src) return 0;
	vector<HdRect> &seq = blobList;
	vector<HdRect> *line1 = new vector<HdRect>;  //存储上一个分析的高度为h的行图像中的前景矩形
	vector<HdRect> *line2 = new vector<HdRect>;   //存储当前分析的高度为h的行图像中的前景矩形
	const uchar *p = src;
	size_t step = widthStride;
	int b_width = width;
	int b_height = height;
	int w = operateSz.width, h = operateSz.height;
	double thresh = w*h*255.0*ratio;

	vector<HdRect>::iterator index1 = line1->begin();
	vector<HdRect>::iterator idx2 = line2->begin();

	for (int i = 0; i <= b_height-h; i += h)	//--可以取等号    根据小的操作单元，将前景图像中出现的块进行合并操作，形成初始的bloblist
	{
		line2->clear();
		for (int j = 0; j <= b_width-w; j += w)     //每次对一个高度为h的一行图像进行分析，首先分析是否2*3的小区域满足前景条件
		{                                           //如果满足，则将该矩形与之前的矩形对比，看是否邻近，邻近则合并；不论合不合并，都存放在line2中
			bool c = false;
			double tmp = 0.0;

			for (int ii = 0; ii < h; ii++)
			{
				for(int jj = 0; jj < w; jj++)
				{
					tmp += p[ii*step+j+jj];
					if (tmp >= thresh)
					{
						c = true;
						break;
					}
				}
				if (c) break;
			}

			if (c)
			{
				HdRect curRect = hdRect(j,i,w,h);	//--mark
				if (line2->size() == 0)
					line2->push_back(curRect);
				else
				{
					HdRect *last_elem = &(line2->back());
					if (IsAdjacent(*last_elem,curRect))
						Merge(last_elem,curRect);
					else
						line2->push_back(curRect);
				}
			}
		}

		index1 = line1->begin();
		idx2 = line2->begin();
		while (index1 != line1->end())                //对上一行分析line1的前景矩形与本次行分析line2的前景矩形进行合并处理，将合并的存储到line2中，并删除line1中的对应矩形
		{
			bool l2flag = true;
			for (vector<HdRect>::iterator index2 = idx2; index2 != line2->end(); index2 ++)
			{
				HdRect *elem1 = &(*index1);
				HdRect *elem2 = &(*index2);

				if (IsAdjacent(*elem1, *elem2))
				{
					Merge(elem2, *elem1);
					while(++ index2 != line2->end())
					{
						HdRect next_elem = *index2;
						if (IsAdjacent(next_elem, *elem2))
						{
							Merge(elem2, next_elem);
							index2 = line2->erase(index2);
							if (index2 != line2->begin()) index2--;
						}
						else
						{
							break;
						}
					}
					if(index2 != line2->begin()) 	idx2 = index2 - 1;
					index1 = line1->erase(index1); 
					if(index1 != line1->begin()) index1--;
					l2flag = false;
					break;//---????:? 结束line2，回到line1
				}
			}
			if (l2flag && index1!=line1->end()) index1 ++;
		}

		for (index1 = line1->begin(); index1 != line1->end(); index1 ++)    //将未合并到line2中的line1的矩形，存在bloblist中
		{
			HdRect tmp = *index1;
			if (tmp.height*tmp.width >= MINOBJSIZE)
			{
				seq.push_back(tmp);
			}

		}
		line1->clear();                                  //将line1置为line2的信息，line2清空，进行下一行分析
		{
			vector<HdRect> *tmp = line1;
			line1 = line2;
			line2 = tmp;
		}
		p += h*step;
	}

	//-- 最后一行的单元！
	for (index1 = line1->begin(); index1 != line1->end(); index1 ++)
	{
		HdRect tmp = *index1;
		if (tmp.height*tmp.width >= MINOBJSIZE)
		{
			seq.push_back(tmp);
		}
	}

	//-- 相邻的大块合并                      下面的一段代码对检测到的相邻目标做了合并处理，可以根据需求去掉这一段代码
	vector<HdRect>::iterator seq_1 = seq.begin();
	bool flag = false;
	while(seq_1 != seq.end())
	{
		HdRect *rect1 = &(*seq_1);
		for(vector<HdRect>::iterator seq_2 = seq_1 + 1; seq_2 != seq.end(); seq_2 ++)
		{
			HdRect rect2 = *seq_2;
			if (IsAdjacent(*rect1,rect2))
			{
				Merge(rect1,rect2);
				seq.erase(seq_2);
				flag = true;
				break;
			}
		}
		if(flag) 
		{
			seq_1 = seq.begin();
			flag = false;
		}
		else seq_1 ++;
	}

	delete line1;
	delete line2;
/*
	vector<HdRect> seqbig,seqsmall;
	int empty=0;
	float ratelow=0.5;
	for(vector<HdRect>::iterator sq=seq.begin();sq!=seq.end();sq++)    
	{
		for(int i=sq->x;i<=sq->x+sq->width;i++)         
		{
			for(int j=sq->y;j<=sq->y+sq->height;j++)
			{
				if(src[j*step+i]==0) empty++;
			}
		}
		if(empty*1.0/(sq->width*sq->height)<ratelow) seqbig.push_back(*sq);
		else seqsmall.push_back(*sq);
		empty=0;
	}
	seq.clear();

	vector<HdRect> seqnow;
	float coversimilar=0.2,coverhigh=0.7,coverlow=0.3;
	int posifg1=1,posifg2=1,countsame=0;
	int startcol,endcol;
	bool divid=true;

	for(vector<HdRect>::iterator sq=seqsmall.begin();sq!=seqsmall.end();sq++)
	{
		for(int j=sq->x;j<=sq->x+sq->width-1;j++)
		{
			if(divid) startcol=j,endcol=j;
			for(int i=sq->y;i<=sq->y+sq->height;i++)
			{
				if(src[i*step+j]==255) posifg1++;
				if(src[i*step+j+1]==255) posifg2++;
				if(src[i*step+j]==src[i*step+j+1]&&src[i*step+j+1]==255) countsame++;
			}
            if(j+1==sq->x+sq->width){
			    HdRect newrect=hdRect(startcol,sq->y,j+1-startcol+1,sq->height);
		        if((j+1-startcol+1)*sq->height>=MINOBJSIZE) seqnow.push_back(newrect);
			    divid=true;
			}else if((countsame*1.0/posifg1>coverhigh&&countsame*1.0/posifg2>coverhigh)
				||(abs(posifg1-posifg2)*1.0/sq->height<coversimilar&&countsame*1.0/posifg1>coverlow&&countsame*1.0/posifg2>coverlow))
			{
			    endcol=j+1;
				divid=false;
			} else {
                HdRect newrect=hdRect(startcol,sq->y,endcol-startcol+1,sq->height);
				if((endcol-startcol+1)*sq->height>=MINOBJSIZE) seqnow.push_back(newrect);
				divid=true;
			}

			posifg1=1,posifg2=1,countsame=0;
		}
		divid=true;
	}
    seqsmall.clear();

	vector<HdRect> seqfinal;
	for(vector<HdRect>::iterator sq=seqnow.begin();sq!=seqnow.end();sq++)
	{
		for(int j=sq->y;j<=sq->y+sq->height-1;j++)
		{
			if(divid) startcol=j,endcol=j;
			for(int i=sq->x;i<=sq->x+sq->width;i++)
			{
				if(src[j*step+i]==255) posifg1++;
				if(src[(j+1)*step+i]==255) posifg2++;
				if(src[j*step+i]==src[(j+1)*step+i]&&src[j*step+i]==255) countsame++;
			}
			if(j+1==sq->y+sq->height){
				HdRect newrect=hdRect(sq->x,startcol,sq->width,j+1-startcol+1);
				if((j+1-startcol+1)*sq->width>=MINOBJSIZE) seqfinal.push_back(newrect);
				divid=true;
			}else if((countsame*1.0/posifg1>coverhigh&&countsame*1.0/posifg2>coverhigh)
				||(abs(posifg1-posifg2)*1.0/sq->height<coversimilar&&countsame*1.0/posifg1>coverlow&&countsame*1.0/posifg2>coverlow))
			{
				endcol=j+1;
				divid=false;
			} else {
				HdRect newrect=hdRect(sq->x,startcol,sq->width,endcol-startcol+1);
				if((endcol-startcol+1)*sq->width>=MINOBJSIZE) seqfinal.push_back(newrect);
				divid=true;
			}

			posifg1=1,posifg2=1,countsame=0;
		}
		divid=true;
	}
	seqnow.clear();

	int noempty=0; 
	float noemptyratelow=0.3;
	for(vector<HdRect>::iterator sq=seqfinal.begin();sq!=seqfinal.end();sq++)    //去掉内部前景点很少的矩形
	{
		for(int i=sq->x;i<=sq->x+sq->width;i++)         
		{
			for(int j=sq->y;j<=sq->y+sq->height;j++)
			{
				if(src[j*step+i]==255) noempty++;
			}
		}
		if(noempty*1.0/(sq->width*sq->height)>noemptyratelow) seqsmall.push_back(*sq);
		noempty=0;
	}
	seqfinal.clear();

	vector<HdRect> rectbig;                                           //将序列中较小的矩形则删除不保存
	int smallarea=15,smallsize=2;
	float hwlow=0.25,hwhigh=4;
	for(vector<HdRect>::iterator sq=seqsmall.begin();sq!=seqsmall.end();sq++)
	{
	    if(sq->height*sq->width<smallarea||sq->width<smallsize||sq->height<smallsize||sq->height*1.0/sq->width<hwlow||sq->height*1.0/sq->width>hwhigh) 
		{}
        else rectbig.push_back(*sq);
	}
	for(vector<HdRect>::iterator sq=seqbig.begin();sq!=seqbig.end();sq++)
	{
		if(sq->height*sq->width<smallarea||sq->width<smallsize||sq->height<smallsize||sq->height*1.0/sq->width<hwlow||sq->height*1.0/sq->width>hwhigh) 
		{}
		else rectbig.push_back(*sq);
	}

	seq.swap(rectbig);
	rectbig.clear();
	seqbig.clear();
	seqsmall.clear();
*/

  //int region=1;                                                                  //addded by glenn 2017-9-5
  int count=0;                                                                  
  int cutx1,cutx2,cuty1,cuty2;                                                  //对选定区域进行裁剪，将每行或每列目标元素比例小于宽或高的0.4倍的将删除
  int treshod=0;
  int goal=0;  int sum=0;  int sumcount=0;
  float rate=0.25;                                                  //该参数取0.25-0.35之间鲁棒性比较好
  float uprate=0.25;
  float positionrate=1.35;        //裁剪两端有前景点，中间为空的行或列
  int position1,position2;
  bool posi1=false;

  for(vector<HdRect>::iterator sq=seq.begin();sq!=seq.end();sq++)
  {
	  cutx1=sq->x;
	  cutx2=sq->x+sq->width;
	  cuty1=sq->y;
	  cuty2=sq->y+sq->height;

	  //printf("region:%d\n",region++);
	  for(int i=sq->x;i<=sq->x+sq->width;i++)          //从左到右检查列的平均值
	  {
	  	  for(int j=sq->y;j<=sq->y+sq->height;j++)
		  {
			//printf("%d ",p[j*step+i]);
			  goal+=src[j*step+i];
			  count++;
		  }
		  //printf("average：%d\n",goal/count);
		  sum+=goal/count;  sumcount++;
		  count=0;  goal=0;
	  }
	  //printf("all average:%d\n",sum/sumcount);
	  treshod=sum/sumcount;
	  sum=0;  sumcount=0;

	  //printf("from left to right\n");
	  for(int i=sq->x;i<=sq->x+sq->width;i++)          //从左到右检查列
	  {
		  for(int j=sq->y;j<=sq->y+sq->height;j++)
		  {
			  //printf("%d ",p[j*step+i]);
			  if(src[j*step+i]>treshod)              
			  {
                  if(posi1==true) position2=j;
				  if(posi1==false) position1=j,position2=j,posi1=true;
				  count++;
			  }
		  }
		  //printf("\n");
		  if(count*1.0/sq->height>rate&&(position2-position1)*1.0/count<positionrate)        //检查个数大于阈值的比例
		  {
			  cutx1=i;
			  count=0;
			  posi1=false;
			  break;
		  }
		  posi1=false;position1=sq->y,position2=sq->y;
		  count=0;
	  }

	  //printf("from right to left\n");
	  for(int i=sq->x+sq->width;i>=sq->x;i--)         //从右到左检查列
	  {
		  for(int j=sq->y;j<=sq->y+sq->height;j++)
		  {
			  //printf("%d ",p[j*step+i]);
			  if(src[j*step+i]>treshod)              
			  {
				  if(posi1==true) position2=j;
				  if(posi1==false) position1=j,position2=j,posi1=true;
				  count++;
			  }
		  }
		  //printf("\n");
		  if(count*1.0/sq->height>rate&&(position2-position1)*1.0/count<positionrate)
		  {
			  cutx2=i;
			  count=0;
			  posi1=false;
			  break;
		  }
		  posi1=false;position1=sq->y,position2=sq->y;
		  count=0;
	  }

	  //printf("from up to bottom\n");
	  for(int i=sq->y;i<=sq->y+sq->height;i++)       //从上到下检查行    这里还要考虑头肩模型特殊情况（因为vibe算法在头肩模型处会有很大的空洞区域），为了尽量不减少上部的面积，通过比较一行中两个较大点的距离来判断是否保留位置
	  {

		  for(int j=sq->x;j<=sq->x+sq->width;j++)
		  {
			  //printf("%d ",p[i*step+j]);
			  if(src[i*step+j]>treshod)              
			  {
				  if(posi1==true) position2=j;
				  if(posi1==false) position1=j,position2=j,posi1=true;
				  count++;
			  }
		  }

		  if(count*1.0/sq->width>uprate&&(position2-position1)*1.0/count<positionrate)
		  {
			  cuty1=i;
			  count=0;
			  posi1=false;
			  break;
		  }
		   posi1=false;position1=sq->x,position2=sq->x;
		  count=0;
	  }

	  //printf("from bottom to up\n");
	  for(int i=sq->y+sq->height;i>=sq->y;i--)      //从下到上检查列
	  {
		  for(int j=sq->x;j<=sq->x+sq->width;j++)
		  {
			  //printf("%d ",p[i*step+j]);
			  if(src[i*step+j]>treshod)              
			  {
				  if(posi1==true) position2=j;
				  if(posi1==false) position1=j,position2=j,posi1=true;
			 	  count++;
			  }
		  }
		  //printf("\n");
		  if(count*1.0/sq->width>rate&&(position2-position1)*1.0/count<positionrate)
		  {
			  cuty2=i;
			  count=0;
			  posi1=false;
			  break;
		  }
		  count=0;
		   posi1=false;position1=sq->x,position2=sq->x;
	  }

	  sq->x=cutx1;
	  sq->y=cuty1;
	  sq->width=cutx2-cutx1;
	  sq->height=cuty2-cuty1;
  }


#ifdef DEBUGUSE
  IplImage* maskImg = cvCreateImage( cvSize(width*4,height*4), IPL_DEPTH_8U, 1 );
  for(vector<HdRect>::iterator it=seq.begin();it!=seq.end();it++)
  {
	  cvRectangle(maskImg,Point(it->x*4,it->y*4),Point((it->x+it->width)*4,(it->y+it->height)*4),Scalar(0,255,0),2);
  }
  cvShowImage( "mask2", maskImg );
  cvWaitKey( 1 );
  cvReleaseImage( &maskImg );
#endif

	return seq.size();
}

