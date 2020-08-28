#pragma once
#include <mutex>
struct AVCodecContext;
struct SwrContext;
struct AVFrame;
class XResample
{
public:
	XResample();
	~XResample();

	bool open(AVCodecContext *param);
	void close();
	int resample(AVFrame *indata, unsigned char *d);
	//AV_SAMPLE_FMT_S16
	int outFormat = 1;
private:
	std::mutex mMutex;
	SwrContext *mpSwrCtx = 0;
};

