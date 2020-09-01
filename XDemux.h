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

	bool openMediaFile(const char *);
	AVCodecContext *copyVideoParam();
	AVCodecContext *copyAudioParam();
	AVPacket *readOnePacket();
	bool IsVideo(AVPacket *);
	//Çå¿Õ¶ÁÈ¡»º³å
	void clear();
	void close();
	bool seek(double nPosition);
	AVPacket *readVideo();
	int mnSrcWidth;
	int mnSrcHeight;
	bool mbRepeatPlay;
	int mnSampleRate;
	int mnChannels;
	int mlTotalMs;

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
