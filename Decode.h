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
	void clear();
	void close();
	//当前解码到的pts
	long long mlPts = 0;
private:
	AVCodecContext *mpCodecCtx;
	std::mutex m_mutex;
};

