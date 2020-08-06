#pragma once
#include <mutex>
#include <QObject>

struct  AVCodecContext;
struct AVFormatContext;
struct AVPacket;
struct SwsContext;

class XDemux : public QObject
{
	Q_OBJECT
public:
	XDemux(QObject *parent = 0);
	~XDemux();

	void openMediaFile(const char *);
	AVCodecContext *copyVideoParam();
	AVPacket *readOnePacket();
	bool IsVideo(AVPacket *);
	int mnSrcWidth;
	int mnSrcHeight;
	bool mbRepeatPlay;

signals:
	void signal_read_finish();

public slots:
	void test();
private:
	AVFormatContext *mpFormatCtx;
	int mnVideoIndex;
	int mnAudioIndex;
	std::mutex m_mutex;
};
