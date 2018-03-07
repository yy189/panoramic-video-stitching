#include "mp42yuv.h"
#include "yuv2jpg偏白.h"
#include <iostream>
#include <sstream>
#include <direct.h>
using namespace std;

void main()
{
	stringstream ss;
	char foldername[100], yuvname[3];
	string pathname;
	cout << "！注意：mp4文件应按顺时针顺序标号！\n请输入包含0.mp4~5.mp4的文件夹的名称（在ffmpeg-test根目录下）：\n";
	cin >> pathname;
	for (int i = 0; i < 6; i++)
	{
		ss << pathname << "//" << i;
		ss >> foldername;
		if (_mkdir(foldername) == -1)//创建文件夹
		{
			cout << "文件夹" << i << "已存在，请删除！\n";
			return;
		}
		int result = FileWriteYUV(foldername);//mp4转yuv
		if (result == -1)
		{
			cout << "some error in FileWriteYUV" << endl;
			return;
		}
		FileWriteFrames(foldername);//yuv转jpg
		ss.clear();
	}
	for (int i = 0; i < 6; i++)//删除yuv文件
	{
		ss << pathname << "//" << i << ".yuv";
		ss >> yuvname;
		remove(yuvname);
		ss.clear();
	}
}