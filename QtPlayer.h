#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtPlayer.h"
#include "XDemuxThread.h"
#include <QCloseEvent>
#include <QResizeEvent>
#include <QWheelEvent>

class QtPlayer : public QWidget
{
	Q_OBJECT

public:
	QtPlayer(QWidget *parent = Q_NULLPTR);
	~QtPlayer();

protected:
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);
	void wheelEvent(QWheelEvent *event);
public slots:
	void openFile();
	void pausePlay();
	void pictureScale();

private:
	Ui::QtPlayerClass ui;
	XDemuxThread *demuxThread = NULL;
	int mnWidth = 0;
	int mnHeight = 0;
};
