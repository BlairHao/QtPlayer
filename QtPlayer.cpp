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

	setFocusPolicy(Qt::StrongFocus);
	installEventFilter(this);
	ui.pushButton->installEventFilter(this);
	ui.pushButton_2->installEventFilter(this);
	ui.horizontalSlider->installEventFilter(this);
	mbIsWheelScale = false;
	mbSliderPressed = false;
	mnScaleValue = 1.0;
	mnPaperWidth = 800;
	mnPaperHeight = 450;
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(ui.pushButton_2, SIGNAL(clicked()), this, SLOT(pausePlay()));
	connect(ui.horizontalSlider, SIGNAL(sliderPressed()), this, SLOT(sliderPressedSlot()));
	connect(ui.horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleasedSlot()));
	connect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMovedSlot(int)));
	updateWidthOfPerPixel();
	startTimer(40);
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
	//const char *strFilePathName = "http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8";//CCTV1
	demuxThread->Start();
	demuxThread->openMediaFile(strFilePathName.toLocal8Bit().data(), ui.openGLWidget);
	//demuxThread->openMediaFile(strFilePathName, ui.openGLWidget);
}

void QtPlayer::pausePlay()
{
	demuxThread->pauseThread();
}

void QtPlayer::resizeEvent(QResizeEvent *event)
{
	mnScaleValue = 1.0;
	mnPaperX = 0;
	mnPaperY = 0;
	//ui.horizontalSlider->move(50, this->height() - 100);
	//ui.horizontalSlider->resize(this->width() - 100, ui.horizontalSlider->height());
	ui.widget_2->move(100, this->height() - 100);
	ui.widget_2->resize(this->width() - 200, 100);
	ui.openGLWidget->resize(this->size());
	ui.openGLWidget->move(mnPaperX, mnPaperY);
	updateWidthOfPerPixel();
	mbIsWheelScale = false;
}

void QtPlayer::closeEvent(QCloseEvent *event)
{
	if (demuxThread)
	{
		demuxThread->StopThread();
	}
}

void QtPlayer::sliderPressedSlot()
{
	mbSliderPressed = true;
}

void QtPlayer::sliderReleasedSlot()
{
	mbSliderPressed = false;
	double ratio = 0.0;
	ratio = (double)ui.horizontalSlider->value() / (double)ui.horizontalSlider->maximum();
	qDebug() << "QtPlayer ratio: " << ratio;
	demuxThread->seek(ratio);
}

void QtPlayer::sliderMovedSlot(int nPosition)
{
	mbSliderPressed = true;
}

void QtPlayer::timerEvent(QTimerEvent *event)
{
	if (mbSliderPressed)
	{
		return;
	}
	if (demuxThread)
	{
		long long totalMs = demuxThread->mlTotalMs;
		if (totalMs > 0)
		{
			double ratio = (double)demuxThread->pts / (double)totalMs;
			int nValue = ui.horizontalSlider->maximum()*ratio;
			ui.horizontalSlider->setValue(nValue);
		}
	}
}

void QtPlayer::fastWard(double offset)
{
	if (mbSliderPressed)
	{
		return;
	}
	double ratio = 0.0;
	long long curPts = demuxThread->pts;
	qDebug() << "fastWard curPts1: " << curPts;
	long long totalMs = demuxThread->mlTotalMs;
	if ((curPts >= (totalMs-offset)) && (curPts <totalMs))
	{
		curPts = totalMs;
	}
	else
	{
		curPts += offset;
	}
	qDebug() << "fastWard curPts2: " << curPts;
	ratio = (double)curPts / (double)totalMs;
	qDebug() << "fastWard ratio: " << ratio;
	demuxThread->seek(ratio);
}

void QtPlayer::backWard(double offset)
{
	if (mbSliderPressed)
	{
		return;
	}
	double ratio = 0.0;
	long long curPts = demuxThread->pts;
	qDebug() << "backWard curPts3: " << curPts;
	long long totalMs = demuxThread->mlTotalMs;
	if (curPts <= offset)
	{
		curPts = 0;
	}
	else
	{
		curPts -= offset;
	}
	ratio = (double)curPts / (double)totalMs;
	qDebug() << "backWard curPts4: " << curPts;
	qDebug() << "backWard ratio: " << ratio;
	demuxThread->seek(ratio);
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

