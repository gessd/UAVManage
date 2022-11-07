#include "musicscale.h"
#include <QPainter>
#include <QDebug>

MusicScale::MusicScale(QWidget *parent)
	: QWidget(parent)
{
	m_nduration = 0;
}

MusicScale::~MusicScale()
{}

void MusicScale::updateScale(qint64 duration)
{
	m_nduration = duration / 1000;
	update();
}

void MusicScale::paintEvent(QPaintEvent* event)
{
	if(m_nduration > 0){
		qDebug() << "更新刻度";
		QPainter painter(this);
		QPen pen;
		pen.setStyle(Qt::SolidLine);
		pen.setColor("#0000FF");
		pen.setWidth(1);
		painter.setPen(pen);
		int nHeight = this->height();
		int nWidget = this->width();
		//painter.drawRect(0, 0, nWidget-1, nHeight-1);
		int clearance = nWidget / m_nduration;
		int surplus = nWidget% m_nduration;
		int maxwidget = clearance * m_nduration;
		updateMaxWidget(maxwidget);
		for (int i = 0; i <= m_nduration; i++) {
			int h = nHeight;
			int pw = 1;
			if(0 == i){
				pw = 3;
			}
			else if (0 == i % 60) {
				pw = 3;
			}
			else if (0 == i % 10) {
				pw = 2;
				h = nHeight / 2;
			}
			else {
				pw = 1;
				h = nHeight / 3;
			}
			pen.setWidth(pw);
			painter.setPen(pen);
			painter.drawLine(i * clearance, 0, i * clearance, h);
		}
		
	}
}
