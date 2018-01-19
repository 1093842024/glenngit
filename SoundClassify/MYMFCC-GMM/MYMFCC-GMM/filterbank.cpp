#include "filterbank.h"
//#define WRITESAMPLE

#ifdef WRITESAMPLE
#include <iostream>
#include <fstream>
#include <assert.h>
#endif


/*读取PCM文件到wavdata中*/
void LoadFile(const char *s, struct Wave *w)
{
    FILE *fp;
    fp = fopen(s, "rb");           //打开地址在s中的音频文件
    if (!fp)
    {
        printf("can not open this file\n");
		_sleep(5000);
        exit(0);
    }
    unsigned char ch1, ch2, ch3, ch4;  int i;
    for (i = 0; i < 40; i++)
    {
        ch1 = fgetc(fp);           //从流中读取一个字符，并增加文件指针的位置。
    }
     ch1 = fgetc(fp); ch2 = fgetc(fp); ch3 = fgetc(fp); ch4 = fgetc(fp);
    w->nSample = (ch2 * 16 * 16 + ch1) + (ch4 * 16 * 16 + ch3) * 16 * 16 * 16 * 16;   //根据读入的信息计算真实的采样点个数
    w->nSample /= 2;
//    printf(" sample dot num:%ld\n", w->nSample);                 
	if(w->nSample>RESERVESAMPLEDOT)
	{   printf("sample dot num exceed maxnum\n");
	    _sleep(5000);
	    exit(0);
	}else if(w->nSample<w->frSize){	
		printf("sample dot num less than minnum\n");
		_sleep(5000);
	    exit(0);}

	//delete []w->wavdata;

	w->wavdata = new float[w->nSample]; 
    float *buf;                    
    buf = w->wavdata;              //创建一个buf指针，指向数据存储参数w->wavdata
    w->nRow = (w->nSample - w->frSize) / w->frRate + 1;   //根据真实的采样点个数，更新需要的帧率
	
#ifdef WRITESAMPLE
	string writefile="writesample.txt";
	ofstream out_to_svm(writefile);
	assert(out_to_svm);
#endif

    for (i = 0; i<w->nSample; i++)                                          //对于每个采样点，进行数据大小转换，并存入w->wavdata中
    {
        ch1 = fgetc(fp); //每次读取两个字符，存在数组ch中  低两位
        ch2 = fgetc(fp);                                 //高两位
        //if (i % 8 == 0)      //每行输出16个字符对应的十六进制数
        //printf("\n");
        float temp = ch2 * 16 * 16 + ch1;
		//printf("%f   ",temp);
        if(temp<32768){                //如果高位为0
            //printf("%f ", temp);
            *buf++ = temp;
            //w->wavdata[i]=temp;
			//printf("%lf ", (float)(ch2 * 16 * 16 + ch1) / 32767);
        }
        else{                        //如果高位为1
            //printf("%f ",temp - 65535 - 1);
#ifdef PROCESSBYTEORIGIN
            *buf++ = temp - 65535 - 1;
#else
            *buf++ = 32767-temp;
#endif
			        
            //w->wavdata[i]=32768-temp;
			//printf("%lf ", (float)((ch2 * 16 * 16 + ch1)-65535-1) / 32768);
         }
		//if(i%50==0)  printf("%d ",(int)w->wavdata[i]);
		//printf("%d %d       ",ch1,ch2);
#ifdef WRITESAMPLE
		out_to_svm<<(int)ch1<<" "<<(int)ch2<<" ";
#endif
    }
#ifdef WRITESAMPLE
        out_to_svm.close();
#endif
    fclose(fp);
}

struct wav_struct
{
	unsigned long file_size;        //文件大小
	unsigned short channel;            //通道数
	unsigned long frequency;        //采样频率
	unsigned long Bps;                //Byte率
	unsigned short sample_num_bit;    //一个样本的位数
	unsigned long data_size;        //数据大小
	unsigned char *data;            //音频数据 ,这里要定义什么就看样本位数了，我这里只是单纯的复制数据
};
void LoadFile2(char *s, struct Wave *w)
{
	fstream fs;
	wav_struct WAV;

	fs.open(s, ios::binary | ios::in);
	//    fs.seekg(0x04);                //从文件数据中获取文件大小
	//    fs.read((char*)&WAV.file_size,sizeof(WAV.file_size));
	//    WAV.file_size+=8;
	fs.seekg(0, ios::end);        //用c++常用方法获得文件大小
	WAV.file_size = fs.tellg();

	fs.seekg(0x14);
	fs.read((char*)&WAV.channel, sizeof(WAV.channel));
	fs.seekg(0x18);
	fs.read((char*)&WAV.frequency, sizeof(WAV.frequency));
	fs.seekg(0x1c);
	fs.read((char*)&WAV.Bps, sizeof(WAV.Bps));
	fs.seekg(0x22);
	fs.read((char*)&WAV.sample_num_bit, sizeof(WAV.sample_num_bit));
	fs.seekg(0x28);
	fs.read((char*)&WAV.data_size, sizeof(WAV.data_size));

	w->nSample = WAV.data_size;   //根据读入的信息计算真实的采样点个数
	w->nSample /= 2;
	printf("sample dot num:%ld\n", w->nSample);
	w->nRow = (w->nSample - w->frSize) / w->frRate + 1;  

	WAV.data = new unsigned char[WAV.data_size];

	fs.seekg(0x2c);
	fs.read((char *)WAV.data, sizeof(char)*WAV.data_size);

	cout << "文件大小为  ：" << WAV.file_size << endl;
	cout << "音频通道数  ：" << WAV.channel << endl;
	cout << "采样频率    ：" << WAV.frequency << endl;
	cout << "Byte率      ：" << WAV.Bps << endl;
	cout << "样本位数    ：" << WAV.sample_num_bit << endl;
	cout << "音频数据大小：" << WAV.data_size << endl;

	for (unsigned long i =0; i<WAV.data_size; i = i + 2)
	{       
		//右边为大端
		unsigned long data_low = WAV.data[i];
		unsigned long data_high = WAV.data[i + 1];
		double data_true = data_high * 256 + data_low;
		//printf("%d ",data_true);
		long data_complement = 0;       
		//取大端的最高位（符号位）
		int my_sign = (int)(data_high / 128);
		//printf("%d ", my_sign);
		if (my_sign == 1)
		{
			data_complement = data_true - 65536;    //******************************这里可以进行修改 改为32678-datatrue
		}
		else
		{
			data_complement = data_true;
		}
		//printf("%d ", data_complement);
		//setprecision(4); //double float_data = (double)(data_complement/(double)32768);    
		//printf("%f ", float_data);
		w->wavdata[i/2]=data_complement;
	}
	fs.close();
	delete[] WAV.data;
}

typedef struct wave_tag  
{  
	char ChunkID[4];                    // "RIFF"标志                                                  4字节
	unsigned int ChunkSize;     // 文件长度(WAVE文件的大小, 不含前8个字节)                             4字节
	char Format[4];                     // "WAVE"标志                                                  4字节
	char SubChunk1ID[4];                // "fmt "标志                                                  4字节
	unsigned int SubChunk1Size; // 过渡字节(不定)                                                      4字节
	unsigned short int AudioFormat;     // 格式类别(10H为PCM格式的声音数据)                            2字节
	unsigned short int NumChannels;     // 通道数(单声道为1, 双声道为2)                                2字节
	unsigned int SampleRate;      // 采样率(每秒样本数), 表示每个通道的播放速度                        4字节
	unsigned int ByteRate;          // 波形音频数据传输速率, 其值为:通道数*每秒数据位数*每样本的数据位数/8       4字节
	unsigned short int BlockAlign;      // 每样本的数据位数(按字节算), 其值为:通道数*每样本的数据位值/8          2字节
	unsigned short int BitsPerSample;   // 每样本的数据位数, 表示每个声道中各个样本的数据位数.                   2字节                                                                                         
	char SubChunk2ID[4];                // 数据标记"data"                                                        4字节
	unsigned int SubChunk2Size; // 语音数据的长度                                                                4字节
} WAVE;
void LoadFile3(char *s, struct Wave *w)
{
	FILE *fp;  
	WAVE wav;  
	fp=fopen(s,"rb");   
	fread(&wav, sizeof(struct wave_tag), 1, fp);  
	printf("ChunkID---->%s\n",         wav.ChunkID);         // "RIFF"标志  
	printf("ChunkSize---->%ld\n",      wav.ChunkSize);      // 文件长度(WAVE文件的大小, 不含前8个字节)  
	printf("Format---->%s\n",          wav.Format);          // "WAVE"标志  
	printf("SubChunk1ID---->%s\n",     wav.SubChunk1ID);     // "fmt "标志  
	printf("SubChunk1Size---->%ld\n", wav.SubChunk1Size);    // 过渡字节(不定)  
	printf("AudioFormat---->%d\n",     wav.AudioFormat);     // 格式类别(10H为PCM格式的声音数据)  
	printf("NumChannels---->%d\n",     wav.NumChannels);     // 通道数(单声道为1, 双声道为2)  
	printf("SampleRate---->%ld\n",     wav.SampleRate);      // 采样率(每秒样本数), 表示每个通道的播放速度  
	printf("ByteRate---->%ld\n",       wav.ByteRate);       // 波形音频数据传输速率, 其值为:通道数*每秒数据位数*每样本的数据位数/8  
	printf("BlockAlign---->%d\n",      wav.BlockAlign);      // 每样本的数据位数(按字节算), 其值为:通道数*每样本的数据位值/8  
	printf("BitsPerSample---->%d\n",   wav.BitsPerSample);   // 每样本的数据位数, 表示每个声道中各个样本的数据位数.   
	printf("SubChunk2ID---->%s\n",     wav.SubChunk2ID);    // 数据标记"data"  
	printf("SubChunk2Size---->%ld\n", wav.SubChunk2Size);
	fclose(fp);
}

void LoadServerFile(const char *data, int len, struct Wave *w)
{
	w->nSample =len/2; 
	w->nRow = (w->nSample - w->frSize) / w->frRate + 1;   //根据真实的采样点个数，更新需要的帧率
    w->wavdata = new float[w->nSample]; 
    float *buf;                    
    buf = w->wavdata; //创建一个buf指针，指向数据存储参数w->wavdata
    //printf(" sample dot num:%ld\n", w->nSample);                 
	if(w->nSample>RESERVESAMPLEDOT)
	{   printf("sample dot num exceed maxnum\n");
	    _sleep(5000);
	    exit(0);
	}else if(w->nSample<w->frSize){	
		printf("sample dot num less than minnum\n");
		_sleep(5000);
	    exit(0);}
	                                  //char* const 表示指针的值不可变，但指向的目标的值可变     
	const char *s=data;               //const char * 表示指针指向的目标空间的内容不可变化，但是指针（即地址）可以变化 等同于 char const*
	unsigned char ch1, ch2;
    for (int i = 0; i<w->nSample; i++)                    //对于每个采样点，进行数据大小转换，并存入w->wavdata中
    {
        ch1 = *s; s++;//每次读取两个字符，存在数组ch中  低两位
        ch2 = *s; s++;                               //高两位
        float temp = ch2 * 16 * 16 + ch1;
        if(temp<32768){                            //如果高位为0
            *buf++ = temp;
        }
        else{                                      //如果高位为1
#ifdef PROCESSBYTEORIGIN
			*buf++ = temp - 65535 - 1;
#else
			*buf++ = 32767-temp;
#endif
		}
    }
}
 
/*从wavdata中提取当前帧*/
void GetWave(float *buf, struct Wave *w)
{
    int k;
    if (w->frIdx + w->frSize > w->nSample) //最后一帧如果不够数据，则尾部补零
    {
        printf("GetWave: attempt to read past end of buffer\n");
        for (k = 0; k < w->frSize; k++)
        {
            buf[k] = 0;
        }
        for (k = 0; w->frIdx + k < w->nSample; k++)
        {
            buf[k] = w->wavdata[w->frIdx + k];
        }

    }else{
        for (k = 0; k < w->frSize; k++)
        {
            buf[k] = w->wavdata[w->frIdx + k];
        }
	}
    w->frIdx += w->frRate;     //将当前帧的起始位置进行帧移
}

void ZeroMeanFrame(float *frame)
{
    int size, i;
    float sum = 0.0, off;
    size = frame[0];
    for (i = 1; i <= size; i++) sum += frame[i];
    off = sum / size;
    for (i = 1; i <= size; i++) frame[i] -= off;
}
void PreEmphasise(float *frame, float k)
{
    int i;
    float preE = k;
    int size = frame[0];
    for (i = size; i >= 2; i--)
        frame[i] -= frame[i - 1] * preE;
    frame[1] *= 1.0 - preE;
}
void Ham(float *frame,const vector<float> &hamWin)
{
    for (int i = 1; i <= frame[0]; i++)
    {
        frame[i] *= hamWin[i];
    }
}

void initham(int frameSize,vector<float> &hamWin)  //由于每帧的数据从1-framesize，所以hamwin的分配的空间也为framesize+1
{
	float a = TPI / (frameSize - 1);
	hamWin[0]=frameSize;
	for (int i = 1; i <= frameSize; i++)
		hamWin[i] = 0.54 - 0.46 * cos(a*(i - 1));
}

float Mel(int k, float fres)
{
    return 1127 * log(1 + (k - 1)*fres);
}

float WarpFreq(float fcl, float fcu, float freq, float minFreq, float maxFreq, float alpha)
{
    if (alpha == 1.0)
        return freq;
    else {
        float scale = 1.0 / alpha;
        float cu = fcu * 2 / (1 + scale);
        float cl = fcl * 2 / (1 + scale);

        float au = (maxFreq - cu * scale) / (maxFreq - cu);
        float al = (cl * scale - minFreq) / (cl - minFreq);

        if (freq > cu)
            return  au * (freq - cu) + scale * cu;
        else if (freq < cl)
            return al * (freq - minFreq) + minFreq;
        else
            return scale * freq;
    }
}
struct FBankInfo InitFBank(struct IOConfig *cf)
{
    int numChans = NUMCHANS; int usePower = 0; int takeLogs = 1; int sampPeriod = SAMPLEPERIOD;//sampPeriod要考虑一下
    float alpha = 1; int warpLowCut = 0; int warpUpCut = 0;
    struct FBankInfo fb;
    float mlo, mhi, ms, melk;
    int k, chan, maxChan, Nby2;
    int doubleFFT = 0;


    /* Save sizes to cross-check subsequent usage */
    fb.frameSize = cf->frSize;
    fb.numChans = numChans;
    fb.sampPeriod = sampPeriod;
    fb.usePower = usePower;
    fb.takeLogs = takeLogs;
    /* Calculate required FFT size */
    fb.fftN = 2;
    while (fb.frameSize>fb.fftN)
        fb.fftN *= 2;
    if (doubleFFT)//不执行    由于之前定义该参数为0，因此不执行
        fb.fftN *= 2;
    Nby2 = fb.fftN / 2;   
    fb.fres = 1.0E7 / (sampPeriod * fb.fftN * 700.0);
    maxChan = numChans + 1;
    /* set lo and hi pass cut offs if any */
    fb.klo = 2; fb.khi = Nby2;       /* apply lo/hi pass filtering */
    mlo = 0; mhi = Mel(Nby2 + 1, fb.fres);


    /* Create vector of fbank centre frequencies */   //创建滤波器组的中心频率的向量
    //fb.cf = (float*)malloc(sizeof(float)*maxChan + 1);
	fb.cf = new float[maxChan + 1];                            //********************
    fb.cf[0] = maxChan;
    ms = mhi - mlo;
    for (chan = 1; chan <= maxChan; chan++) {
        if (alpha == 1.0) {
            fb.cf[chan] = ((float)chan / (float)maxChan)*ms + mlo;
        }
        else {
            /* scale assuming scaling starts at lopass */
            float minFreq = 700.0 * (exp(mlo / 1127.0) - 1.0);
            float maxFreq = 700.0 * (exp(mhi / 1127.0) - 1.0);
            float cf = ((float)chan / (float)maxChan) * ms + mlo;

            cf = 700 * (exp(cf / 1127.0) - 1.0);

            fb.cf[chan] = 1127.0 * log(1.0 + WarpFreq(warpLowCut, warpUpCut, cf, minFreq, maxFreq, alpha) / 700.0);
        }
    }

    /* Create loChan map, loChan[fftindex] . lower channel index */
    //fb.loChan = (float*)malloc(sizeof(float)*Nby2 + 1);
	fb.loChan = new float[Nby2 + 1];                          //************************
    fb.loChan[0] = Nby2;
    for (k = 1, chan = 1; k <= Nby2; k++){
        melk = Mel(k, fb.fres);
        if (k<fb.klo || k>fb.khi) fb.loChan[k] = -1;
        else {
            while (fb.cf[chan] < melk  && chan <= maxChan) ++chan;
            fb.loChan[k] = chan - 1;
        }
    }

    /* Create vector of lower channel weights */
    //fb.loWt = (float*)malloc(sizeof(float)*Nby2 + 1);
	fb.loWt = new float[Nby2 + 1];                              //**************************
    fb.loWt[0] = Nby2;
    for (k = 1; k <= Nby2; k++) {
        chan = fb.loChan[k];
        if (k<fb.klo || k>fb.khi) fb.loWt[k] = 0.0;
        else {
            if (chan>0)
                fb.loWt[k] = ((fb.cf[chan + 1] - Mel(k, fb.fres)) /
                (fb.cf[chan + 1] - fb.cf[chan]));
            else
                fb.loWt[k] = (fb.cf[1] - Mel(k, fb.fres)) / (fb.cf[1] - mlo);
        }
    }
    /* Create workspace for fft */
    //fb.x = (float*)malloc(sizeof(float)*fb.fftN + 1);
	fb.x = new float[fb.fftN + 1];                               //*************************
    fb.x[0] = fb.fftN;
    return fb;
}

void FFT(float *s, int invert)
{
    int ii, jj, n, nn, limit, m, j, inc, i;
    double wx, wr, wpr, wpi, wi, theta;
    double xre, xri, x;

    n = s[0];
    nn = n / 2; j = 1;
    for (ii = 1; ii <= nn; ii++) {
        i = 2 * ii - 1;
        if (j>i) {
            xre = s[j]; xri = s[j + 1];
            s[j] = s[i];  s[j + 1] = s[i + 1];
            s[i] = xre; s[i + 1] = xri;
        }
        m = n / 2;
        while (m >= 2 && j > m) {
            j -= m; m /= 2;
        }
        j += m;
    };
    limit = 2;
    while (limit < n) {
        inc = 2 * limit; theta = TPI / limit;
        if (invert) theta = -theta;
        x = sin(0.5 * theta);
        wpr = -2.0 * x * x; wpi = sin(theta);
        wr = 1.0; wi = 0.0;
        for (ii = 1; ii <= limit / 2; ii++) {
            m = 2 * ii - 1;
            for (jj = 0; jj <= (n - m) / inc; jj++) {
                i = m + jj * inc;
                j = i + limit;
                xre = wr * s[j] - wi * s[j + 1];
                xri = wr * s[j + 1] + wi * s[j];
                s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
                s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
            }
            wx = wr;
            wr = wr * wpr - wi * wpi + wr;
            wi = wi * wpr + wx * wpi + wi;
        }
        limit = inc;
    }
    if (invert)
        for (i = 1; i <= n; i++)
            s[i] = s[i] / nn;
}
void Realft(float *s)
{
    int n, n2, i, i1, i2, i3, i4;
    double xr1, xi1, xr2, xi2, wrs, wis;
    double yr, yi, yr2, yi2, yr0, theta, x;

    n = s[0] / 2; n2 = n / 2;
    theta = PI / n;
    FFT(s, 0);
    x = sin(0.5 * theta);
    yr2 = -2.0 * x * x;
    yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
    for (i = 2; i <= n2; i++) {
        i1 = i + i - 1;      i2 = i1 + 1;
        i3 = n + n + 3 - i2; i4 = i3 + 1;
        wrs = yr; wis = yi;
        xr1 = (s[i1] + s[i3]) / 2.0; xi1 = (s[i2] - s[i4]) / 2.0;
        xr2 = (s[i2] + s[i4]) / 2.0; xi2 = (s[i3] - s[i1]) / 2.0;
        s[i1] = xr1 + wrs * xr2 - wis * xi2;
        s[i2] = xi1 + wrs * xi2 + wis * xr2;
        s[i3] = xr1 - wrs * xr2 + wis * xi2;
        s[i4] = -xi1 + wrs * xi2 + wis * xr2;
        yr0 = yr;
        yr = yr * yr2 - yi  * yi2 + yr;
        yi = yi * yr2 + yr0 * yi2 + yi;
    }
    xr1 = s[1];
    s[1] = xr1 + s[2];
    s[2] = 0.0;
}
void Wave2FBank(float *s, float *fbank, struct FBankInfo info,float *mfcc)
{
    const float melfloor = 1.0;
    int k, bin;
    float t1, t2;   /* real and imag parts */
    float ek;      /* energy of k'th fft channel */

    float te = 0.0;
    for (k = 1; k <= info.frameSize; k++)
        te += (s[k] * s[k]);
    /* Apply FFT */
    for (k = 1; k <= info.frameSize; k++)
        info.x[k] = s[k];    /* copy to workspace */
	if(info.frameSize<info.fftN)
	{
        for (k = info.frameSize + 1; k <= info.fftN; k++)
          info.x[k] = 0.0;   /* pad with zeroes */
	}
    Realft(info.x);                            /* take fft */  //快速傅里叶变换

    /* Fill filterbank channels */                             //梅尔滤波器组
    int i = 0;
    for (i = 1; i <= fbank[0]; i++)
        fbank[i] = 0.0;
    for (k = info.klo; k <= info.khi; k++) {             /* fill bins */
        t1 = info.x[2 * k - 1]; t2 = info.x[2 * k];
        if (info.usePower)
            ek = t1*t1 + t2*t2;
        else
            ek = sqrt(t1*t1 + t2*t2);
        bin = info.loChan[k];
        t1 = info.loWt[k] * ek;
        if (bin > 0) fbank[bin] += t1;
        if (bin < info.numChans) fbank[bin + 1] += ek - t1;
        //printf("k:%d bin:%d info.loWt:%f fbank[bin]:%f\n", k, bin, info.loWt[k], fbank[bin]);

    }

    /* Take logs */                                            //计算每个滤波器组的对数能量
    if (info.takeLogs)                               
        for (bin = 1; bin <= info.numChans; bin++) {
            t1 = fbank[bin];
            if (t1 < melfloor) t1 = melfloor;
            fbank[bin] = log(t1);
			 //printf("fbank[%d]:%f\n", bin,fbank[bin]);
        }

	/*computing discrete cosine transform */     //DCT运算 提取前13维的MFCC特征
    for(int i=0;i<MFCCCOEF;i++) mfcc[i]=0;
    for(int i=0;i<MFCCCOEF;i++)
	{
		for(int j=1;j<=info.numChans; j++)
			mfcc[i]+=fbank[j]*cos(PI*i/(2*info.numChans)*(2*j+1));
		//printf("第%d个mfcc：%f\n",i,mfcc[i]);
	}
}

void ConvertFrame(struct IOConfig *cf, struct Wave *w,const vector<float> &ham)
{
    cf->frIdx = w->frIdx; cf->frSize = w->frSize; cf->frRate = w->frRate;
    cf->preEmph = PREEMPHA;
    ZeroMeanFrame(cf->s);//零均值处理：一帧中所有采样点都减去其均值
    PreEmphasise(cf->s, cf->preEmph);//预加重处理
    Ham(cf->s,ham);//加窗 此处可以优化，创建ham窗多次十分耗费时间,可以用全局申请ham的内存   //此处已优化
    cf->fbInfo = InitFBank(cf);
    Wave2FBank(cf->s, cf->fbank, cf->fbInfo,cf->mfcc);//提取特征维度NUMCHANS-1的fbank    利用帧数据、特征信息以及fbINfo
	delete []cf->fbInfo.cf;
	delete []cf->fbInfo.loChan;
	delete []cf->fbInfo.loWt;
	delete []cf->fbInfo.x;
}
void linkdata( struct IOConfig *cf, struct Wave *w, int k)
{
#ifdef RDATA
    for (int i = 0; i < NUMCHANS; i++)     //将每一帧获取的cf->fbank中的特征信息存入w->Rdata中
    {
        *(w->Rdata + i + (k*NUMCHANS)) = *(cf->fbank + 1 + i);
    }
#endif
	for (int i = 0; i < MFCCCOEF; i++)     //将每一帧获取的cf->mfcc中的特征信息存入w->MFCCs中
	{
		*(w->MFCCs + i + (k*MFCCCOEF)) = *(cf->mfcc+ i);
		//printf("第%d帧的mfcc的第%d个特征为%f\n",k,i,cf->mfcc[i]);
	}
	*(w->ShortEnergy+ k) = cf->shortenergy;
	//printf("第%d帧的短时能量特征为%f\n",k,cf->shortenergy);
	*(w->Zerorate+ k) = cf->zerorate;
}

void zeromean(struct Wave *w)
{
    int i, j;
    int n = w->nRow;
#ifdef RDATA
    float sum[NUMCHANS];
    for (i = 0; i < NUMCHANS; i++)
    {
        sum[i] = 0.0;
        for (j = 0; j < n; j++)
        {
            sum[i] += *(w->Rdata + j*NUMCHANS + i);
        }
        sum[i] = sum[i] / n;
    }
    for (i = 0; i < NUMCHANS*n; i++)
    {
        *(w->Rdata + i) -= sum[i%NUMCHANS];
    }
#endif
	float summfcc[MFCCCOEF];
	for (i = 0; i < MFCCCOEF; i++)
	{
		summfcc[i] = 0.0;
		for (j = 0; j < n; j++)
		{
			summfcc[i] += *(w->MFCCs + j*MFCCCOEF + i);
		}
		summfcc[i] = summfcc[i] / n;
	}
	for (i = 0; i < MFCCCOEF*n; i++)
	{
		*(w->MFCCs + i) -= summfcc[i%MFCCCOEF];
	}
	
	float sumdevmfcc[MFCCCOEF];
	for (i = 0; i < MFCCCOEF; i++)
	{
		sumdevmfcc[i] = 0.0;
		for (j = 1; j < n-1; j++)
		{
			sumdevmfcc[i] += *(w->DevMFCCs + j*MFCCCOEF + i);
		}
		sumdevmfcc[i] = sumdevmfcc[i] / (n-2);
	}
	for (i = MFCCCOEF*1; i < MFCCCOEF*(n-1); i++)
	{
		*(w->DevMFCCs + i) -= summfcc[i%MFCCCOEF];
	}
}

void calcu_mfcc_diff(struct Wave *w)
{
	int i, j;
	int n = w->nRow;
	for (i = 0; i < MFCCCOEF; i++)
	{
		for (j = 1; j < n-1; j++)
		{
			*(w->DevMFCCs + j*MFCCCOEF+ i) = (w->MFCCs[(j+1)*MFCCCOEF + i]-w->MFCCs[(j-1)*MFCCCOEF + i])/2;
		}
	}
}

struct Wave* filter_bank(const char *s,const int soundtype,const int len)
{
    /*初始化*/
    /*采样个数为16028*/
    /*由HTK，采样率为25ms，则帧长为400，默认帧移为160*/
    struct Wave *w;                                    //创建wave数据结构
    //w = (struct Wave*)malloc(sizeof(struct Wave));
	w=new Wave[sizeof(struct Wave)];                               //********  w
    w->frSize = FRAMESIZE;w->frIdx = 0; w->frRate = FRAMESHIFT;    //把帧长改为512 帧移改为256   
   if(soundtype==0) 
   {
        //w->nSample = RESERVESAMPLEDOT;                                         //假定采样点数为160000
        //w->nRow = (w->nSample - w->frSize) / w->frRate + 1;          //根据假定的采样点数（160000）计算要处理的帧数  
        //w->wavdata = (float*)malloc(sizeof(float) * w->nSample);     //根据假定的采样点数计算音频数据需要的存储空间
	    //w->wavdata = new float[w->nSample];                           //********  w
	    //LoadFile2(s, w);//LoadFile3(s, w);  //调用该接口可以显示详细的音频信息
	    LoadFile(s, w);                                              //载入地址为s的音频数据，提取其中的数据到w，其中可根据s更新采样点数、处理帧数以及存储空间
   }else if(soundtype==1) LoadServerFile(s,len, w);
	// for(int i=0;i<w->nSample;i++) printf("%.1f    ",w->wavdata[i]);

	//w->Rdata = (float*)malloc(sizeof(float)*NUMCHANS*w->nRow);   //分配存储每一帧（w->nrow）*40维度的信息
	w->Rdata = new float[NUMCHANS*w->nRow];                       //*********  w
	w->MFCCs= new float[MFCCCOEF*w->nRow];                        //*********  w
	for(int i=0;i<MFCCCOEF*w->nRow;i++) w->MFCCs[i]=0;
	w->DevMFCCs=new float[MFCCCOEF*w->nRow];
	for(int i=0;i<MFCCCOEF*w->nRow;i++) w->DevMFCCs[i]=0;
	w->ShortEnergy=new float[w->nRow];               //指针分配的空间一定不能小于操作的数量，否则会出现指针的溢出，造成堆栈的错误
	w->Zerorate=new float[w->nRow];                  //这两个量之前分配时，出现过错误，误分配为w->frSize，处理导致超过500帧以后出现错误
	w->valideframe=new int[w->nRow];

    struct IOConfig *cf;                                 //创建帧数据信息  并分配尺寸
    //cf = (struct IOConfig*)malloc(sizeof(struct IOConfig)); 
	cf = new IOConfig[sizeof(struct IOConfig)];                 //*********  cf
    //cf->s = (float*)malloc(sizeof(float) * w->frSize + 1);    //对于cf->s分配空间为 一帧+1 的数据空间大小
	cf->s = new float[w->frSize + 1];                           //*********  cf
	cf->mfcc=new float[MFCCCOEF];                               //*********  cf
	cf->fbank = new float[NUMCHANS+1];      //mel滤波器组滤波后的特征信息，第一个储存滤波器组维数信息 **********cf
	cf->fbank[0] = NUMCHANS;                //fbank[0]存储滤波器组的维度

	vector<float> hamWin(w->frSize+1); //***********  由于每帧的数据从1-framesize，所以hamwin的分配的空间也为framesize+1
    initham(w->frSize,hamWin);                               //初始化hamming窗的数据

    /*读PCM文件到wavdata中，可以直接读取缓存数据，此处是为了方便PC测试*/
	//clock_t  clockBegin, clockEnd;    clockBegin = clock();
    for (int k = 0; k < w->nRow; k++)
    {
        /*分帧,计算帧能量*/
        cf->s[0] = w->frSize;   //s[0]中存一帧的采样点数
        GetWave(cf->s + 1, w);  //从wavdata中提取当前帧到cf->s+1
        float m, e, x,mins=cf->s[1],maxs=cf->s[1];
		int j,count=0,minsnum=0,maxsnum=0;

		for (j = 2; j <= w->frSize; j++) 
		{
			if(cf->s[j]<mins) mins=cf->s[j];
			else if(cf->s[j]>maxs) maxs=cf->s[j];
		}

		float xx=cf->s[1]*hamWin[1];
        for (j = 1, m = e = 0.0; j <= w->frSize; j++) {
            x = cf->s[j]*hamWin[j];
            //e += x*x; //m += x;
			if(cf->s[j]==mins) minsnum++;
			else if(cf->s[j]==maxs && maxs!=mins) maxsnum++;
			else e+=abs(x);
			if((xx<=0&&x>0)||(xx>=0&&x<0)) count++;
			xx=x;
        }
		if(minsnum+maxsnum<w->frSize) cf->shortenergy=e/(w->frSize-minsnum-maxsnum); 
		else cf->shortenergy=0;
		if(cf->shortenergy> 1E-20)cf->shortenergy=log10(cf->shortenergy);     //计算每帧短时能量 
		else cf->shortenergy=-20;
		//printf("%f   ",cf->shortenergy);
		cf->zerorate=count*100.0/w->frSize;               //计算短时平均过零率      
		//printf("shortenergy:%.2f, zerorate:%.2f\n",cf->shortenergy ,cf->zerorate);
        // m = m / w->frSize; e = e / w->frSize - m*m;     //计算帧能量e
        //if (e>0.0) e = 10.0*log10(e / 0.32768); else e = 0.0;
        // cf->curVol = e;

         if((cf->shortenergy>LOWSHORTENERGY&&cf->zerorate>HIGHZERORATE)||(cf->shortenergy>HIGHSHORTENERGY&&cf->zerorate>LOWZERORATE))
		 {      w->valideframe[k]=1;
		 }else  w->valideframe[k]=0;

        ConvertFrame(cf, w, hamWin);/*处理*/
        linkdata(cf, w, k);
    }
	//clockEnd = clock();   printf("covert time: %dms\n", clockEnd - clockBegin);
	/* for (int i = 0; i < NUMCHANS*w->nRow; i++)   //输出了
    {
        if (i%NUMCHANS == 0)
            printf("\n第%d帧：\n", i / NUMCHANS);
        printf("%f ", *(w->Rdata + i));
    }
	for (int i = 0; i < MFCCCOEF*w->nRow; i++)   //输出了
	{
		if (i%MFCCCOEF == 0) {printf("\n第%d帧：", i / MFCCCOEF); printf("shortene:%.2f   ", *(w->ShortEnergy + i / MFCCCOEF));}
		if (i%MFCCCOEF == 0) {printf("zerorate:%.2f", *(w->Zerorate + i / MFCCCOEF));}
		//if (i%MFCCCOEF == 0) {printf("MFCCs: \n");}
		//printf("%f ", *(w->MFCCs + i));
	}*/

	delete []cf->mfcc;
	delete []cf->fbank;
	delete []cf->s;
	/*delete []cf->fbInfo.cf;
	delete []cf->fbInfo.loChan;
	delete []cf->fbInfo.loWt;
	delete []cf->fbInfo.x;*/
	delete []cf;

    calcu_mfcc_diff(w);
    zeromean(w);                 //在计算结束之后有一个零均值计算
	
    return w;  //w为指针，*w为wave类型对象
}

void deletewav(struct Wave* w)
{
	delete []w->wavdata;
	delete []w->Rdata;
	delete []w->MFCCs;
	delete []w->DevMFCCs;
	delete []w->ShortEnergy;     
	delete []w->Zerorate;
	delete []w->valideframe;
	delete []w;
}

int AudioBeginDetect(unsigned char* wave,int len,float lowshortenegy, float lowzerorate)
{
	if(len!=FRAMESIZE*2){ printf("input audio length false!\n"); return 0;}
	unsigned char ch1,ch2;
	int* s=new int[FRAMESIZE];
	for(int i=0;i<FRAMESIZE;i++)
	{
		ch1=*wave;wave++;
		ch2=*wave;wave++;
		s[i]=ch2 * 16 * 16 + ch1;
        if(s[i]>32767){                            //如果高位为1    
#ifdef PROCESSBYTEORIGIN
           s[i]=s[i] - 65535 - 1; 
#else
		   s[i]=32767-s[i];
#endif 
	    }
	}
	
	float* hamWin=new float[FRAMESIZE];
	float a = TPI / (FRAMESIZE - 1);
	for (int i = 0; i <FRAMESIZE; i++)
		hamWin[i] = 0.54 - 0.46 * cos(a*(i - 1));

	float m, e, x;
	int j,count=0;
	float xx=s[0]*hamWin[0];
	for (j = 0, m = e = 0.0; j < FRAMESIZE; j++) {
		x = s[j]*hamWin[j];
		e+=abs(x);
		if((xx<=0&&x>0)||(xx>=0&&x<0)) count++;
		xx=x;
	}
	float shortenergy,zerorate;
	shortenergy=e/FRAMESIZE; 
	if(shortenergy> 1E-20) shortenergy=log10(shortenergy);     //计算每帧短时能量 
	else shortenergy=-20;
	
	zerorate=count*100.0/FRAMESIZE;               //计算短时平均过零率     
	
	delete [] s;
	delete [] hamWin;

	
	if((shortenergy>lowshortenegy&&zerorate>HIGHZERORATE)||(shortenergy>HIGHSHORTENERGY&&zerorate>lowzerorate)){
		printf("shortenergy is %.2f,zerorate is %.2f\n",shortenergy,zerorate);
		return 1;
	}else return 0;



}


