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
private:
	AVCodecContext *mpCodecCtx;
	std::mutex m_mutex;
};

