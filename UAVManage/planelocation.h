#ifndef PLANELOCATION_H
#define PLANELOCATION_H

#include <QWidget>
#include <QPaintEvent>

class PlaneLocation : public QWidget
{
    Q_OBJECT
public:
    explicit PlaneLocation(QWidget *parent = nullptr);
    void drawPoint(QMap<QString, QPointF> map);
protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void wheelEvent(QWheelEvent *event);
private:
    //界面区域场地大小，正方形
    QRectF m_rect;
    int m_nScale;
    QMap<QString, QPointF> m_mapPoint;
};

#endif // PLANELOCATION_H
