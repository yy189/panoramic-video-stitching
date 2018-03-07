#ifndef CHONGHESTITCH_H
#define CHONGHESTITCH_H
#include <iostream>
#include <cv.h>
#include <cmath>
class ChongheStitch{
public:
	cv::Mat chongHeStitch(cv::Mat& img1, cv::Mat& img2, int chongheWidth);//完成img1和img2的融合（这是两个相邻的已经融合过得图像区中间部分得到的区域），最后一个参数表示重合区域宽度
	//为使用多线程，融合过程中为使得融合的图像能够在主线程中始终存在，而更改的版本，也是为了后续融合而做的更改
	void chongHeStitch3Init(cv::Mat& img1, cv::Mat& img2, int chongheWidth, int& resultWidth, int& resultHeight,int& central1, int& central2);
	void chongHeStitch3(cv::Mat& img1, cv::Mat& img2, cv::Mat& imgResult, int chongheWidth, int& central1, int& central2);//和上一个函数功能相同，central1和central2分别表示img1和img2拼接的边缘处有像素区域中间位置，具体可以看上一个函数
};
#endif

