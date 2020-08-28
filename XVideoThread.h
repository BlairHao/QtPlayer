#pragma once
#include "IVideoCall.h"
#include "Decode.h"
#include "DecodeThread.h"
#include <mutex>

struct AVCodecContext;
class XVideoThread : public DecodeThread
{
public:
	XVideoThread();
	~XVideoThread();

	void open(AVCodecContext *pCodecCtx, IVideoCall *pCall, int nWidth, int nHeight);
	//同步时间，由外部传入
	long long synpts = 0;
protected:
	void run();
private:
	AVCodecContext *mpCodecCtx;
	IVideoCall *pVideoCall;
	Decode *pDecode;
	std::mutex m_mutex;
};

