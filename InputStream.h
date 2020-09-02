#pragma once
#include <QtWidgets/QWidget>
#include "ui_InputStream.h"
class InputStream : public QWidget
{
	Q_OBJECT
public:
	InputStream(QWidget *parent = Q_NULLPTR);
	~InputStream();

signals:
	void startPlaySignal(QString);

private slots:
	void startPlay();

private:
	Ui::InputStreamClass ui;
};

