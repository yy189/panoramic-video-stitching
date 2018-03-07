#ifndef CYLINMAP2_H
#define CYLINMAP2_H
#include <cmath>
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <thread>
#include <mutex>

struct Position
{
	double x;
	double y;
	Position(double x1, double y1){ x = x1; y = y1; }
};

//注意 这里默认处理的是6个摄像头的6张图像
class CylindricalMapping2
{
public:
	CylindricalMapping2(double fvalue, cv::Mat* imageIn);
	Position mappingPointCaculate(int newx1, int newy1);//获取柱面上一点对于的图像上的位置
	bool cylindricalMapThread(int hBegin, int hEnd, int wBegin, int wEnd, int picIndex);//为提高柱面映射速度 开辟多线程,参数分别表示此线程处理得到的柱面映射部分的开始位置和结束位置，最后一个参数表示当前处理的图像序号（0—5）
	bool cylindricalMapping();//将原始图柱面映射相应的柱面图，保存到out中，返回是否成功
	cv::Mat* getMapresult();//返回的多组映射图像out
	~CylindricalMapping2();
private:
	int width, height;//实景图像的宽和高
	int mapWidth, mapHeight;
	double f;//多个图像对应的焦距
	cv::Mat out[6];//用于存储映射到柱面上的6张图像
	cv::Mat*  in;//用于保存原始6张图像的地址
	double angle;//用于保存每次计算圆柱映射点时对应的角度
	double angle2;//用于保存原始图像相对中心的角度的一半
	std::mutex mut;//锁
};
#endif