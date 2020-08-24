#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtPlayer.h"
#include "XDemuxThread.h"
#include <QCloseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QPaintEvent>

class QtPlayer : public QWidget
{
	Q_OBJECT

public:
	QtPlayer(QWidget *parent = Q_NULLPTR);
	~QtPlayer();

	void updateWidthOfPerPixel();
	double calcWidthOfPerPixel(double &scale_value,
		int &paper_width,
		const int &widget_width);
	void setRect();
	int paperWidth2DrawWidth(int &paper_width);
	void onWheelValueChanged(QPoint &mouse_pos, QPoint &step);
	QPoint mousePoint2PaperPoint(QPoint &point);
	int drawWidth2PaperWidth(const int &draw_width);
	void resizeAndMoveGLWidget();
protected:
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void paintEvent(QPaintEvent *event);
public slots:
	void openFile();
	void pausePlay();
	void pictureScale();

private:
	Ui::QtPlayerClass ui;

	bool mbIsWheelScale;

	const double SCALE_VALUE_MAX = 20.0;
	const double SCALE_VALUE_MIN = 0.5;

	double mdWidthPerPixel = 0.0;
	double mnScaleValue = 1.0;
	
	int mnWidth = 0;
	int mnHeight = 0;
	int mnPaperX = 0;
	int mnPaperY = 0;
	int mnPaperWidth = 1000;
	int mnPaperHeight = 1000;
	
	QRect mRect;
	XDemuxThread *demuxThread = NULL;
};
