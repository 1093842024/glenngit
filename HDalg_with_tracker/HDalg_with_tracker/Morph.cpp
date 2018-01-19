#include "Morph.h"

#define MINOBJSIZE 10
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
	vector<HdRect> *line1 = new vector<HdRect>;
	vector<HdRect> *line2 = new vector<HdRect>;
	const uchar *p = src;
	size_t step = widthStride;
	int b_width = width;
	int b_height = height;
	int w = operateSz.width, h = operateSz.height;
	double thresh = w*h*255.0*ratio;
	
	vector<HdRect>::iterator index1 = line1->begin();
	vector<HdRect>::iterator idx2 = line2->begin();

	for (int i = 0; i <= b_height-h; i += h)	//--可以取等号  利用operateSZ单元遍历图像vibe生成的图像，将单元内亮度达到比例的作为有效单元记录（比如2*3个格子中，有0.4比例的格子的像素值是255，则可以算作有效单元）
	{
		line2->clear();
		for (int j = 0; j <= b_width-w; j += w)
		{
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
		while (index1 != line1->end())
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

		for (index1 = line1->begin(); index1 != line1->end(); index1 ++)
		{
			HdRect tmp = *index1;
			if (tmp.height*tmp.width >= MINOBJSIZE)
			{
				seq.push_back(tmp);
			}

		}
		line1->clear();
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


	//-- 相邻的大块合并
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


	///下面出现过很严重的bug，对于const unsigne char *p 上面的代码中其实已经不是指向图像的最开始位置，因此下面再使用会有很大的问题
	///找到这个问题后，直接使用传入的原始指针 *src即可


	//int region=1;                                                                  //addded by glenn 2017-9-5
	int count=0;                                                                  
	int cutx1,cutx2,cuty1,cuty2;                                                  //对选定区域进行裁剪，将每行或每列目标元素比例小于宽或高的0.4倍的将删除
	int treshod=0;
	int goal=0;  int sum=0;  int sumcount=0;
	float rate=0.25;                                                  //该参数取0.25-0.35之间鲁棒性比较好
	float uprate=0.15;

	int position1=0,position2=0, firstposition=0;
	float positionrate=0.6; float positionthreshodrate=1.2;
	int positionthreshod=0;

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
					count++;
				}
			}
			//printf("\n");
			if(count*1.0/sq->height>rate)        //检查个数大于阈值的比例
			{
				cutx1=i;
				count=0;
				break;
			}
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
					count++;
				}
			}
			//printf("\n");
			if(count*1.0/sq->height>rate)
			{
				cutx2=i;
				count=0;
				break;
			}
			count=0;
		}

		//printf("from up to bottom\n");
		for(int i=sq->y;i<=sq->y+sq->height;i++)       //从上到下检查行    这里还要考虑头肩模型特殊情况（因为vibe算法在头肩模型处会有很大的空洞区域），为了尽量不减少上部的面积，通过比较一行中两个较大点的距离来判断是否保留位置
		{
			for(int j=sq->x;j<=sq->x+sq->width;j++)
			{
				goal+=src[i*step+j];
				count++;
			}
			//printf("average：%d\n",goal/count);
			positionthreshod=goal/count;
			count=0;  goal=0;

			for(int j=sq->x;j<=sq->x+sq->width;j++)
			{
				//printf("%d ",p[i*step+j]);
				if(src[i*step+j]>treshod)              
				{
					count++;
				}

				if(src[i*step+j]>positionthreshodrate*positionthreshod)
				{
					if(firstposition==0)
					{
						position1=j;
						position2=j;
						firstposition=1;
					}
					position2=j;
				}
			}
			//printf("\n"); printf("position range:%f\n",(position2-position1)*1.0/sq->width);
			if(count*1.0/sq->width>uprate)
			{
				cuty1=i;
				count=0;
				position1=0;position2=0;firstposition=0;
				break;
			}else if((position2-position1)*1.0/sq->width>positionrate)
			{
				cuty1=i;
				count=0;
				position1=0;position2=0;firstposition=0;
				break;
			}
			position1=0;position2=0;firstposition=0;
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
					count++;
				}
			}
			//printf("\n");
			if(count*1.0/sq->width>rate)
			{
				cuty2=i;
				count=0;
				break;
			}
			count=0;
		}

		sq->x=cutx1;
		sq->y=cuty1;
		sq->width=cutx2-cutx1;
		sq->height=cuty2-cuty1;
	}

	
	return seq.size();
}