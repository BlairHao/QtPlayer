#pragma once
#include <QThread>
#include <mutex>
#include <list>

struct AVPacket;

class DecodeThread : public QThread
{
public:
	DecodeThread();
	~DecodeThread();

	virtual AVPacket *Pop();
	virtual bool Push(AVPacket *);
	bool mbIsExit;
	bool mbIsPause;
private:
	std::mutex m_mutex;
	std::list<AVPacket*> m_packetList;
	int mnMax;
};

