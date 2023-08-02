#include "planelocation.h"
#include <QDebug>
#include <QPainter>
#include <QWheelEvent>
#include <QPaintEvent>

PlaneLocation::PlaneLocation(QWidget *parent)
    : QWidget{parent}
{
    //默认场地大小，单位米，为了方便查看
    m_rect = QRect(0, 0, 20.0f, 20.0f);
    m_nScale = 0;
}


void PlaneLocation::drawPoint(QMap<QString, QPointF> map)
{
    m_mapPoint = map;
    update();
}

void PlaneLocation::paintEvent(QPaintEvent *event)
{
    int marginwidth = 40;   //右侧绘制文字的宽度
    int marginheight = 20;  //下边绘制文字的高度
    int gridNumber = 10;    //中间网格数量
    //需要提前设定好宽高是gridNumber的整数倍
    int w = width();
    int h = height();
    QPainter painter(this);
	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setColor("#0000FF");
	pen.setWidth(1);
    painter.setPen(pen);
    //painter.drawRect(0, 0, w - pen.width(), h - pen.width());
    //网格间隙值
    double xgap = (w - marginwidth) / gridNumber;
    double ygap = (h - marginheight) / gridNumber;
    int th = 10;    //坐标文字高度
    int m = 2;      //坐标轴左下角距0位差的单元格数
	 //放大与缩小控制
	int t = 20;
	if (m_nScale > 0) {
        t = m_rect.width() * qAbs(m_nScale + 1);
	}
	else if (m_nScale < 0) {
        int n = qAbs(m_nScale + 1);
        if (0 == n) n = 2;
        t = m_rect.width() / n;
	}
	QRect rect = QRect(0, 0, t, t);
    for (int i = 0; i <= gridNumber; i++) {
        //绘制网格
		pen.setColor("#AAAAAA");    //灰色网格
		painter.setPen(pen);
        int x = i * xgap;
        int y = i * ygap;
        painter.drawLine(x, 0, x, h - marginheight);    //竖轴
        painter.drawLine(0, y, w - marginwidth, y);     //横轴
        //绘制坐标文字
		pen.setColor("#467FC1");
		painter.setPen(pen);
        int n = rect.width() / gridNumber;
        int px = (i - m) * n;
        int py = rect.width() -(i + m) * n;
        if (0 == i) y += 4;
		if (i == gridNumber) {
			x -= 10;
            y -= 10;
		}
        painter.drawText(w - marginwidth + 2, y + th, QString::number(py));       //横轴坐标
        painter.drawText(x, h - marginheight + th + 4, QString::number(px));       //竖轴坐标
    }
    //绘制基站位置
    painter.setBrush(QBrush("#467FC1", Qt::SolidPattern));
	QStringList keys = m_mapPoint.keys();
	foreach(QString name, keys) {
		QPointF point = m_mapPoint.value(name);
        //计算坐标位置
        int x = point.x() / rect.width() * (w - marginwidth) + m * xgap;
        int y = h - point.y() / rect.height() * (h - marginheight) - marginheight - m * ygap;
        //绘制圆点，圆心放置到坐标点
        painter.drawEllipse(x - 4, y - 4, 8, 8);
		//绘制基站名称，文字放到圆点上方
		painter.drawText(x - th / 2, y - th / 2, name);
	}
}

void PlaneLocation::wheelEvent(QWheelEvent *event)
{
	QPoint numDegrees = event->angleDelta();
	if (numDegrees.y() > 0) {
		if (m_nScale <= -1) return;
		m_nScale--;
        update();
        
	} else {
		if (m_nScale >= 3) return;
		m_nScale++;
        update();
	}
}

