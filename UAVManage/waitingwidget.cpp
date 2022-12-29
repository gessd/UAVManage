#include "waitingwidget.h"
#include "ui_waitingwidget.h"

WaitingWidget::WaitingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitingWidget)
{
    ui->setupUi(this);
	QMovie* pMovie = new QMovie(":/res/images/waiting.gif");
	ui->labelGif->setMovie(pMovie);
	pMovie->start();
}

WaitingWidget::~WaitingWidget()
{
    delete ui;
}

void WaitingWidget::visibleWidget()
{
    ui->labelGif->setVisible(false);
    ui->labelMessage->setVisible(false);
}
