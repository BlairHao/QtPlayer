#include "QtPlayer.h"
#include <iostream>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QString>
#include <QLatin1Char>
#include <QMenu>
#include <QAction>

using namespace std;

QtPlayer::QtPlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initUI();
	initDataAndStatus();
	connect(ui.stop_btn, SIGNAL(clicked()), this, SLOT(stopPlaySlot()));
	connect(ui.previous_btn, SIGNAL(clicked()), this, SLOT(playPreviousFile()));
	connect(ui.next_btn, SIGNAL(clicked()), this, SLOT(playNextFile()));
	connect(ui.play_pause_btn, SIGNAL(clicked()), this, SLOT(pausePlay()));
	connect(ui.horizontalSlider, SIGNAL(sliderPressed(int)), this, SLOT(sliderPressedSlot(int)));
	connect(ui.horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleasedSlot()));
	connect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMovedSlot(int)));
	connect(mpInputStream, SIGNAL(startPlaySignal(QString)), this, SLOT(playLiveStream(QString)));
}

QtPlayer::~QtPlayer()
{

}
void QtPlayer::initDataAndStatus()
{
	mbIsWheelScale = false;
	mbSliderPressed = false;
	mbIsHide = false;
	mbIsPause = false;
	mbIsLive = false;
	mnScaleValue = 1.0;
	mnPaperWidth = 800;
	mnPaperHeight = 450;

	mpInputStream = new InputStream;
	mpDemuxThread = new XDemuxThread;
	mpDemuxThread->Start();

	setFocusPolicy(Qt::StrongFocus);
	installEventFilter(this);
	ui.stop_btn->installEventFilter(this);
	ui.stop_btn->installEventFilter(this);
	ui.previous_btn->installEventFilter(this);
	ui.play_pause_btn->installEventFilter(this);
	ui.next_btn->installEventFilter(this);
	ui.horizontalSlider->installEventFilter(this);

	//ui.stop_btn->setEnabled(false);
	//ui.previous_btn->setEnabled(false);
	//ui.play_pause_btn->setEnabled(false);
	//ui.next_btn->setEnabled(false);

	updateWidthOfPerPixel();
	startTimer(40);
}

void QtPlayer::initUI()
{
	ui.stop_btn->setFixedSize(32, 32);
	ui.stop_btn->setStyleSheet("QPushButton{border-image: url(:/button/Resources/icon/stop_normal.png);}"
		"QPushButton:hover{border-image: url(:/button/Resources/icon/stop_active.png);}"
		"QPushButton:pressed{border-image: url(:/button/Resources/icon/stop_normal.png);}"
		"QPushButton {background-color:transparent;}");
	ui.previous_btn->setFixedSize(32, 32);
	ui.previous_btn->setStyleSheet("QPushButton{border-image: url(:/button/Resources/icon/previous_normal.png);}"
		"QPushButton:hover{border-image: url(:/button/Resources/icon/previous_active.png);}"
		"QPushButton:pressed{border-image: url(:/button/Resources/icon/previous_normal.png);}"
		"QPushButton {background-color:transparent;}");
	ui.play_pause_btn->setFixedSize(32, 32);
	ui.play_pause_btn->setStyleSheet("QPushButton{border-image: url(:/button/Resources/icon/play_normal.png);}"
		"QPushButton:hover{border-image: url(:/button/Resources/icon/play_active.png);}"
		"QPushButton:pressed{border-image: url(:/button/Resources/icon/play_normal.png);}"
		"QPushButton {background-color:transparent;}");
	ui.next_btn->setFixedSize(32, 32);
	ui.next_btn->setStyleSheet("QPushButton{border-image: url(:/button/Resources/icon/next_normal.png);}"
		"QPushButton:hover{border-image: url(:/button/Resources/icon/next_active.png);}"
		"QPushButton:pressed{border-image: url(:/button/Resources/icon/next_normal.png);}"
		"QPushButton {background-color:transparent;}");

	/*//首先是设置主体
	QSlider{
	border-color: #bcbcbc;
	}
	QSlider::groove:horizontal {
		border: 1px solid #999999;
		height: 1px;
		margin: 0px 0;
		left: 5px; right: 5px;
	}
	//设置中间的那个滑动的键
	QSlider::handle:horizontal
	//还没有滑上去的地方
	QSlider::add-page:horizontal
	//已经划过的从地方
	QSlider::sub-page:horizontal*/
	ui.horizontalSlider->setStyleSheet("QSlider::groove:horizontal {border: 0px solid #bbb;}\
       QSlider::sub-page:horizontal{background: rgb(235,97,0);border-radius: 0px;margin-top:8px;margin-bottom:8px;}\
       QSlider::add-page:horizontal{background: rgb(255,255, 255);border: 0px solid #777;border-radius: 2px;margin-top:8px;margin-bottom:8px;}\
       QSlider::handle:horizontal{background: rgb(255,153,102);border: 1px solid rgb(255,153,102);width: 14px;height:10px;border-radius: 7px;margin-top:2px;margin-bottom:2px;}\
       QSlider::handle:horizontal:hover{background: rgb(255,128,6);border: 1px solid rgba(102,102,102,102);border-radius: 7px;}\
       QSlider::sub-page:horizontal:disabled{background: #bbb;border-color: #999;}\
       QSlider::add-page:horizontal:disabled{background: #eee;border-color: #999;}\
       QSlider::handle:horizontal:disabled{background: #eee;border: 1px solid #aaa;border-radius: 4px;}");
}

void QtPlayer::modPlayStatus(bool bIsPause)
{
	if (bIsPause)
	{
		ui.play_pause_btn->setFixedSize(32, 32);
		ui.play_pause_btn->setStyleSheet("QPushButton{border-image: url(:/button/Resources/icon/pause_normal.png);}"
			"QPushButton:hover{border-image: url(:/button/Resources/icon/pause_active.png);}"
			"QPushButton:pressed{border-image: url(:/button/Resources/icon/pause_normal.png);}"
			"QPushButton {background-color:transparent;}");
	}
	else
	{
		ui.play_pause_btn->setFixedSize(32, 32);
		ui.play_pause_btn->setStyleSheet("QPushButton{border-image: url(:/button/Resources/icon/play_normal.png);}"
			"QPushButton:hover{border-image: url(:/button/Resources/icon/play_active.png);}"
			"QPushButton:pressed{border-image: url(:/button/Resources/icon/play_normal.png);}"
			"QPushButton {background-color:transparent;}");
	}
}

void QtPlayer::playLiveStream(QString strFilePathName)
{
	if (startPlay(strFilePathName))
	{
		mbIsLive = true;
		forbitChildControl(mbIsLive);
	}
}

bool QtPlayer::startPlay(QString strFilePathName)
{
	if (mpInputStream)
	{
		mpInputStream->hide();
	}
	
	if (!mpDemuxThread)
	{
		mpDemuxThread = new XDemuxThread();
	}
	//const char *strFilePathName = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";//CCTV1
	if (mpDemuxThread->openMediaFile(strFilePathName.toLocal8Bit().data(), ui.openGLWidget))
	{
		mbIsHide = true;
		modPlayStatus(true);
		mbIsPause = false;
		return true;
	}
	return false;
}

void QtPlayer::forbitChildControl(bool isForbit)
{
	if (isForbit)
	{
		ui.stop_btn->setEnabled(false);
		ui.previous_btn->setEnabled(false);
		ui.play_pause_btn->setEnabled(false);
		ui.next_btn->setEnabled(false);
		ui.horizontalSlider->setValue(0);
		ui.horizontalSlider->setEnabled(false);
	}
	else
	{
		ui.stop_btn->setEnabled(true);
		ui.previous_btn->setEnabled(true);
		ui.play_pause_btn->setEnabled(true);
		ui.next_btn->setEnabled(true);
		ui.horizontalSlider->setValue(0);
		ui.horizontalSlider->setEnabled(true);
	}
}

void QtPlayer::openFile()
{
	QString strFilePathName = QFileDialog::getOpenFileName(this, "open file", ".", "*.*");
	if (strFilePathName.isEmpty())
	{
		QMessageBox::information(this, "Tips", "strFilePathName is empty!!!");
		return;
	}
	if (startPlay(strFilePathName))
	{
		mbIsLive = false;
		forbitChildControl(mbIsLive);
	}
}

void QtPlayer::openNetworkStream()
{
	if (mpInputStream)
	{
		mpInputStream->show();
	}
}

void QtPlayer::stopPlaySlot()
{
	if (mpDemuxThread)
	{
		mpDemuxThread->close();
		ui.horizontalSlider->setValue(0);
	}
}

void QtPlayer::pausePlay()
{
	if (mpDemuxThread)
	{
		if (mbIsPause)
		{
			mpDemuxThread->pauseThread(false);
		}
		else
		{
			mpDemuxThread->pauseThread(true);
		}
		modPlayStatus(mbIsPause);
		mbIsPause = !mbIsPause;
	}
}

void QtPlayer::resizeEvent(QResizeEvent *event)
{
	mnScaleValue = 1.0;
	mnPaperX = 0;
	mnPaperY = 0;
	ui.widget_2->move(50, this->height() - 100);
	ui.widget_2->resize(this->width() - 100, 100);
	ui.openGLWidget->resize(this->size());
	ui.openGLWidget->move(mnPaperX, mnPaperY);
	updateWidthOfPerPixel();
	mbIsWheelScale = false;
}

void QtPlayer::closeEvent(QCloseEvent *event)
{
	if (mpDemuxThread)
	{
		mpDemuxThread->StopThread();
	}
}

void QtPlayer::sliderPressedSlot(int nPosition)
{
	mbSliderPressed = true;
	if (mbIsLive)
	{
		return;
	}
	ui.horizontalSlider->setValue(nPosition);
}

void QtPlayer::sliderReleasedSlot()
{
	if (mbIsLive)
	{
		return;
	}
	if (mpDemuxThread)
	{
		mbSliderPressed = false;
		double ratio = 0.0;
		ratio = (double)ui.horizontalSlider->value() / (double)ui.horizontalSlider->maximum();
		qDebug() << "QtPlayer ratio: " << ratio;
		mpDemuxThread->seek(ratio);
	}
}

void QtPlayer::sliderMovedSlot(int nPosition)
{
	mbSliderPressed = true;
	if (mbIsLive)
	{
		return;
	}
	ui.horizontalSlider->setValue(nPosition);
}

void QtPlayer::timerEvent(QTimerEvent *event)
{
	if (mbSliderPressed)
	{
		return;
	}
	if (mpDemuxThread)
	{
		long long totalMs = mpDemuxThread->mlTotalMs;
		if (totalMs > 0)
		{
			if (mbIsLive)
			{
				return;
			}
			double ratio = (double)mpDemuxThread->pts / (double)totalMs;
			int nValue = ui.horizontalSlider->maximum()*ratio;
			ui.horizontalSlider->setValue(nValue);
			refreshPlayTime(mpDemuxThread->pts, totalMs);
		}
	}
}

void QtPlayer::refreshPlayTime(long long lCurPts, long long lTotalMs)
{
	//播放时间显示
	long lCurTime = qRound(static_cast<double>((double)lCurPts / (double)1000));
	//cout << "lCurTime: " << lCurTime << endl;
	int nSec = lCurTime % 60;
	//cout << "nSec: " << nSec << endl;
	int nMin = lCurTime / 60 % 60;
	//cout << "nMin: " << nMin << endl;
	int nHour = lCurTime / 3600;
	//cout << "nHour: " << nHour << endl;
	QString strCurTime = QString("%1:%2:%3").arg(nHour, 2, 10, QLatin1Char('0'))
		.arg(nMin, 2, 10, QLatin1Char('0')).arg(nSec, 2, 10, QLatin1Char('0'));
	ui.curTime->setText(strCurTime);

	//总时间
	long lTotalTime = qRound(static_cast<double>((double)lTotalMs / (double)1000));
	int nTsec = lTotalTime % 60;
	int nTmin = lTotalTime / 60 % 60;
	int nThour = lTotalTime / 3600;
	QString strTotalTime = QString("%1:%2:%3").arg(nThour, 2, 10, QLatin1Char('0'))
		.arg(nTmin, 2, 10, QLatin1Char('0')).arg(nTsec, 2, 10, QLatin1Char('0'));
	ui.totalTime->setText(strTotalTime);
	if ((lCurTime == lTotalTime) && mbIsHide)
	{
		modPlayStatus(false);
		//ui.openGLWidget->Init(demuxThread->mnWidth, demuxThread->mnHeight);
		//ui.openGLWidget->clear();
		mbIsHide = false;
	}
}

void QtPlayer::fastWard(double offset)
{
	if (mbSliderPressed)
	{
		return;
	}
	if (!mpDemuxThread)
	{
		return;
	}
	double ratio = 0.0;
	long long curPts = mpDemuxThread->pts;
	//qDebug() << "fastWard curPts1: " << curPts<<endl;
	long long totalMs = mpDemuxThread->mlTotalMs;
	if ((curPts >= (totalMs-offset)) && (curPts <totalMs))
	{
		curPts = totalMs;
	}
	else
	{
		curPts += offset;
	}
	//qDebug() << "fastWard curPts2: " << curPts << endl;
	ratio = (double)curPts / (double)totalMs;
	//qDebug() << "fastWard ratio: " << ratio;
	mpDemuxThread->seek(ratio);
}

void QtPlayer::backWard(double offset)
{
	if (mbSliderPressed)
	{
		return;
	}
	if (!mpDemuxThread)
	{
		return;
	}
	double ratio = 0.0;
	long long curPts = mpDemuxThread->pts;
	//qDebug() << "backWard curPts3: " << curPts << endl;
	long long totalMs = mpDemuxThread->mlTotalMs;
	if (curPts <= offset)
	{
		curPts = 0;
	}
	else
	{
		curPts -= offset;
	}
	ratio = (double)curPts / (double)totalMs;
	//qDebug() << "backWard curPts4: " << curPts << endl;
	//qDebug() << "backWard ratio: " << ratio << endl;
	mpDemuxThread->seek(ratio);
}

bool QtPlayer::eventFilter(QObject *target, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		int i = 0;
		if (keyEvent->key() == Qt::Key_Left)
		{
			backWard(mnOffset);
		}
		else if (keyEvent->key() == Qt::Key_Right)
		{
			fastWard(mnOffset);
		}
	}
	return false;
}

void QtPlayer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		QMenu *pMenu = new QMenu(this);
		pMenu->addAction(QIcon(), QStringLiteral("打开视频文件"), this, SLOT(openFile()));
		pMenu->addAction(QIcon(), QStringLiteral("打开网络串流"), this, SLOT(openNetworkStream()));
		pMenu->exec(QCursor::pos());
	}
}

void QtPlayer::wheelEvent(QWheelEvent *event)
{
	mbIsWheelScale = true;
	QPoint numPixels = event->pixelDelta();
	QPoint numDegrees = event->angleDelta() / 8;

	if (!numPixels.isNull()) {
		onWheelValueChanged(event->pos(), numPixels);
	}
	else if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;
		onWheelValueChanged(event->pos(), numSteps);
	}
	event->accept();
}

QPoint QtPlayer::mousePoint2PaperPoint(QPoint &point) {
	QPoint ret;
	//qDebug() << "MousePoint2PaperPoint...";
	//qDebug() << "point.x(): " << point.x() << "  point.y(): " << point.y() << "  mnPaperX: " << mnPaperX << "  mnPaperY: " << mnPaperY;
	ret.setX(point.x() - mnPaperX);
	ret.setY(point.y() - mnPaperY);
	return ret;
}

void QtPlayer::onWheelValueChanged(QPoint &mouse_pos, QPoint &step) {

	//if mouse point in paper
	if (mRect.contains(mouse_pos)) {
		QPoint before_resize_mouse_point_at_paper = mousePoint2PaperPoint(mouse_pos);
		//qDebug()<<"before_resize_mouse_point_at_paper: " <<before_resize_mouse_point_at_paper;
		int temp_paper_point_x = drawWidth2PaperWidth(before_resize_mouse_point_at_paper.x());
		int temp_paper_point_y = drawWidth2PaperWidth(before_resize_mouse_point_at_paper.y());
		//qDebug()<<"temp_paper_point_x: " <<temp_paper_point_x;
		//qDebug()<<"temp_paper_point_y: " <<temp_paper_point_y;

		//resize
		int step_value = step.y();
		mnScaleValue += static_cast<double>(step_value) / 20.0;
		if (mnScaleValue > SCALE_VALUE_MAX) mnScaleValue = SCALE_VALUE_MAX;
		if (mnScaleValue < SCALE_VALUE_MIN) mnScaleValue = SCALE_VALUE_MIN;
		updateWidthOfPerPixel();

		int temp_draw_point_x = paperWidth2DrawWidth(temp_paper_point_x);
		int temp_draw_point_y = paperWidth2DrawWidth(temp_paper_point_y);
		QPoint after_resize_mouse_point_at_paper(temp_draw_point_x, temp_draw_point_y);


		QPoint should_move_length = after_resize_mouse_point_at_paper - before_resize_mouse_point_at_paper;

		mnPaperX -= should_move_length.x();
		mnPaperY -= should_move_length.y();
		//qDebug() << "OnWheelValueChanged1 paper point changed mnPaperX:  " << mnPaperX << "  mnPaperY: " << mnPaperY;

		update();

	}
	else 
	{ //else using center resize
		int old_width = mRect.width();
		int old_height = mRect.height();

		//resize
		int step_value = step.y();
		mnScaleValue += static_cast<double>(step_value) / 20.0;
		if (mnScaleValue > SCALE_VALUE_MAX) mnScaleValue = SCALE_VALUE_MAX;
		if (mnScaleValue < SCALE_VALUE_MIN) mnScaleValue = SCALE_VALUE_MIN;
		updateWidthOfPerPixel();

		int new_width = paperWidth2DrawWidth(mnPaperWidth);
		int new_height = paperWidth2DrawWidth(mnPaperHeight);

		int adjusted_height = new_height - old_height;
		int adjusted_width = new_width - old_width;

		mnPaperX -= adjusted_width / 2;
		mnPaperY -= adjusted_height / 2;
		//qDebug()<<"OnWheelValueChanged1 paper point changed paper_x_:  " <<paper_x_<<"  paper_y_: "<<paper_y_;

		update();
	}
}

void QtPlayer::paintEvent(QPaintEvent *event)
{
	setRect();
}

/**
 * update widget of per pixel
 */
void QtPlayer::updateWidthOfPerPixel()
{
	if (this->height() < this->width()) {
		mdWidthPerPixel = calcWidthOfPerPixel(mnScaleValue, mnPaperWidth, this->width());
	}
	else {
		mdWidthPerPixel = calcWidthOfPerPixel(mnScaleValue, mnPaperWidth, this->height());
	}
	//qDebug() << "mdWidthPerPixel: " << mdWidthPerPixel;
}

/**
 * calc width of per pixel
 *
 * @param nScaleValue: scale factor 0.5~20
 * @param nPaperWidth: width of drawpaper 800
 * @param nWidgetWidth: width of current openglWidget
 * @return width of per pixel after resize
 */
double QtPlayer::calcWidthOfPerPixel(double &nScaleValue, int &nPaperWidth, const int &nWidgetWidth) {
	//qDebug() << "nScaleValue: " << nScaleValue
	//	<< "  nPaperWidth: " << nPaperWidth 
	//	<< "  nWidgetWidth: " << nWidgetWidth;
	int nScaledWidgetWidth = qRound(static_cast<double>(nWidgetWidth) * nScaleValue);

	double dWidgetOfPerPixel = 
		static_cast<double>(nPaperWidth) /
		static_cast<double>(nScaledWidgetWidth);
	//limit readable per-pixel value
	if (dWidgetOfPerPixel < 0.0005) dWidgetOfPerPixel = 0.0005;
	//qDebug()<<"paper_width_of_per_pixel: "<< paper_width_of_per_pixel;
	return dWidgetOfPerPixel;
}

/**
 * set rect position and width, height.
 * The rect for judge is if contain mouse 
 */
void QtPlayer::setRect()
{
	int nResizeWidth = paperWidth2DrawWidth(mnPaperWidth);
	int nResizeHeight = paperWidth2DrawWidth(mnPaperHeight);
	mRect.setX(mnPaperX);
	mRect.setY(mnPaperY);
	mRect.setWidth(nResizeWidth);
	mRect.setHeight(nResizeHeight);

	if (mbIsWheelScale)
	{
		resizeAndMoveGLWidget();
	}
	//qDebug() << "mRect: " << mRect;
}

void QtPlayer::resizeAndMoveGLWidget()
{
	ui.openGLWidget->resize(mRect.width(), mRect.height());
	ui.openGLWidget->move(mnPaperX, mnPaperY);
}

int QtPlayer::paperWidth2DrawWidth(int &paper_width)
{
	//qDebug() << "paperWidth2DrawWidth......";
	double draw_width = static_cast<double>(paper_width) / mdWidthPerPixel;
	//qDebug() << "draw_width:  " << draw_width << "   paper_width: " << paper_width << "   mdWidthPerPixel: " << mdWidthPerPixel;
	return qRound(draw_width);
}

int QtPlayer::drawWidth2PaperWidth(const int &draw_width) {
	//qDebug() << "DrawWidth2PaperWidth......";
	double paper_width = static_cast<double>(draw_width) * mdWidthPerPixel;
	//qDebug() << "draw_width:  " << draw_width << "   paper_width: " << paper_width << "   mdWidthPerPixel: " << mdWidthPerPixel;
	return static_cast<int>(qRound(paper_width));
}

