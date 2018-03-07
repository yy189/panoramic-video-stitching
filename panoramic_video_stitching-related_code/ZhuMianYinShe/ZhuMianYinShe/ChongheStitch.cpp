#include "ChongheStitch.h"
#include <highgui.h>

cv::Mat ChongheStitch::chongHeStitch(cv::Mat& img1, cv::Mat& img2, int chongheWidth)
{
	int w1, w2, h1, h2;//用于分别存储img1和img2的宽和高
	int resultWidth, resultHeight;
	w1 = img1.cols;
	w2 = img2.cols;
	h1 = img1.rows;
	h2 = img2.rows;
	resultWidth = w1 + w2 - chongheWidth;
	int b1, e1, b2, e2;//b1代表img1最右边（去掉重合部分）可见图像开始点的高，e1代表其可见图像结束点高，b2对应img2最左边可见图像的开始点高，e2对应其结束点高
	uchar *p1 = img1.data;
	uchar *p2 = img2.data;
	b1 = b2 = 0;
	e1 = e2 = 0;
	p1 += img1.elemSize()*(w1 - chongheWidth);
	while (*p1 == 0 && *(p1 + 1) == 0 && *(p1 + 2) == 0)
	{
		b1++;
		p1 += img1.step;
	}
	p1 = img1.data + (h1 - 1)*img1.step + (w1 - chongheWidth)*img1.elemSize();
	while (*p1 == 0 && *(p1 + 1) == 0 && *(p1 + 2) == 0)
	{
		e1++;
		p1 -= img1.step;
	}
	e1 = h1 - e1;
	while (*p2 == 0 && *(p2 + 1) == 0 && *(p2 + 2) == 0)
	{
		b2++;
		p2 += img2.elemSize()*w2;
	}
	p2 = img2.data + (h2 - 1)*img2.step;
	while (*p2 == 0 && *(p2 + 1) == 0 && *(p2 + 2) == 0)
	{
		e2++;
		p2 -= img2.step;
	}
	e2 = h2 - e2;
	int central1 = (b1 + e1) / 2;//边缘可见区域的中点  拼接时使得img1和img2的这两个中点对齐
	int central2 = (b2 + e2) / 2;
	int frontHeight = std::max(central1, central2);//表示融合图像前半段的高
	int endHeight = std::max(h1 - central1, h2 - central2);//表示融合图像后半段的高
	resultHeight = frontHeight + endHeight;
	cv::Mat imgResult = cv::Mat::zeros(resultHeight, resultWidth, img1.type());
	uchar* p3 = imgResult.data;
	int line = 0;
	//以下分为根据img1和img2的大小和位置关系分为四种情况来进行图像融合
	p1 = img1.data;
	p2 = img2.data;
	if (frontHeight == central1 && endHeight == h1 - central1)
	{
		while (line<central1 - central2 || (line >= central1 + h2 - central2 && line<w1))
		{
			int j;
			for (j = 0; j<w1; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			line++;
		}
		while (line >= central1 - central2 && line<central1 + h2 - central2)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
	}
	else if (frontHeight == central2 && endHeight == h2 - central2)
	{
		while (line<central2 - central1 || (line >= central2 + h1 - central1 && line<h2))
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			for (j = w1 - chongheWidth; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= central2 - central1 && line<central2 + h1 - central1)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
	}
	else if (frontHeight == central1 && endHeight == h2 - central2)
	{
		while (line<central1 - central2)
		{
			int j;
			for (j = 0; j<w1; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			line++;
		}
		while (line >= central1 - central2 && line<h1)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= h1 && line<resultHeight)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			for (j = w1 - chongheWidth; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
	}
	else
	{
		while (line<central2 - central1)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			for (j = w1 - chongheWidth; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= central2 - central1 && line<h2)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= h2 && line<resultHeight)
		{
			int j;
			for (j = 0; j<w1; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			line++;
		}
	}
	return imgResult;
}

void ChongheStitch::chongHeStitch3Init(cv::Mat& img1, cv::Mat& img2, int chongheWidth, int& resultWidth, int& resultHeight, int& central1, int& central2)
{
	int w1, w2, h1, h2;//用于分别存储img1和img2的宽和高
	w1 = img1.cols;
	w2 = img2.cols;
	h1 = img1.rows;
	h2 = img2.rows;
	resultWidth = w1 + w2 - chongheWidth;
	int b1, e1, b2, e2;//b1代表img1最右边（去掉重合部分）可见图像开始点的高，e1代表其可见图像结束点高，b2对应img2最左边可见图像的开始点高，e2对应其结束点高
	uchar *p1 = img1.data;
	uchar *p2 = img2.data;
	b1 = b2 = 0;
	e1 = e2 = 0;
	p1 += img1.elemSize()*(w1 - chongheWidth);
	while (*p1 == 0 && *(p1 + 1) == 0 && *(p1 + 2) == 0)
	{
		b1++;
		p1 += img1.step;
	}
	p1 = img1.data + (h1 - 1)*img1.step + (w1 - chongheWidth)*img1.elemSize();
	while (*p1 == 0 && *(p1 + 1) == 0 && *(p1 + 2) == 0)
	{
		e1++;
		p1 -= img1.step;
	}
	e1 = h1 - e1;
	while (*p2 == 0 && *(p2 + 1) == 0 && *(p2 + 2) == 0)
	{
		b2++;
		p2 += img2.elemSize()*w2;
	}
	p2 = img2.data + (h2 - 1)*img2.step;
	while (*p2 == 0 && *(p2 + 1) == 0 && *(p2 + 2) == 0)
	{
		e2++;
		p2 -= img2.step;
	}
	e2 = h2 - e2;
	central1 = (b1 + e1) / 2;//边缘可见区域的中点  拼接时使得img1和img2的这两个中点对齐
	central2 = (b2 + e2) / 2;
	int frontHeight = std::max(central1, central2);//表示融合图像前半段的高
	int endHeight = std::max(h1 - central1, h2 - central2);//表示融合图像后半段的高
	resultHeight = frontHeight + endHeight;
}

void ChongheStitch::chongHeStitch3(cv::Mat& img1, cv::Mat& img2, cv::Mat& imgResult, int chongheWidth, int& central1, int& central2)
{
	int w1, w2, h1, h2;//用于分别存储img1和img2的宽和高
	int resultWidth, resultHeight;
	w1 = img1.cols;
	w2 = img2.cols;
	h1 = img1.rows;
	h2 = img2.rows;
	resultWidth = w1 + w2 - chongheWidth;
	int frontHeight = std::max(central1, central2);//表示融合图像前半段的高
	int endHeight = std::max(h1 - central1, h2 - central2);//表示融合图像后半段的高
	resultHeight = frontHeight + endHeight;
	uchar* p3 = imgResult.data;
	int line = 0;
	//以下分为根据img1和img2的大小和位置关系分为四种情况来进行图像融合
	uchar* p1 = img1.data;
	uchar* p2 = img2.data;
	if (frontHeight == central1 && endHeight == h1 - central1)
	{
		while (line<central1 - central2 || (line >= central1 + h2 - central2 && line<w1))
		{
			int j;
			for (j = 0; j<w1; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			line++;
		}
		while (line >= central1 - central2 && line<central1 + h2 - central2)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
	}
	else if (frontHeight == central2 && endHeight == h2 - central2)
	{
		while (line<central2 - central1 || (line >= central2 + h1 - central1 && line<h2))
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			for (j = w1 - chongheWidth; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= central2 - central1 && line<central2 + h1 - central1)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
	}
	else if (frontHeight == central1 && endHeight == h2 - central2)
	{
		while (line<central1 - central2)
		{
			int j;
			for (j = 0; j<w1; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			line++;
		}
		while (line >= central1 - central2 && line<h1)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= h1 && line<resultHeight)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			for (j = w1 - chongheWidth; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
	}
	else
	{
		while (line<central2 - central1)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			for (j = w1 - chongheWidth; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= central2 - central1 && line<h2)
		{
			int j;
			for (j = 0; j<w1 - chongheWidth; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			double alpha;
			for (j = w1 - chongheWidth; j<w1; j++)
			{
				alpha = 1;
				*p3 = *p1*alpha + *p2*(1 - alpha); *(p3 + 1) = *(p1 + 1)*alpha + *(p2 + 1)*(1 - alpha); *(p3 + 2) = *(p1 + 2)*alpha + *(p2 + 2)*(1 - alpha);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
				p2 += img2.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = *p2; *(p3 + 1) = *(p2 + 1); *(p3 + 2) = *(p2 + 2);
				p3 += imgResult.elemSize();
				p2 += img1.elemSize();
			}
			line++;
		}
		while (line >= h2 && line<resultHeight)
		{
			int j;
			for (j = 0; j<w1; j++)
			{
				*p3 = *p1; *(p3 + 1) = *(p1 + 1); *(p3 + 2) = *(p1 + 2);
				p3 += imgResult.elemSize();
				p1 += img1.elemSize();
			}
			for (j = w1; j<resultWidth; j++)
			{
				*p3 = 0; *(p3 + 1) = 0; *(p3 + 2) = 0;
				p3 += imgResult.elemSize();
			}
			line++;
		}
	}
}