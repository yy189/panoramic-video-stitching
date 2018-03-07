#include "yuv2jpg偏白.h"
#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <fstream>
#include <io.h>

using namespace std;
using namespace cv;

#define FCount 1000//大小随意
#define ISizeX 1280
#define ISizeY 720

unsigned char Y[FCount][ISizeX][ISizeY];
unsigned char buf[FCount][ISizeY / 2][ISizeX / 2];
unsigned char buf2[FCount][ISizeY / 2][ISizeX / 2];


// 将图片文件写入
void FileWriteFrames(char* foldername)
{
	char yuvpath[100] = "";
	sprintf(yuvpath, "%s%s", foldername, ".yuv");
	FILE* file = fopen(yuvpath, "rb");
	int size;
	if (file)
	{
		size = _filelength(_fileno(file));
		size /= (ISizeX * ISizeY * 1.5);
		fclose(file);
	}
	ifstream readMe(yuvpath, ios::in | ios::binary);//打开并读yuv数据
	IplImage *image, *rgbimg, *yimg, *uimg, *vimg, *uuimg, *vvimg;
	rgbimg = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 3);
	image = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 3);

	yimg = cvCreateImageHeader(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 1);//亮度分量
	uimg = cvCreateImageHeader(cvSize(ISizeX / 2, ISizeY / 2), IPL_DEPTH_8U, 1);//这两个都是色度分量
	vimg = cvCreateImageHeader(cvSize(ISizeX / 2, ISizeY / 2), IPL_DEPTH_8U, 1);

	uuimg = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 1);
	vvimg = cvCreateImage(cvSize(ISizeX, ISizeY), IPL_DEPTH_8U, 1);
	int nframes;
	for (nframes = 0; nframes < size; nframes++)
	{
		char nframesstr[50];
		readMe.read((char*)Y[nframes], ISizeX*ISizeY);
		readMe.read((char*)buf[nframes], ISizeX / 2 * ISizeY / 2);
		readMe.read((char*)buf2[nframes], ISizeX / 2 * ISizeY / 2);
		cvSetData(yimg, Y[nframes], ISizeX);
		cvSetData(uimg, buf[nframes], ISizeX / 2);
		cvSetData(vimg, buf2[nframes], ISizeX / 2);

		cvResize(uimg, uuimg, CV_INTER_LINEAR);
		cvResize(vimg, vvimg, CV_INTER_LINEAR);
		cvMerge(yimg, vvimg, uuimg, NULL, image);//合并单通道为三通道
		cvCvtColor(image, rgbimg, CV_YUV2BGR);

		stringstream ss;//类型转换统一转换为char* 类型
		ss << foldername;
		ss << "//";
		ss << nframes;//文件名从0开始计数
		ss << ".jpg";
		cout << "写出去1帧" << endl;
		ss >> nframesstr;
		cvSaveImage(nframesstr, rgbimg);
	}
	readMe.close();
	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);
	cvReleaseImage(&image);
}


