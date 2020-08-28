#pragma once
#include "DecodeThread.h"
#include "XAudioPlay.h"
#include "XResample.h"
#include "Decode.h"
#include <mutex>

struct AVCodecContext;
class XAudioThread : public DecodeThread
{
public:
	XAudioThread();
	~XAudioThread();

	bool open(AVCodecContext *pCodecCtx, int nSample, int nChannels);
	//停止线程清理资源
	void close();
	void clear();
	void setPause(bool isPause);
	//当前音频播放的pts
	long long pts = 0;
	bool mbIsPause = false;
	bool mbIsExit = false;
protected:
	void run();

private:
	std::mutex mMutex;
	XAudioPlay *mpAudioPlay = NULL;
	XResample *mpResample = NULL;
	Decode *pDecode = NULL;
};

