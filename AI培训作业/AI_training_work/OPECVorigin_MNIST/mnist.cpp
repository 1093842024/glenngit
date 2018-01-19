#include "mnist.h"

//计时器
double cost_time;
clock_t start_time;
clock_t end_time;

//测试item个数
int testNum = 10000;

int reverseInt(int i) {
	unsigned char c1, c2, c3, c4;

	c1 = i & 255;
	c2 = (i >> 8) & 255;
	c3 = (i >> 16) & 255;
	c4 = (i >> 24) & 255;

	return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
}

Mat read_mnist_image(const string fileName) {
	int magic_number = 0;
	int number_of_images = 0;
	int n_rows = 0;
	int n_cols = 0;

	Mat DataMat;

	ifstream file(fileName, ios::binary);
	if (file.is_open())
	{
		//cout << "成功打开图像集 ... \n";

		file.read((char*)&magic_number, sizeof(magic_number));
		file.read((char*)&number_of_images, sizeof(number_of_images));
		file.read((char*)&n_rows, sizeof(n_rows));
		file.read((char*)&n_cols, sizeof(n_cols));
		//cout << magic_number << " " << number_of_images << " " << n_rows << " " << n_cols << endl;

		magic_number = reverseInt(magic_number);
		number_of_images = reverseInt(number_of_images);
		n_rows = reverseInt(n_rows);
		n_cols = reverseInt(n_cols);
		//cout << "MAGIC NUMBER = " << magic_number<< " ;NUMBER OF IMAGES = " << number_of_images<< " ; NUMBER OF ROWS = " << n_rows<< " ; NUMBER OF COLS = " << n_cols << endl;

		//-test-
		//number_of_images = testNum;
		//输出第一张和最后一张图，检测读取数据无误
		Mat s = Mat::zeros(n_rows, n_rows * n_cols, CV_32FC1);
		Mat e = Mat::zeros(n_rows, n_rows * n_cols, CV_32FC1);

		cout << "开始读取Image数据......\n";
		start_time = clock();
		DataMat = Mat::zeros(number_of_images, n_rows * n_cols, CV_32FC1);
		for (int i = 0; i < number_of_images; i++) {
			for (int j = 0; j < n_rows * n_cols; j++) {
				unsigned char temp = 0;
				file.read((char*)&temp, sizeof(temp));
				float pixel_value = float((temp + 0.0) / 255.0);
				DataMat.at<float>(i, j) = pixel_value;

				//打印第一张和最后一张图像数据
				if (i == 0) {
					s.at<float>(j / n_cols, j % n_cols) = pixel_value;
				}
				else if (i == number_of_images - 1) {
					e.at<float>(j / n_cols, j % n_cols) = pixel_value;
				}
			}
		}
		end_time = clock();
		cost_time = (end_time - start_time) / CLOCKS_PER_SEC;
		cout << "读取Image数据完毕......" << cost_time << "s\n";

		//imshow("first image", s);
		//imshow("last image", e);
        //printf("image.cols:%d,image.rows:%d\n",s.cols,s.rows);
		//waitKey(0);
	}
	file.close();
	return DataMat;
}

Mat read_mnist_label(const string fileName) {
	int magic_number;
	int number_of_items;

	Mat LabelMat;

	ifstream file(fileName, ios::binary);
	if (file.is_open())
	{
		//cout << "成功打开Label集 ... \n";

		file.read((char*)&magic_number, sizeof(magic_number));
		file.read((char*)&number_of_items, sizeof(number_of_items));
		magic_number = reverseInt(magic_number);
		number_of_items = reverseInt(number_of_items);

		//cout << "MAGIC NUMBER = " << magic_number << "  ; NUMBER OF ITEMS = " << number_of_items << endl;

		//-test-
		//number_of_items = testNum;
		//记录第一个label和最后一个label
		unsigned int s = 0, e = 0;

		cout << "开始读取Label数据......\n";
		start_time = clock();
		LabelMat = Mat::zeros(number_of_items, 1, CV_32SC1);
		for (int i = 0; i < number_of_items; i++) {
			unsigned char temp = 0;
			file.read((char*)&temp, sizeof(temp));
			LabelMat.at<unsigned int>(i, 0) = (unsigned int)temp;

			//打印第一个和最后一个label
			if (i == 0) s = (unsigned int)temp;
			else if (i == number_of_items - 1) e = (unsigned int)temp;
		}
		end_time = clock();
		cost_time = (end_time - start_time) / CLOCKS_PER_SEC;
		cout << "读取Label数据完毕......" << cost_time << "s\n";

		//cout << "first label = " << s << endl;
		//cout << "last label = " << e << endl;
	}
	file.close();
	return LabelMat;
}

