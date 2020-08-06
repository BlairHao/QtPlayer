#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtPlayer.h"
#include "XDemuxThread.h"
#include <QCloseEvent>
#include <QResizeEvent>

class QtPlayer : public QWidget
{
	Q_OBJECT

public:
	QtPlayer(QWidget *parent = Q_NULLPTR);
	~QtPlayer();

protected:
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);
public slots:
	void openFile();
	void pausePlay();
	void pictureScale();

private:
	Ui::QtPlayerClass ui;
	XDemuxThread *demuxThread = NULL;
};
