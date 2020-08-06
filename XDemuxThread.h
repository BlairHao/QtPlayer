#pragma once
#include <QThread>
#include <mutex>
#include "IVideoCall.h"
#include "XDemux.h"
#include "XVideoThread.h"

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
	const char *m_strFilePath = "";

	AVFormatContext *mpFormatCtx;
	int mnVideoIndex;
	int mnAudioIndex;
};

