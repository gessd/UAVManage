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
		int surplus = nWidget % m_nduration;
		int maxwidget = clearance * m_nduration;
		emit updateMaxWidget(maxwidget);
		//clearance++;
		for (int i = 0; i <= m_nduration; i++) {
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
		
	}
}
