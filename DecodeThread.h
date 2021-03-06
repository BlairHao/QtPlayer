#pragma once
#include <QThread>
#include <mutex>
#include <list>

struct AVPacket;
struct AVFrame;

class DecodeThread : public QThread
{
public:
	DecodeThread();
	~DecodeThread();

	virtual AVPacket *Pop();
	virtual bool Push(AVPacket *);
	virtual void clear();
	virtual void close();
	void freePacket(AVPacket **pkt);
	void freeFrame(AVFrame **frame);
	bool mbIsExit;
	bool mbIsPause;
private:
	std::mutex m_mutex;
	std::list<AVPacket*> m_packetList;
	int mnMax;
};

