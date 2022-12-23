#include "musicscale.h"
#include <QPainter>
#include <QDebug>

MusicScale::MusicScale(QWidget *parent)
	: QWidget(parent)
{
	m_nduration = 0;
	m_nCurrent = 0;
}

MusicScale::~MusicScale()
{}

void MusicScale::updateScale(qint64 duration)
{
	m_nduration = duration;
	update();
}

void MusicScale::setCurrentPosition(qint64 position)
{
	m_nCurrent = position;
	update();
}

void MusicScale::paintEvent(QPaintEvent* event)
{
	if(m_nduration > 0){
		QPainter painter(this);
		QPen pen;
		pen.setStyle(Qt::SolidLine);
		pen.setColor("#0000FF");
		pen.setWidth(1);
		painter.setPen(pen);
		int nHeight = this->height();
		int nWidget = this->width();
		//painter.drawRect(0, 0, nWidget-1, nHeight-1);
		int nSecondSum = m_nduration / 1000;
		int nMilli = m_nduration % 1000;
		int clearance = nWidget / nSecondSum;
		int surplus = nWidget % nSecondSum;
		int remainder = clearance * nMilli / 1000 + 1;
		int maxwidget = clearance * nSecondSum + remainder;
		emit updateMaxWidget(maxwidget);
		for (int i = 0; i <= nSecondSum; i++) {
			int h = nHeight;
			int pw = 1;
			if(0 == i){
				//起始位置
				pw = 1;
			}
			else if (0 == i % 60) {
				//分钟刻度
				pw = 3;
			}
			else if (0 == i % 10) {
				//10秒刻度
				pw = 2;
				h = nHeight / 2;
			}
			else {
				//秒
				pw = 1;
				h = nHeight / 4;
			}
			pen.setWidth(pw);
			painter.setPen(pen);
			painter.drawLine(i * clearance, nHeight, i * clearance, nHeight-h);
		}
		if (m_nCurrent > 0) {
			int n = m_nCurrent % 1000;
			int x = m_nCurrent / 1000;
			int pixelx = x * clearance + n * clearance / 1000;
			pen.setWidth(2);
			painter.setPen(pen);
			painter.drawLine(pixelx, 0, pixelx, height());
		}
	}
}
