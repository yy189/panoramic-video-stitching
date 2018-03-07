#include "bass.h"
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

std::string file_name("410拍手秒表视频//1-2.mp3");

string outputFilePath = "410拍手秒表视频//scan_peak1-2.txt";

std::vector<float> wave_ampli;
double interval = 0.0416667;
int last_i = 0;//同步结束位置
int first_i = 0;//同步开始位置
int	last_volume(0);//同步结束位置音量大小
int first_volume(0);//同步开始位置音量大小
//int g_min_ampli(1000);
DWORD   chan;                   //the file handle 
DWORD	g_bypePerPixel;
const int MAX_PEAK = 32768;	// peak max amplitude
const int WIDTH = 800;		// display width
const int HEIGHT = 201;		// height (odd number for centre line)


void ScanPeaks(DWORD decoder)
{
	int i = 0;
	DWORD length = BASS_ChannelGetLength(decoder, BASS_POS_BYTE);
	g_bypePerPixel = BASS_ChannelGetLength(decoder, BASS_POS_BYTE) / WIDTH; // bytes per pixel
	if (g_bypePerPixel<BASS_ChannelSeconds2Bytes(decoder, interval)) // minimum 20ms per pixel (BASS_ChannelGetLevel scans 20ms)
		g_bypePerPixel = BASS_ChannelSeconds2Bytes(decoder, interval);
	DWORD cpos = 0, peak[2] = { 0 };
	while (true) {
		DWORD level = BASS_ChannelGetLevel(decoder); // scan peaks
		DWORD pos;
		if (peak[0]<LOWORD(level)) peak[0] = LOWORD(level); // set left peak
		if (peak[1]<HIWORD(level)) peak[1] = HIWORD(level); // set right peak
		if (!BASS_ChannelIsActive(decoder))
			pos = -1; // reached the end
		else
			pos = BASS_ChannelGetPosition(decoder, BASS_POS_BYTE) / g_bypePerPixel;
		if (pos>cpos) {
			DWORD a(0);
			int max_ampli = peak[0] * (HEIGHT / 2) / MAX_PEAK;
			int min_ampli = peak[1] * (HEIGHT / 2) / MAX_PEAK;
			int avg_ampli = (max_ampli + min_ampli) / 2;
			if (avg_ampli > last_volume || (avg_ampli == last_volume && i != last_i+1))//只排除最大值的后面一个
			{
				first_i = last_i;
				first_volume = last_volume;
				last_i = i;
				last_volume = avg_ampli;
			}
			/*if (avg_ampli<g_min_ampli)
				g_min_ampli = avg_ampli;*/
			wave_ampli.push_back(avg_ampli);
			if (pos >= WIDTH)
				break; // gone off end of display
			cpos = pos;
			peak[0] = peak[1] = 0;
			i++;
		}
	}
	BASS_StreamFree(decoder); // free the decoder
}

//////////////////////////////////////////////////////////////////////////  
void CALLBACK LoopSyncProc(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	BASS_ChannelSetPosition(channel, 0, BASS_POS_BYTE); // failed, go to start of file instead  
}

bool PlayFile()
{
	const char* file = file_name.c_str();
	if (!(chan = BASS_StreamCreateFile(FALSE, file, 0, 0, 0))
		&& !(chan = BASS_MusicLoad(FALSE, file, 0, 0, BASS_MUSIC_RAMPS | BASS_MUSIC_POSRESET | BASS_MUSIC_PRESCAN, 0))) {
		return FALSE; // Can't load the file  
	}
	// repeat playing  
	BASS_ChannelSetSync(chan, BASS_SYNC_END | BASS_SYNC_MIXTIME, 0, LoopSyncProc, 0); // set sync to loop at end  
	BASS_ChannelPlay(chan, false);
	return true;
}

void main()
{
	//////////////////////////////////////////////////////////////////////////  
	// Init bass  
	if (!BASS_Init(-1, 48000, 0, 0, NULL))
	{
		std::cout << ("Can't initialize device");
		return;
	}

	ofstream outfile(outputFilePath, ios::out);

	DWORD chan2 = BASS_StreamCreateFile(FALSE, file_name.c_str(), 0, 0, BASS_STREAM_DECODE);
	if (!chan2) chan2 = BASS_MusicLoad(FALSE, file_name.c_str(), 0, 0, BASS_MUSIC_DECODE, 0);
	ScanPeaks(chan2);
	for (size_t i(0); i<wave_ampli.size(); i++)
		outfile << wave_ampli[i] << std::endl;
	outfile << "first_i: " << first_i << endl;
	outfile << "first_volume: " << first_volume << endl;
	outfile << "last_i: " << last_i << endl;
	outfile << "last_volume: " << last_volume << endl;
	//cout << g_min_ampli << endl;

	system("pause");
}