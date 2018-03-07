#include <opencv2/nonfree/nonfree.hpp>  
#include "CylindricalMapping2.h"
#include "ChongheStitch.h"
#include "StreamPusherDll.h"
#include <stdio.h>

using namespace cv;
using namespace std;

const char *out_filename = "result.avi";//全景视频路径

//对img1和img2进行surf特征检测、特征描述、特征匹配得到单应矩阵（变换矩阵）第三个参数是得到的变换矩阵,注意这里面有个inlier_minRx用于表示右图最左边的和左图重合的位置，这里默认为0，故这个参数没有用到，如果以后要改的话，后面的拼接函数也要相应的改
void getHimprove(Mat& img1, Mat& img2, Mat& H);
//对上面得到H变换得到标准位置的变换矩阵H1
void changeHtoH1Improve(Mat& img1, int& imgleftWidth, int& imgRightBeginHeight, Mat&H, Mat& H1);
//通过变换矩阵 将六幅图两两拼接在一起，左后一个参数是左右图重合部分的宽度
//对上面的改进，左图变换到右图视角，有和右图重合区域，也有不重合区域，并且不重合区域位于重合区域的左侧，imgLeftWidth参数代表变形后的左图没有和右图重合区域的宽度，imgRightBeginHeight参数代表右图在拼接图中的开始位置的高
void framePinJieInit(Mat img[6], Mat img_result[6], Mat H[6], int imgLeftWidth[6], int width[6], int height[6], int guoduPexelNums);//完成多线程图像拼接初始化工作，因为多线程后join()会释放掉imgPinJieResult的数据，故采用先初始化imgPinJieResult来防止其因为多线程join()而被释放掉，使得后续的图像融合可以继续
void framePinJieNew3(Mat& img1, Mat& img2, Mat& img_result, Mat& H, int imgLeftWidth, int imgRightBeginHeight, int width, int height, int guoduPexelNums);//前几个参数意义和framePinJieNew2版本一样， width、height两个参数代表img1转换之后的宽和高,最后一个参数代表拼接融合时过度像素的宽度
bool cylindricalStitch2(Mat img[6], Mat H[6], int imgLeftWidth[6], int imgRightBeginHeight[6], Mat& imgResult);//对六张图像同时拼接，获取全景图像，采用多线程

int main(int argc, char** argv)
{
	int frame_num;//想要生成的帧数
	int mode;
	cout << "请选择模式：\n1.视频帧拼接（输入1）\n2.USB摄像头实时拼接(输入2)\n";
	cin >> mode;
	Mat imgResult;//最终帧
	int final_frame_width;//最终帧宽像素
	int final_frame_height;//最终帧高像素
	Mat H[6];//单应矩阵_初步
	Mat H1[6];//单应矩阵_final
	//规格化变换矩阵和并得到相应拼接所需参数
	int imgleftwidth[6];
	int imgrightbeginheight[6];
	Mat MapPic[6];//读取经过柱面映射后的图像
	CvCapture* cap[6];//摄像头捕获的帧
	stringstream ss;
	string folderpath;//图像拼接时的文件夹路径
	int cam[6] = { 0 };//图像拼接时的起始帧编号
	string filename_temp;//临时的文件名
	VideoWriter writer;//视频写入
	double final_video_fps;//全景视频帧率
	if (mode == 1)
	{
		cout << "请输入包含视频帧文件夹0~5的文件夹路径：\n";
		cin >> folderpath;
		cout << "请按顺序输入每个起始帧的编号：\n";
		for (int i = 0; i < 6; i++)
		{
			cin >> cam[i];
		}
		cout << "请设置全景视频帧率：\n";
		cin >> final_video_fps;
	}
	else if (mode == 2)
	{
		int fps = 30, frame_width = 1280, frame_height = 720;
		cout << "请输入摄像头帧率 像素宽 像素高：\n";
		cin >> fps >> frame_width >> frame_height;
		for (int i = 5; i >= 0; i--)//默认计算机不自带网络摄像头
		{
			cap[i] = cvCaptureFromCAM(i);
			cvSetCaptureProperty(cap[i], CV_CAP_PROP_FPS, fps);//帧率
			cvSetCaptureProperty(cap[i], CV_CAP_PROP_FRAME_WIDTH, frame_width);//像素宽
			cvSetCaptureProperty(cap[i], CV_CAP_PROP_FRAME_HEIGHT, frame_height);//像素高
			if (!cap[i])
			{
				cout << "create camera" << i << " capture error" << endl;
				return -1;
			}
		}
	}
	cout << "请输入想要生成的帧数：\n";
	cin >> frame_num;
	cout << "\n请等待...\n" << endl;
	for (int i = 0; i < frame_num; i++)
	{
		if (mode == 1)
		{
			for (int j = 0; j < 6; j++)
			{
				ss << folderpath << "//" << j << "//" << i + cam[j] << ".jpg";
				ss >> filename_temp;
				MapPic[j] = imread(filename_temp);
				ss.clear();
			}
		}
		else if (mode == 2)
		{
			for (int j = 0; j < 6; j++)
			{
				MapPic[j] = cvQueryFrame(cap[j]);
			}
		}

		//柱面映射（可以将原始图像柱面映射的到映射图像） 使用多线程实现更快
		CylindricalMapping2 CylinMaper(615, MapPic);//小蚁相机 左1 半径+焦距==615
		CylinMaper.cylindricalMapping();
		Mat *MapPic;
		MapPic = CylinMaper.getMapresult();

		if (i == 0)//单应矩阵通过第一批6张图获取
		{
			getHimprove(MapPic[0], MapPic[1], H[0]);
			getHimprove(MapPic[1], MapPic[2], H[1]);
			getHimprove(MapPic[2], MapPic[3], H[2]);
			getHimprove(MapPic[3], MapPic[4], H[3]);
			getHimprove(MapPic[4], MapPic[5], H[4]);
			getHimprove(MapPic[5], MapPic[0], H[5]);

			changeHtoH1Improve(MapPic[0], imgleftwidth[0], imgrightbeginheight[0], H[0], H1[0]);
			changeHtoH1Improve(MapPic[1], imgleftwidth[1], imgrightbeginheight[1], H[1], H1[1]);
			changeHtoH1Improve(MapPic[2], imgleftwidth[2], imgrightbeginheight[2], H[2], H1[2]);
			changeHtoH1Improve(MapPic[3], imgleftwidth[3], imgrightbeginheight[3], H[3], H1[3]);
			changeHtoH1Improve(MapPic[4], imgleftwidth[4], imgrightbeginheight[4], H[4], H1[4]);
			changeHtoH1Improve(MapPic[5], imgleftwidth[5], imgrightbeginheight[5], H[5], H1[5]);
		}

		cylindricalStitch2(MapPic, H1, imgleftwidth, imgrightbeginheight, imgResult);

		if (i == 0)//通过第一张全景图调整像素宽高
		{
			if (imgResult.cols % 2 == 0)//偶数不变,奇数-1
				final_frame_width = imgResult.cols;
			else
				final_frame_width = imgResult.cols - 1;
			if (imgResult.rows % 2 == 0)//偶数不变,奇数-1
				final_frame_height = imgResult.rows;
			else
				final_frame_height = imgResult.rows - 1;
			//encodeAndPushInit(final_frame_width, final_frame_height, out_filename);
			writer.open(out_filename, CV_FOURCC('M', 'J', 'P', 'G'), final_video_fps, Size(final_frame_width, final_frame_height));
		}
		resize(imgResult, imgResult, Size(final_frame_width, final_frame_height), 0, 0, CV_INTER_LINEAR);
		//encodeAndPush(final_frame_width, final_frame_height, imgResult.data);//编码&推流
		writer << imgResult;
		cout << "第" << i + 1 << "帧生成完毕！\n";
	}
	//pushEnd();
	if (mode == 2)
	{
		for (int i = 0; i < 6; i++)//释放摄像头
			cvReleaseCapture(&cap[i]);
	}
	remove("ds.h264");//删除中间文件
	remove("output.yuv");
	return 0;
}

void getHimprove(Mat& img11, Mat& img22, Mat& H)
{
	int width = img11.cols / 2;//因为img1和img2的宽和高相同 
	int height = img11.rows;
	Mat img1 = Mat::zeros(height, width, img11.type());
	Mat img2 = Mat::zeros(height, width, img22.type());
	uchar*p1_1 = img1.data;
	uchar*p1_2 = img11.data;
	uchar*p2_1 = img2.data;
	uchar*p2_2 = img22.data;

	for (int row = 0; row<height; row++)
	{
		p1_2 = img11.data + row*img11.step + width*img11.elemSize();
		p2_2 = img22.data + row*img22.step;
		for (int col = 0; col<width; col++)
		{
			*p1_1 = *p1_2; *(p1_1 + 1) = *(p1_2 + 1); *(p1_1 + 2) = *(p1_2 + 2);
			*p2_1 = *p2_2; *(p2_1 + 1) = *(p2_2 + 1); *(p2_1 + 2) = *(p2_2 + 2);
			p1_1 += img1.elemSize();
			p1_2 += img11.elemSize();
			p2_1 += img2.elemSize();
			p2_2 += img22.elemSize();
		}
	}

	initModule_nonfree();//初始化模块，使用SIFT或SURF时用到 
	Ptr<FeatureDetector> detector = FeatureDetector::create("SURF");//创建SIFT特征检测器，可改成SIFT/ORB
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("SURF");//创建特征向量生成器，可改成SIFT/ORB
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create("BruteForce");//创建特征匹配器  
	if (detector.empty() || descriptor_extractor.empty())
		cout << "fail to create detector!";

	//特征点检测  
	vector<KeyPoint> m_LeftKey, m_RightKey;
	detector->detect(img1, m_LeftKey);//检测img1中的SIFT特征点，存储到m_LeftKey中  
	detector->detect(img2, m_RightKey);

	//根据特征点计算特征描述子矩阵，即特征向量矩阵  
	Mat descriptors1, descriptors2;
	descriptor_extractor->compute(img1, m_LeftKey, descriptors1);
	descriptor_extractor->compute(img2, m_RightKey, descriptors2);

	//特征匹配  
	vector<DMatch> matches;//匹配结果  
	descriptor_matcher->match(descriptors1, descriptors2, matches);//匹配两个图像的特征矩阵  

	//计算匹配结果中距离的最大和最小值  
	//距离是指两个特征向量间的欧式距离，表明两个特征的差异，值越小表明两个特征点越接近  
	double max_dist = 0;
	double min_dist = 100;
	for (int i = 0; i<matches.size(); i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//筛选出较好的匹配点  
	vector<DMatch> goodMatches;
	for (int i = 0; i<matches.size(); i++)
	{
		if (matches[i].distance < 0.2 * max_dist)
		{
			goodMatches.push_back(matches[i]);
		}
	}

	//RANSAC匹配过程
	vector<DMatch> m_Matches = goodMatches;
	//分配空间
	int ptCount = (int)m_Matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	//把Keypoint转换为Mat
	Point2f pt;
	for (int i = 0; i<ptCount; i++)
	{
		pt = m_LeftKey[m_Matches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = m_RightKey[m_Matches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}

	// 用RANSAC方法计算F
	vector<uchar> m_RANSACStatus;//这个变量用于存储RANSAC后每个点的状态
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);

	// 计算野点个数

	int OutlinerCount = 0;
	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] == 0)//状态为0表示野点
		{
			OutlinerCount++;
		}
	}
	int InlinerCount = ptCount - OutlinerCount;//计算内点

	// 这三个变量用于保存内点和匹配关系
	vector<Point2f> m_LeftInlier;
	vector<Point2f> m_RightInlier;
	vector<DMatch> m_InlierMatches;

	m_InlierMatches.resize(InlinerCount);
	m_LeftInlier.resize(InlinerCount);
	m_RightInlier.resize(InlinerCount);
	InlinerCount = 0;
	float inlier_minRx = img1.cols;//用于存储内点中右图最小横坐标，以便后续融合

	for (int i = 0; i<ptCount; i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0) + width;//这里加上width 因为需要调整到原图像的空间位置
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;

			if (m_RightInlier[InlinerCount].x<inlier_minRx) inlier_minRx = m_RightInlier[InlinerCount].x;   //存储内点中右图最小横坐标 猜想（这里的右图和左图重合部分的坐标也重合）

			InlinerCount++;
		}
	}
	inlier_minRx = 0;

	//矩阵H用以存储RANSAC得到的单应矩阵
	H = findHomography(m_LeftInlier, m_RightInlier, RANSAC);
}
void changeHtoH1Improve(Mat& img1, int& imgLeftWidth, int& imgRightBeginHeight, Mat&H, Mat& H1)
{
	//存储左图四角，及其变换到右图位置
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0, 0); obj_corners[1] = Point(img1.cols, 0);
	obj_corners[2] = Point(img1.cols, img1.rows); obj_corners[3] = Point(0, img1.rows);
	std::vector<Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H);

	int drift = scene_corners[1].x;//储存偏移量
	//自己添加
	imgLeftWidth = abs(min(scene_corners[0].x, scene_corners[3].x));
	imgRightBeginHeight = abs(min(scene_corners[0].y, scene_corners[1].y));

	//新建一个矩阵存储配准后四角的位置
	int width = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
	int height = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
	float origin_x = 0, origin_y = 0;
	//自己添加
	origin_x = min(scene_corners[0].x, scene_corners[3].x);
	origin_y = min(scene_corners[0].y, scene_corners[1].y);
	width -= int(origin_x);
	height -= int(origin_y);

	//获取新的变换矩阵，使图像完整显示
	for (int i = 0; i<4; i++) {
		scene_corners[i].x -= origin_x;
		//可选：（重要！！！）
		scene_corners[i].y -= origin_y; //因为两个图像拼接函数所需的变换矩阵不同，对于第一个图像拼接函数这条语句忽略，改进的两个拼接函数这条语句要加上
	}
	H1 = getPerspectiveTransform(obj_corners, scene_corners);
}
void framePinJieInit(Mat img[6], Mat img_result[6], Mat H[6], int imgLeftWidth[6], int width[6], int height[6], int guoduPexelNums)
{
	vector<Point2f> obj_corners(4);
	vector<Point2f> scene_corners(4);
	int imgResultHeight, imgResultWidth;
	int nextIndex;

	//改进1 因为几个相机获取到的原始图像大小相同，所以可以统一处理
	obj_corners[0] = Point(0, 0); obj_corners[1] = Point(img[0].cols, 0);
	obj_corners[2] = Point(img[0].cols, img[0].rows); obj_corners[3] = Point(0, img[0].rows);
	for (int i = 0; i < 6; i++)
	{
		perspectiveTransform(obj_corners, scene_corners, H[i]);
		//新建一个矩阵存储配准后四角的位置
		width[i] = int(max(abs(scene_corners[1].x), abs(scene_corners[2].x)));
		height[i] = int(max(abs(scene_corners[2].y), abs(scene_corners[3].y)));
		nextIndex = (i + 1) % 6;
		imgResultHeight = max(height[i], img[nextIndex].rows);
		imgResultWidth = img[nextIndex].cols / 2 + imgLeftWidth[i] - width[i] / 2 + 2 * guoduPexelNums;//注意这里width和img2.cols为偶数和奇数对后面for循环有不同的处理
		img_result[i] = Mat::zeros(imgResultHeight, imgResultWidth, img[i].type());
		img_result[i].rows = imgResultHeight;
		img_result[i].cols = imgResultWidth;
	}
}
void framePinJieNew3(Mat& img1, Mat& img2, Mat& img_result, Mat& H, int imgLeftWidth, int imgRightBeginHeight, int width, int height, int guoduPexelNums)
{
	//新建一个矩阵存储配准后四角的位置
	Mat imageturn = Mat::zeros(height, width, img1.type());

	//进行图像变换，显示效果
	warpPerspective(img1, imageturn, H, Size(width, height));
	int imgResultHeight = max(height, img2.rows);

	//当imgLeftWidth < width/2时，说明图像img1到imgturn的图像变形有问题，变换矩阵H不准确，此时无法继续拼接下去
	if (imgLeftWidth < width / 2)
	{
		cout << "变换矩阵计算不准确，imgLeftWidth < width/2，无法继续拼接下去" << endl;
		return;
	}

	uchar* ptr1 = imageturn.data;
	uchar* ptr2 = img2.data;
	uchar* ptr3 = img_result.data;
	double alpha;

	for (int row = 0; row<imgResultHeight; row++)
	{
		ptr1 += (width / 2 - guoduPexelNums)*imageturn.elemSize();//width为奇数和偶数时关系相差1个像素，不过跳过像素数的关系表达式相同
		for (int col = width / 2 - guoduPexelNums; col<imgLeftWidth; col++)
		{
			*ptr3 = *ptr1; *(ptr3 + 1) = *(ptr1 + 1); *(ptr3 + 2) = *(ptr1 + 2);
			ptr3 += img_result.elemSize();
			ptr1 += imageturn.elemSize();
		}
		for (int col = 0; col<(img2.cols) / 2 + guoduPexelNums; col++)
		{
			if (row<imgRightBeginHeight)
			{
				if (col < width - imgLeftWidth)
				{
					*ptr3 = *ptr1; *(ptr3 + 1) = *(ptr1 + 1); *(ptr3 + 2) = *(ptr1 + 2);
					ptr3 += img_result.elemSize();
					ptr1 += imageturn.elemSize();
				}
				else
				{
					*ptr3 = 0; *(ptr3 + 1) = 0; *(ptr3 + 2) = 0;
					ptr3 += img_result.elemSize();
				}
			}
			else if (row < imgRightBeginHeight + img2.rows)
			{
				if (col<width - imgLeftWidth)
				{
					alpha = (double)col / (width - imgLeftWidth);
					if (*ptr1 == 0)
					{
						*ptr3 = *ptr2; *(ptr3 + 1) = *(ptr2 + 1); *(ptr3 + 2) = *(ptr2 + 2);
					}
					else
					{
						*ptr3 = *ptr1*(1 - alpha) + *ptr2*alpha; *(ptr3 + 1) = *(ptr1 + 1)*(1 - alpha) + *(ptr2 + 1)*alpha; *(ptr3 + 2) = *(ptr1 + 2)*(1 - alpha) + *(ptr2 + 2)*alpha;
					}
					ptr1 += imageturn.elemSize();
					ptr2 += img2.elemSize();
					ptr3 += img_result.elemSize();
				}
				else
				{
					*ptr3 = *ptr2; *(ptr3 + 1) = *(ptr2 + 1); *(ptr3 + 2) = *(ptr2 + 2);
					ptr2 += img2.elemSize();
					ptr3 += img_result.elemSize();
				}
				//跳过img2后半部分（去掉过渡点）
			}
			else
			{
				if (col<width - imgLeftWidth)
				{
					*ptr3 = *ptr1; *(ptr3 + 1) = *(ptr1 + 1); *(ptr3 + 2) = *(ptr1 + 2);
					ptr1 += imageturn.elemSize();
					ptr3 += img_result.elemSize();
				}
				else
				{
					*ptr3 = 0; *(ptr3 + 1) = 0; *(ptr3 + 2) = 0;
					ptr3 += img_result.elemSize();
				}
			}
		}
		//跳过img2后半部分（去掉过渡点）
		if (row >= imgRightBeginHeight && row < img2.rows + imgRightBeginHeight)
		{
			if (img2.cols % 2 == 0)
				ptr2 += (img2.cols / 2 - guoduPexelNums)*img2.elemSize();
			else
				ptr2 += ((img2.cols) / 2 + 1 - guoduPexelNums)*img2.elemSize();
		}
	}
}

bool cylindricalStitch2(Mat img[6], Mat H[6], int imgLeftWidth[6], int imgRightBeginHeight[6], Mat& imgResult)
{
	//以下多线程完成6对相邻图像的拼接，得到两两中间融合区域的图像
	static const int numThreads = 6;
	vector<thread> cylindricalThreads(numThreads);
	Mat imgPinJieResult[6];

	//完成多线程图像拼接初始化工作，因为多线程后join()会释放掉imgPinJieResult的数据，故采用先初始化imgPinJieResult来防止其因为多线程join()而被释放掉
	int width[6], height[6];
	int guoduLength = 10;
	framePinJieInit(img, imgPinJieResult, H, imgLeftWidth, width, height, guoduLength);
	
	//多线程处理图像拼接
	for (int i = 0; i < numThreads; i++)
	{
		switch (i)
		{
			case 0:
				cylindricalThreads[0] = thread(&framePinJieNew3, img[0], img[1], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 1:
				cylindricalThreads[1] = thread(&framePinJieNew3, img[1], img[2], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 2:
				cylindricalThreads[2] = thread(&framePinJieNew3, img[2], img[3], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 3:
				cylindricalThreads[3] = thread(&framePinJieNew3, img[3], img[4], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 4:
				cylindricalThreads[4] = thread(&framePinJieNew3, img[4], img[5], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			case 5:
				cylindricalThreads[5] = thread(&framePinJieNew3, img[5], img[0], imgPinJieResult[i], H[i], imgLeftWidth[i], imgRightBeginHeight[i], width[i], height[i], guoduLength);
				break;
			default:
				cout << "some error in cylindricalStitch2 cylindricalThreads" << endl;
				return false;
				break;
		}
	}

	//多线程
	for (int i = 0; i < numThreads; i++)
	{
		cylindricalThreads[i].join();
	}

	//以下多线程完成上面6副图像的两两融合，得到三副融合的图像
	static const int numThreads2 = 3;
	vector<thread> cylindricalThreads2(numThreads2);
	ChongheStitch Stitcher;
	Mat imgChongHeStitchResult[3];
	int resultWidth[3];
	int resultHeight[3];//分别用于存储融合后的三副图像的宽和高
	int central[6];

	//这里也可以使用多线程处理
	for (int i = 0; i < numThreads2;i++)
	{
		Stitcher.chongHeStitch3Init(imgPinJieResult[i * 2], imgPinJieResult[i * 2 + 1], guoduLength, resultWidth[i], resultHeight[i], central[i * 2], central[i * 2 + 1]);
	}
	for (int i = 0; i < numThreads2;i++)
	{
		imgChongHeStitchResult[i] = cv::Mat::zeros(resultHeight[i], resultWidth[i], imgPinJieResult[i*2].type());
	}
	for (int i = 0; i < numThreads2; i++)
	{
		switch (i)
		{
		case 0:
			cylindricalThreads2[0] = thread(&ChongheStitch::chongHeStitch3, &Stitcher, imgPinJieResult[0], imgPinJieResult[1], imgChongHeStitchResult[i], guoduLength, central[i * 2], central[i * 2 + 1]);
			break;
		case 1:
			cylindricalThreads2[1] = thread(&ChongheStitch::chongHeStitch3, &Stitcher, imgPinJieResult[2], imgPinJieResult[3], imgChongHeStitchResult[i], guoduLength, central[i * 2], central[i * 2 + 1]);
			break;
		case 2:
			cylindricalThreads2[2] = thread(&ChongheStitch::chongHeStitch3, &Stitcher, imgPinJieResult[4], imgPinJieResult[5], imgChongHeStitchResult[i], guoduLength, central[i * 2], central[i * 2 + 1]);
			break;
		default:
			cout << "some error in cylindricalStitch2 cylindricalThreads2" << endl;
			return false;
			break;
		}
	}
	for (int i = 0; i < numThreads2; i++)
	{
		cylindricalThreads2[i].join();
	}

	//以下对上面获得的3副图像进行融合，得到最终的全景图
	Mat imgRongheResult;
	imgRongheResult = Stitcher.chongHeStitch(imgChongHeStitchResult[0], imgChongHeStitchResult[1], guoduLength);
	imgResult = Stitcher.chongHeStitch(imgRongheResult, imgChongHeStitchResult[2], guoduLength);
	return true;
}