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
	//ͬ��ʱ�䣬���ⲿ����
	long long synpts = 0;
protected:
	void run();
private:
	AVCodecContext *mpCodecCtx;
	IVideoCall *pVideoCall;
	Decode *pDecode;
	std::mutex m_mutex;
};

