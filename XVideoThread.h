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

	bool open(AVCodecContext *pCodecCtx, IVideoCall *pCall, int nWidth, int nHeight);
	virtual void clear();
	virtual void close();
	bool repaintPts(AVPacket *pkt, long long seekPts);
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

