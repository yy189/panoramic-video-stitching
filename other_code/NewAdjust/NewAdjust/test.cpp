#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "highgui.h"  
#include "cxcore.h"  
#include "cv.h"
//#include "cvcam.h"  
using namespace cv;
using namespace std;

//图像的像素直接提取  
#define        _I(img,x,y) ((unsigned char*)((img)->imageData + (img)->widthStep*(y)))[(x)]  
//亚像素级灰度值  
#define        _IF(image,x,y)    ( ((int)(x+1)-(x))*((int)(y+1)-(y))*_I((image),(int)(x),(int)(y)) + ((int)(x+1)-(x))*((y)-(int)(y))*_I((image),(int)(x),(int)(y+1)) + ((x)-(int)(x))*((int)(y+1)-(y))*_I((image),(int)(x+1),(int)(y)) + ((x)-(int)(x))*((y)-(int)(y))*_I((image),(int)(x+1),(int)(y+1)) )//插值后的像素值(IN表示interpolation),x、y可以为小数  


int main(int argc, char *argv[])
{
	int count = 2;  //图片数量
	int row = 1580;  //2220 740
	int col = 1300;  //1300 578
	int mx =140;    //220  74
	int my = 130;    //100  44
	char input[100];
	char output[100];
	//IplImage* Show3 = cvCreateImage(cvSize(1920, 1080), IPL_DEPTH_8U, 1);
	IplImage* Show0 = cvCreateImage(cvSize(row, col), IPL_DEPTH_8U, 3);
	IplImage* Showg = cvCreateImage(cvSize(row, col), IPL_DEPTH_8U, 1);
	IplImage* Showb = cvCreateImage(cvSize(row, col), IPL_DEPTH_8U, 1);
	IplImage* Showr = cvCreateImage(cvSize(row, col), IPL_DEPTH_8U, 1);
	IplImage *r = cvCreateImage(cvSize(1280, 1024), IPL_DEPTH_8U, 1);//subpixel
	IplImage *g = cvCreateImage(cvSize(1280, 1024), IPL_DEPTH_8U, 1);//subpixel
	IplImage *b = cvCreateImage(cvSize(1280, 1024), IPL_DEPTH_8U, 1);//subpixel
	//IplImage* ImageC = cvCreateImage(cvSize(1920, 1080), IPL_DEPTH_8U, 1);
	//double *mi;
	//double *md;

	//mi = new double[3 * 3];
	//md = new double[4];

	//CvMat intrinsic_matrix, distortion_coeffs;
	////摄像机内参数
	//cvInitMatHeader(&intrinsic_matrix, 3, 3, CV_64FC1, mi);

	////镜头畸变参数
	//cvInitMatHeader(&distortion_coeffs, 1, 4, CV_64FC1, md);

	double fc1, fc2, cc1, cc2, kc1, kc2, kc3, kc4;
	/* 1*/
	/*fc1 = 1077.40856934;
	fc2 = 1078.91088867;
	cc1 = 557.903564453;
	cc2 = 533.199584961;
	kc1 = -0.431172907352;
	kc2 = 0.200800836086;
	kc3 = -0.000706690247171;
	kc4 = 0.00340239051729;*/
	
	//1矫正  98 110
	//fc1 = 987.820129395;
	//fc2 = 1000.194824219;
	//cc1 = 591.643798828;
	//cc2 = 493.256256104;
	//kc1 = -0.40989869833;
	//kc2 = 0.198635876179;
	//kc3 = 0.00201177503914;
	//kc4 = -0.00163069402333;
	
	////2矫正  140 120
	//fc1 = 1077.40856934;
	//fc2 = 1200.06085205;
	//cc1 = 637.955810547;
	//cc2 = 493.980529785;
	//kc1 = -0.455669671297;
	//kc2 = 0.24515286088;
	//kc3 = -0.00140676402953;
	//kc4 = -0.0017889183946;

	////3矫正 140 120
	//fc1 = 999.098388672;
	//fc2 = 1000.43731689;
	//cc1 = 700.284362793;
	//cc2 = 540.265258789;
	//kc1 = -0.420014262199;  //-0.420014262199
	//kc2 = 0.209278345108;
	//kc3 = -0.00306036346592;
	//kc4 = 0.00134115582821;


	////4矫正 160 130
	//fc1 = 984.635864258;
	//fc2 = 1000.41607666;
	//cc1 = 652.885192871;
	//cc2 = 526.891235352;
	//kc1 = -0.402411401272;
	//kc2 = 0.150330275297;
	//kc3 = 0.000288729934255;
	//kc4 = 0.00209959922358;
	
	////5矫正 120 110
	//fc1 = 1014.41491699;
	//fc2 = 1022.07476807;
	//cc1 = 550.664916992;
	//cc2 = 472.457733154;
	//kc1 = -0.406452804804;
	//kc2 = 0.147502288222;
	//kc3 = -0.00638687703758;
	//kc4 = 0.0139594608918;

	//6矫正 140 130
	fc1 = 996.056152344;
	fc2 = 1022.127563477;
	cc1 = 592.514709473;
	cc2 = 509.90625;
	kc1 = -0.494589734077;
	kc2 = 0.302703738213;
	kc3 = -0.00256470614113;
	kc4 = 0.004401542712;

	/*cvmSet(&intrinsic_matrix, 0, 0, fc1);
	cvmSet(&intrinsic_matrix, 0, 1, 0);
	cvmSet(&intrinsic_matrix, 0, 2, cc1);
	cvmSet(&intrinsic_matrix, 1, 0, 0);
	cvmSet(&intrinsic_matrix, 1, 1, fc2);
	cvmSet(&intrinsic_matrix, 1, 2, cc2);
	cvmSet(&intrinsic_matrix, 2, 0, 0);
	cvmSet(&intrinsic_matrix, 2, 1, 0);
	cvmSet(&intrinsic_matrix, 2, 2, 1);

	cvmSet(&distortion_coeffs, 0, 0, kc1);
	cvmSet(&distortion_coeffs, 0, 1, kc2);
	cvmSet(&distortion_coeffs, 0, 2, kc3);
	cvmSet(&distortion_coeffs, 0, 3, kc4);*/

	for (int i = 1; i < count + 1; i++){
		//读入图像
		sprintf(input, "e:\\全景拼接\\6矫正\\CAM (%d).jpg", i);
		Mat Img = imread(input);
		IplImage* img = &IplImage(Img);
		
		cvFlip(img, NULL, 0);

		cvSplit(img, b, g, r, NULL);
		//cvCvtColor(img, ImageC, CV_RGB2GRAY);
		/*cvFlip(r, NULL, 0);
		cvFlip(g, NULL, 0);
		cvFlip(b, NULL, 0);*/

		//矫正
		//cvUndistort2(ImageC, Show3, &intrinsic_matrix, &distortion_coeffs);

		//矫正畸变 
		for (int nx = 0; nx < row; nx++)
		{
			for (int ny = 0; ny < col; ny++)
			{
				double x = nx - mx;
				double y = ny - my;
				double xx = (x - cc1) / fc1;
				double yy = (y - cc2) / fc2;
				double r2 = pow(xx, 2) + pow(yy, 2);
				double r4 = pow(r2, 2);
				double xxx = xx*(1 + kc1*r2 + kc2*r4) + 2 * kc3*xx*yy + kc4*(r2 + 2 * xx*xx);
				double yyy = yy*(1 + kc1*r2 + kc2*r4) + 2 * kc4*xx*yy + kc3*(r2 + 2 * yy*yy);
				double xxxx = xxx*fc1 + cc1;
				double yyyy = yyy*fc2 + cc2;
				if (xxxx > 0 && xxxx<1280 && yyyy>0 && yyyy < 1024)
				{
					_I(Showr, nx, ny) = (int)_IF(r, xxxx, yyyy);
					_I(Showg, nx, ny) = (int)_IF(g, xxxx, yyyy);
					_I(Showb, nx, ny) = (int)_IF(b, xxxx, yyyy);

				}
				else
				{
					_I(Showr, nx, ny) = 0;
					_I(Showg, nx, ny) = 0;
					_I(Showb, nx, ny) = 0;
				}

			}
		}
		//imshow("img", Img);
		//imshow("undistort", ImgUndistort);
		//cvSaveImage("undistort.jpg", Show3);
		//cvFlip(Showr, NULL, 0);
		////cvSaveImage("undistortR.jpg", Showr);
		//cvFlip(Showg, NULL, 0);
		////cvSaveImage("undistortG.jpg", Showg);
		//cvFlip(Showb, NULL, 0);
		//cvSaveImage("undistortB.jpg", Showb);
		cvMerge(Showb, Showg, Showr, NULL, Show0);
		cvFlip(Show0, NULL, 0);
		sprintf(output, "e:\\全景拼接\\6矫正\\CAM%d_Adjust.jpg", i);
		cvSaveImage(output, Show0);
		waitKey(0);
		Img.release();
	}

	cvReleaseImage(&r);
	cvReleaseImage(&g);
	cvReleaseImage(&b);
	cvReleaseImage(&Showr);
	cvReleaseImage(&Showb);
	cvReleaseImage(&Showg);
	cvReleaseImage(&Show0);
	//cvReleaseImage(&Show3);

}