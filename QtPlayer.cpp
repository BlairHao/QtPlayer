#include "QtPlayer.h"
#include <iostream>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QString>

using namespace std;

QtPlayer::QtPlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//openFile();
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(pausePlay()));
	connect(ui.openGLWidget, SIGNAL(resized()), this, SLOT(pictureScale()));
}

QtPlayer::~QtPlayer()
{

}
void QtPlayer::openFile()
{
	QString strFilePathName = QFileDialog::getOpenFileName(this, "open file", ".", "*.*");
	if (strFilePathName.isEmpty())
	{
		QMessageBox::information(this, "Tips", "strFilePathName is empty!!!");
		return;
	}
	if (!demuxThread)
	{
		demuxThread = new XDemuxThread();
	}
	//strFilePathName = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";
	demuxThread->Start();
	demuxThread->openMediaFile(strFilePathName.toLocal8Bit().data(), ui.openGLWidget);
	//demuxThread->openMediaFile(strFilePathName.toLocal8Bit().data());
}

void QtPlayer::pausePlay()
{
	demuxThread->pauseThread();
}

void QtPlayer::resizeEvent(QResizeEvent *event)
{
	//ui.horizontalSlider->move(50, this->height() - 100);
	//ui.horizontalSlider->resize(this->width() - 100, ui.horizontalSlider->height());
	ui.widget_2->move(100, this->height() - 100);
	ui.widget_2->resize(this->width() - 200, 100);
	ui.openGLWidget->resize(this->size());
}

void QtPlayer::closeEvent(QCloseEvent *event)
{
	if (demuxThread)
	{
		demuxThread->StopThread();
	}
}

void QtPlayer::pictureScale()
{
	//cout << "uuuuuuuuuuu" << endl;
	if (demuxThread)
	{
		//demuxThread->pictureScale(ui.openGLWidget->width(),ui.openGLWidget->height());
	}
}

void QtPlayer::wheelEvent(QWheelEvent *event)
{
	mnWidth = ui.openGLWidget->width();
	mnHeight = ui.openGLWidget->height();
	QSize size;
	if (event->delta() > 0) {
		// 当滚轮远离使用者时
		qDebug() << "滚轮事件 --------> 向前" << endl;
		mnWidth = mnWidth + 40;
		mnHeight = mnWidth * 9 / 16;
		size.setWidth(mnWidth);
		size.setHeight(mnHeight);
	}
	else {
		// 当滚轮向使用者方向旋转时
		qDebug() << "滚轮事件 --------> 向后" << endl;
		mnWidth = mnWidth - 40;
		mnHeight = mnWidth * 9 / 16;
		size.setWidth(mnWidth);
		size.setHeight(mnHeight);
	}
	ui.openGLWidget->resize(size);
}
