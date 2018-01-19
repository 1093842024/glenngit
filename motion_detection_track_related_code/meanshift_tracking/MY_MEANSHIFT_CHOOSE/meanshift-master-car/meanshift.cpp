#include "meanshift.h"

//���캯��
MeanShift::MeanShift()
{ 
	cfg._Maxiter = 20;								//����������Ϊ10
	cfg._num_bins = 16;								//����16*16*16ֱ��ͼ����ɫͼƬ��������ɫ��ÿ����ɫ�ֳ�16�����䣩
	cfg._piexl_range = 256;							//����ֵ����256����
	_bin_width = cfg._piexl_range / cfg._num_bins;	//ÿ������Ŀ��
	_flag_overflow = false;
}

//�����ʼ����Ŀ��ģ�ͼ��㣬��������������Բ�������Ϊ���ò���
void MeanShift::Ini_target_frame(Mat & frame, Rect & rec)
{
	_frame_cols = frame.cols;	//������Ƭ�Ĺ�񣨳ߴ磩
	_frame_rows = frame.rows;		
	_target_region = rec;		//�����ѡ�ľ���Ŀ������rec�Ĳ�����������ͼ����ԣ�
	_rec_binValue = Mat::zeros(rec.height, rec.width, CV_32F);
	_target_model = Calpdfmodel(frame,_target_region);
}

//����Epanechnikov�ˣ����غ˵�����Ԫ�صĺͣ�ע�������kernel����Ϊ���ò������ú�����Calpdfmodel�������ã�
//���øú���֮ǰ��Ҫ�ȶ���kernel������ָ��kernel��ά����ά������ѡ�ľ��������Сһ��
float MeanShift::Epanechnikov_kernel(Mat & kernel)
{
	float cd = PI;							//����[M.P.Wand,M.C.Jones]Kernel Smoothing P105����cd�������漰gamma�����������ѧ����
	int d = 2;								//����ֻ��x,y�������꣬2ά��z=(x,y)
	int rows = kernel.rows;
	int cols = kernel.cols;
	float x, y, normz, sum_kernel = 0.0;
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < cols;j++)
		{
			x = static_cast<float>(j - cols / 2);
			y = static_cast<float>(i - rows / 2);
			//normz = (x*x + y*y) / (rows*rows / 4 + cols*cols / 4);	//Ϊ��ʽ��13��ͳһ�������������ķ���
			normz = (x*x) / (rows*rows / 4) + (y*y) / (cols*cols / 4);	//z=(x,y)�������normz��ʵ��norm(z)��ƽ��
			kernel.at<float>(i, j) = normz <= 1 ? 0.5 / cd*(d + 2)*(1.0 - normz) : 0;
			
			sum_kernel += kernel.at<float>(i, j);
		}
	}
	return sum_kernel;
}

//����ʽ��2����3������ģ�ͣ�Ŀ�꼰��ѡģ�Ͷ��ǵ��øú���������������Ҳ���Բ�����Ϊ���ò���
Mat MeanShift::Calpdfmodel(const Mat & frame, const Rect & rec)
{
	Mat kernel(rec.height, rec.width, CV_32F, cv::Scalar(1e-10));	//�˳�ʼ����ע���ʼ��ֵΪ1e-10������ʽ��3����������ĸ����Ȼ�˴������ܳ�����Ҫ�������ϰ��
	float C = 1 / Epanechnikov_kernel(kernel);						//����ʽ��3������

	int bins = cfg._num_bins*cfg._num_bins*cfg._num_bins;			//ֱ��ͼ��ά������������ά������ά��ɫתΪ������һά��ɫֵ��������ֱ��ͼҲ���һά
	//Mat pdf_model = Mat::zeros(1, bins, CV_32F);					
	Mat pdf_model(1, bins, CV_32F, cv::Scalar(1e-10));				//ֱ��ͼ��ʼ������Ҫ�Ǵ�С��ʼ��
	int bin_value = 0;												//����������ÿ��Ԫ�ض�Ӧ����������
	//std::cout << "Part of the frame are:" << std::endl << frame(rec) << std::endl << std::endl;
	std::vector<cv::Mat> bgr_plane;									//��ɫͼ˺�ֳ�����ƽ��ͼ
	cv::split(frame, bgr_plane);
	//std::cout << "Part of the bgr_plane0 are:" << std::endl << bgr_plane[0](rec) << std::endl << std::endl;
	//std::cout << "Part of the bgr_plane1 are:" << std::endl << bgr_plane[1](rec) << std::endl << std::endl;
	//std::cout << "Part of the bgr_plane2 are:" << std::endl << bgr_plane[2](rec) << std::endl << std::endl;
	int row_index = rec.y;
	int col_index;
	int rows = rec.height;
	int cols = rec.width;

	Mat mixImg = Mat::zeros(rows,cols,CV_32F);						//��ά��ɫͼתΪ��Ӧ��һά��������ɫͼ�������ά����ָ������ɫ��
	int tempb, tempg, tempr;

	for (int i = 0;i < rows;i++)
	{
		col_index = rec.x;

		if (col_index<0 || col_index>_frame_cols - 1 || row_index<0 || row_index>_frame_rows - 1)
		{
			_flag_overflow = true;
			cout << "Warning: OVERFLOW!! The calculated location of the target is out of the picture!" << endl;
			break;
		}

		uchar *pb = bgr_plane[0].ptr<uchar>(row_index);
		uchar *pg = bgr_plane[1].ptr<uchar>(row_index);
		uchar *pr = bgr_plane[2].ptr<uchar>(row_index);
		int *pmixImg = mixImg.ptr<int>(i);

		//std::cout << "The row index is: " << row_index << std::endl;
		for (int j = 0;j < cols;j++)
		{
			int b = pb[col_index];
			//std::cout << "0(b) is: " << b << std::endl;

			tempb = pb[col_index] / 16;
			tempg = pg[col_index] / 16;
			tempr = pr[col_index] / 16;

			pmixImg[j] = tempb * 256 + tempg * 16 + tempr;		//ʹ�ù�ʽblue*256+green*16+red��������Ҫ�ȰѸ�����ɫ���ֳ�16������
			//pmixImg[j] = tempb / 16 + tempg + tempr * 16;		//ʹ�ù�ʽblue*256+green*16+red��������Ҫ�ȰѸ�����ɫ���ֳ�16������

			//bin_value = pmixImg[j] / _bin_width;				//���ݳ�������ɫֵ����������ֱ��ͼ���䣬bin_value��float����
			bin_value = pmixImg[j];
			_rec_binValue.at<float>(i, j) = bin_value;					//_rec_binValue��������Ԫ�ض�Ӧ��ֱ��ͼ������������Calweight�����н�����ʹ��

			pdf_model.at<float>(0, bin_value) += C*kernel.at<float>(i, j);		//��Ӧ��ֱ��ͼ�������ֵ������bin_value���Զ�תΪint

			col_index++;
		}
		std::cout << std::endl << std::endl;
		row_index++;
	}
	
	return pdf_model;
}

//����ʽ��10������Ȩ�أ�����Ĳ�������������Ϊ�����ò������������û������frame����������Ϊ�Ѿ���_rec_binValue������Ҫframe�ˣ�
//��������Ҫ�Ľ��������꣬�����Ϣrec�����ṩ����˲���Ҫframe��
Mat MeanShift::Calweight(const Rect & rec, Mat target_model, Mat candidate_model)
{
	Mat weight(rec.height, rec.width, CV_32F, cv::Scalar(1e-10));		//ȡֵ�����ʼ��������������Сһ��
	int bin_value;
	float* ptarget = target_model.ptr<float>(0);
	float* pcandidate = candidate_model.ptr<float>(0);

	//�������������Ԫ�ر�������ø���Ԫ�����۵�Ȩ��
	for (int i = 0;i < rec.height;i++)
	{
		float* pweight = weight.ptr<float>(i);

		for (int j = 0;j < rec.width;j++)
		{
			bin_value = _rec_binValue.at<float>(i, j);
			pweight[j] = sqrt(ptarget[bin_value] / pcandidate[bin_value]);
		}
	}
	return weight;
}

//ִ�и���
Rect MeanShift::track(Mat next_frame)
{
	Rect next_rec;

	for (int iter = 0;iter < cfg._Maxiter;iter++)
	{
		next_rec = _target_region;
		float delta_x = 0.0;
		float delta_y = 0.0;
		float normlized_x;
		float normlized_y;
		float flag;

		Mat candidate_model = Calpdfmodel(next_frame, _target_region);

		if (_flag_overflow)
			break;

		Mat weight = Calweight(_target_region, _target_model, candidate_model);
		float center_row = static_cast<float>((weight.rows - 1) / 2.0);
		float center_col = static_cast<float>((weight.cols - 1) / 2.0);
		float sum_weight = 0.0;

		for (int i = 0;i < weight.rows;i++)
		{
			normlized_y = static_cast<float>(i - center_row) / center_row;
			for (int j = 0;j < weight.cols;j++)
			{
				normlized_x = static_cast<float>(j - center_col) / center_col;
				flag = normlized_x*normlized_x + normlized_y*normlized_y>1 ? 0.0 : 1.0;
				delta_x += static_cast<float>(normlized_x*weight.at<float>(i, j)*flag);
				delta_y += static_cast<float>(normlized_y*weight.at<float>(i, j)*flag);

				sum_weight += static_cast<float>(weight.at<float>(i, j)*flag);
			}
		}

		next_rec.x += static_cast<float>(delta_x / sum_weight*center_col);	//֮ǰ��Ϊ�����˹�һ����ע�������ʵ��ƫ����Ҫ�˻�center_row
		next_rec.y += static_cast<float>(delta_y / sum_weight*center_row);

		if (abs(next_rec.x - _target_region.x) < 1 && abs(next_rec.y - _target_region.y) < 1)
			break;
		else
		{
			_target_region.x = next_rec.x;
			_target_region.y = next_rec.y;
		}
	}
	return next_rec;
}
