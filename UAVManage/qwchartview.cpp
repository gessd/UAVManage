#include "qwchartview.h"

QWChartView::QWChartView(QWidget *parent):QChartView(parent)
{
    //setDragMode(QGraphicsView::RubberBandDrag);//拖动鼠标可以设置矩形
    //    this->setRubberBand(QChartView::RectangleRubberBand);//设置为矩形选择方式
    //    this->setRubberBand(QChartView::VerticalRubberBand);
    //    this->setRubberBand(QChartView::HorizontalRubberBand);
    
    setMouseTracking(true);           //必须开启此功能，启用鼠标跟踪
    m_chart = new QChart();
	m_chart->layout()->setContentsMargins(0, 0, 0, 0);//设置外边界全部为0
	m_chart->setMargins(QMargins(0, 0, 0, 0));//设置内边界全部为0
	m_chart->setBackgroundRoundness(0);//设置背景区域无圆角
	m_chart->legend()->setVisible(false);
    m_chart->setBackgroundBrush(QBrush(QColor(0x00, 0x00, 0x00, 0x00)));
    //内外边距全都设置为0
	this->setViewportMargins(0, 0, 0, 0);
	this->setContentsMargins(0, 0, 0, 0);
    setChart(m_chart);
    setRenderHint(QPainter::Antialiasing);  //消除边缘锯齿
    setCursor(Qt::CrossCursor);             //设置鼠标指针为十字星
	initMouseLabel();
    initSeries();
    initAxis();
    addAxisAndSeries();
	prepareData();
}

QWChartView::~QWChartView()
{
    delete m_series1;
    delete m_series2;
    delete m_axisX;
    delete m_axisY;
    delete m_chart;
}

void QWChartView::resetAxisAndSeries(const AxisAndSeriesNorm &norm)
{//重置音波图表规范-序列和轴
    if(m_norm.SampleSizes == norm.SampleSizes
            && m_norm.ChannelCount == norm.ChannelCount
            && m_norm.frameCount == norm.frameCount)
        return;
    m_norm = norm;
    m_series1->clear();
    m_series2->clear();
    m_series3->clear();
    m_norm.timeScale = norm.showFrameCount / (norm.duration / 1.00); //计算一微秒移动的距离

    /*calcAxisX();*/
	m_axisX->setRange(0, 5000);
    if(norm.ChannelCount == 1)
        m_axisY->setRange(0, 126);
    else if(norm.ChannelCount == 2)
        m_axisY->setRange(0, 20000);
}

void QWChartView::initSeries()
{//初始化序列
    m_series1 = new QLineSeries();
    m_series2 = new QLineSeries();
    m_series3 = new QLineSeries();

    QPen    pen;
    pen.setStyle(Qt::SolidLine);//Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine
    pen.setColor(Qt::green);
    pen.setWidth(1);
    m_series1->setPen(pen);
    pen.setColor("#7DFF7B");
    m_series2->setPen(pen);
    pen.setColor(Qt::blue);
	pen.setWidth(2);
    m_series3->setPen(pen);

	m_timeNode = new QScatterSeries();
	m_timeNode->setMarkerShape(QScatterSeries::MarkerShapeCircle);//MarkerShapeRectangle,MarkerShapeCircle
	m_timeNode->setBorderColor(Qt::red);
	m_timeNode->setBrush(QBrush(Qt::darkCyan));
	m_timeNode->setMarkerSize(12);
}

void QWChartView::initAxis()
{//初始化坐标轴
    m_axisX = new QCategoryAxis;
    m_axisX->setRange(0, 5000); //设置坐标轴范围
//    m_axisX->setLabelFormat("%.1f"); //标签格式
    m_axisX->setTickCount(11); //主分隔个数
    m_axisX->setMinorTickCount(2);//4
    //m_axisX->setTitleText("time(secs)"); //标题
	m_axisX->setTitleVisible(false);
    m_axisX->setGridLineVisible(true);
    //隐藏Y轴刻度尺
    m_axisX->setVisible(false);

    m_axisY = new QCategoryAxis;
    m_axisY->setRange(-100, 100);
   /* m_axisY->setTitleText("value");*/
    m_axisY->setTickCount(5);
    m_axisY->setLabelFormat("%.2f"); //标签格式
    m_axisY->setGridLineVisible(true);
    m_axisY->setMinorTickCount(2);//4
	m_axisY->setTitleVisible(false);
    //隐藏Y轴刻度尺
    m_axisY->setVisible(false);
}

void QWChartView::initMouseLabel()
{//初始化鼠标标签
	m_mouseLabel = new QLabel(this);
	m_mouseLabel->setWindowFlags(Qt::FramelessWindowHint); //设置不显示边框
	//设置QLabel背景透明
	QPalette pal;
	pal.setColor(QPalette::Background, QColor(0x00, 0xff, 0x00, 0x00));
	//设置QLabel颜色
	pal.setColor(QPalette::WindowText, Qt::cyan);
	m_mouseLabel->setPalette(pal);
	m_mouseLabel->hide();
}

void QWChartView::addAxisAndSeries()
{//添加坐标轴和序列值
    m_chart->addSeries(m_series1);
    m_chart->addSeries(m_series2);
    m_chart->addSeries(m_series3);
	//m_chart->addSeries(m_timeNode);

    m_chart->addAxis(m_axisX,Qt::AlignBottom); //坐标轴添加到图表，并指定方向
    m_chart->addAxis(m_axisY,Qt::AlignLeft);

    m_series1->attachAxis(m_axisX); //序列 series1 附加坐标轴
    m_series1->attachAxis(m_axisY);

    m_series2->attachAxis(m_axisX); //序列 series2 附加坐标轴
    m_series2->attachAxis(m_axisY);

    m_series3->attachAxis(m_axisX); //序列 series1 附加坐标轴
    m_series3->attachAxis(m_axisY);

	//m_timeNode->attachAxis(m_axisX);
	//m_timeNode->attachAxis(m_axisY);
}

void QWChartView::prepareData()
{//初始化数据
    return;
    m_series1->clear();
    m_series2->clear();
    m_series3->clear();
    qsrand(QTime::currentTime().second());//随机数初始化
    qreal   t=0,y1,y2,intv=1;
    qreal   rd;
    int cnt = 5000;
    for(int i=0;i<cnt;i++)
    {
        rd=(qrand() % 100)-5; //随机数,-5~+5
        y1=qSin(t)+rd/2;//+qrand();
        m_series1->append(t,y1);

        rd=(qrand() % 100)-5; //随机数,-5~+5
        y2=qSin(t+20)+rd/2;
        m_series2->append(t,y2);

		if (i % 500 == 0)
			m_timeNode->append(t, m_axisY->max() - 5);

        t+=intv;
    }
}

void QWChartView::calcAxisX()
{//根据音频文件排布X轴
    //1秒 = 1000豪秒 = 1000000微秒
    int timeSec = m_norm.duration;
    double timeScale =  m_norm.showFrameCount / (m_norm.duration / 1.00);   //1微秒所移动的距离,用于排布X抽
    int label = 0;
    QString durationTime;					//轴时间展示
    quint64 timeUnits;                      //时间轴单位
	QStringList strList = m_axisX->categoriesLabels();
	for (QString str : strList)
		m_axisX->remove(str);
	m_axisX->setRange(0, m_norm.showFrameCount);
    if(timeSec < 20000000)
    {
        timeUnits = 1000000;
        m_axisX->setTitleText("time(secs)"); //标题
    }
    else if(timeSec < 60000000)
    {
        m_axisX->setTitleText("time(secs)"); //标题
        timeUnits = 10000000;
    }
    else {
        m_axisX->setTitleText("time(min)"); //标题
        timeUnits = 60000000;
    }
    while ((timeSec -= timeUnits) > 0)
    {
        ++label;
        if(timeUnits == 1000000)
            m_axisX->append(QString::asprintf("0:%d",label),timeScale * label * timeUnits);
        else if(timeUnits == 10000000)
            m_axisX->append(QString::asprintf("0:%d0",label),timeScale * label * timeUnits);
        else
            m_axisX->append(QString::asprintf("%d:00",label),timeScale * label * timeUnits);
    }
    if(timeSec += timeUnits > 1000000)
    {
        int   secs= m_norm.duration/1000000;//秒
        int   mins=secs/60; //分钟
        secs=secs % 60;//余数秒
        if(secs < 10)
            durationTime=QString::asprintf("%d:0%d",mins,secs);
        else
            durationTime=QString::asprintf("%d:%d",mins,secs);
        m_axisX->append(durationTime,m_norm.showFrameCount);
    }
}

void QWChartView::axisXPlaying(qint64 duration)
{//更新播放时间轴
    m_series3->clear();
    QVector<QPointF> points3;
    points3.append(QPointF(duration * m_norm.timeScale, m_axisY->min()));
    points3.append(QPointF(duration * m_norm.timeScale, m_axisY->max()));
    m_series3->replace(points3);
}

void QWChartView::mousePressEvent(QMouseEvent *event)
{//鼠标左键按下，记录beginPoint
    if (event->button()==Qt::LeftButton)
        beginPoint=event->pos();
    QChartView::mousePressEvent(event);
}

void QWChartView::enterEvent(QEvent  *event)
{//鼠标进入事件
    if (isActiveWindow()) {
        m_mouseLabel->setMinimumWidth(80);
        m_mouseLabel->show();
    }
}
void QWChartView::leaveEvent(QEvent *event)
{//鼠标移出事件
	m_mouseLabel->hide();
}

void QWChartView::mouseMoveEvent(QMouseEvent *event)
{//鼠标移动事件
    QPoint  point;
    point=event->pos();
	QPointF pt = m_chart->mapToValue(point); //转换为图表的数值
	double time = pt.x() / m_norm.timeScale;
	QString strMic, strSec, strMin; //豪秒，秒，分钟字符串
	int mic, secs, mins;			//豪秒，秒，分钟
	mic = time / 1000;				//豪秒
	secs = time / 1000000;			//秒
	mins = secs / 60;				//分钟
	mic = mic % 1000;				//余数微秒
	secs = secs % 60;				//余数秒
	strMin = mins < 10 ? QString::asprintf("0%1").arg(QString::number(mins)) : QString::number(mins);
	strSec = secs < 10 ? QString::asprintf("0%1").arg(QString::number(secs)) : QString::number(secs);
	strMic = mic < 100 ? "0" : strMic;
	strMic = mic < 10 ? "00" : strMic;
	strMic.append(QString::number(mic));
	
	QString durationTime = QString::asprintf("%1:%2:%3").arg(strMin).arg(strSec).arg(strMic);
	m_mouseLabel->setText(durationTime);
    int x = point.x();
    int y = point.y();
    if (x + m_mouseLabel->width() > this->width()) {
        x = x - m_mouseLabel->width();
    }
    m_mouseLabel->move(QPoint(x + 10, point.y() - 20));		//跟踪在鼠标右上角显示
    /*emit mouseMovePoint(point);*/
    QChartView::mouseMoveEvent(event);
}

void QWChartView::wheelEvent(QWheelEvent* event)
{//滚动事件
    /*
	if (event->delta() > 0) {//如果滚轮往上滚
		chart()->zoom(1.2);
	}
	else {//同样的
		chart()->zoom(0.8);
	}*/
}

void QWChartView::mouseReleaseEvent(QMouseEvent *event)
{//单击事件
    /*
	if (event->button() == Qt::LeftButton)
	{ //鼠标左键释放，获取矩形框的endPoint,进行缩放
		endPoint = event->pos();
		QRectF  rectF;
		rectF.setTopLeft(this->beginPoint);
		rectF.setBottomRight(this->endPoint);
		this->chart()->zoomIn(rectF);
	}
	else if (event->button() == Qt::RightButton)
		this->chart()->zoomReset(); //鼠标右键释放，resetZoom
    */
    QChartView::mouseReleaseEvent(event);
}

void QWChartView::keyPressEvent(QKeyEvent *event)
{//按键控制
    /*
	switch (event->key()) {
	case Qt::Key_Plus:  //+
		chart()->zoom(1.2);
		break;
	case Qt::Key_Minus:
		chart()->zoom(0.8);
		break;
	case Qt::Key_Left:
		chart()->scroll(10, 0);
		break;
	case Qt::Key_Right:
		chart()->scroll(-10, 0);
		break;
	case Qt::Key_Up:
		chart()->scroll(0, -10);
		break;
	case Qt::Key_Down:
		chart()->scroll(0, 10);
		break;
	case Qt::Key_PageUp:
		chart()->scroll(0, 50);
		break;
	case Qt::Key_PageDown:
		chart()->scroll(0, -50);
		break;
	case Qt::Key_Home:
		chart()->zoomReset();
		break;
	default:
		QGraphicsView::keyPressEvent(event);
	}
    */
        QGraphicsView::keyPressEvent(event);
}

template<typename Customtype>
void QWChartView::addSeriesPoint(const Customtype * data,qint64 maxSize)
{
    int index = 0;
    if(m_norm.ChannelCount == 1)
    {
        QVector<QPointF> oldPoints = m_series1->pointsVector();
        QVector<QPointF> points1;
        if (oldPoints.count() < m_norm.showFrameCount)
        { //序列的数据未满要显示的帧数
            points1 = m_series1->pointsVector();
        }
        else
        {//将原来maxSize至4000的数据点前移，
            for (int i = maxSize; i < oldPoints.count(); i++)
                points1.append(QPointF(i - maxSize, oldPoints.at(i).y()));
        }
        qint64 size = points1.count();
        for (int k = 0; k < maxSize / m_norm.drawInterval; k++) //数据块内的数据填充序列的尾部
        {
            points1.append(QPointF(k + size, data[index]));
            index += m_norm.drawInterval;
        }
        m_series1->replace(points1); //最快的方式
    }
    else if(m_norm.ChannelCount == 2)
    {
        QVector<QPointF> oldPoints1 = m_series1->pointsVector(),oldPoints2 = m_series2->pointsVector();
        QVector<QPointF> points1,points2;
        if (oldPoints1.count() < m_norm.showFrameCount)
        { //序列的数据未满要显示的帧数
            points1 = m_series1->pointsVector();
            points2 = m_series2->pointsVector();
        }
        else
        {//将原来maxSize至4000的数据点前移，
            for (int i = maxSize; i < oldPoints1.count(); i++)
            {
                points1.append(QPointF(i - maxSize, oldPoints1.at(i).y()));
                points2.append(QPointF(i - maxSize, oldPoints2.at(i).y()));
            }
        }
        qint64 size = points1.count();
        for (int k = 0; k < maxSize / m_norm.drawInterval; k++) //数据块内的数据填充序列的尾部
        {
            points1.append(QPointF(k + size, data[index++]));
            points2.append(QPointF(k + size, data[index]));
            index += m_norm.drawInterval;
        }
        m_series1->replace(points1); //最快的方式
        m_series2->replace(points2); //最快的方式

		//假设一段动作节点
		QVector<quint64> timeNode;
		timeNode.push_back(2000000);
		timeNode.push_back(8000000);
		addTimeNode(timeNode);
    }
    emit updateMusicWaveFinished();
}

void QWChartView::addTimeNode(const QVector<quint64>& timeNode)
{//更新时间节点
	m_timeNode->clear();
	QVector<QPointF> nodePoints;
	for (auto mec : timeNode)
	{
		nodePoints.append(QPointF(mec * m_norm.timeScale, m_axisY->max() - 10));
	}
	m_timeNode->replace(nodePoints);
}

void QWChartView::clearSeriesPoint()
{
	m_series1->clear();
	m_series2->clear();
	m_series3->clear();
	m_timeNode->clear();
}
template void QWChartView::addSeriesPoint<qint8>(const qint8 * data,qint64 maxSize);
template void QWChartView::addSeriesPoint<qint16>(const qint16 * data,qint64 maxSize);
