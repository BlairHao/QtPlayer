#pragma once
#include <QThread>
#include <mutex>
#include "IVideoCall.h"
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"

struct AVFormatContext;
class XDemuxThread : public QThread
{
public:
	XDemuxThread();
	~XDemuxThread();

	void openMediaFile(const char *strFilePath);
	void openMediaFile(const char *, IVideoCall *);
	void Start();
	void pauseThread();
	void StopThread();
	int mnWidth;
	int mnHeight;
	long long pts = 0;
public slots:
	void repeatPlay();
protected:
	void run();

private:
	bool mbIsExit = false;
	bool mbIsPause = false;
	std::mutex mMutex;
	XDemux *mpXDemux;
	IVideoCall *mpVideoCall;
	XVideoThread *mpVideoThread;
	XAudioThread *mpAudioThread;
	const char *m_strFilePath = "";

	AVFormatContext *mpFormatCtx;
	int mnVideoIndex;
	int mnAudioIndex;
};

