#pragma once
#include <mutex>

struct AVCodecContext;
struct AVPacket;
struct AVFrame;
class Decode
{
public:
	Decode();
	~Decode();

	void openCodecParam(AVCodecContext *);
	bool Send(AVPacket *);
	AVFrame *Recv();
	long long mlPts = 0;
private:
	AVCodecContext *mpCodecCtx;
	std::mutex m_mutex;
};

