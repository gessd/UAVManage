#ifndef QWCHARTVIEW_H
#define QWCHARTVIEW_H


#include <QtCharts\QChartView>
#include <QtCharts\QtCharts>      //必须这么设置
#include <QVector>

QT_CHARTS_USE_NAMESPACE     //必须这么设置

//音波图表规范
struct AxisAndSeriesNorm
{
    uint ChannelCount;          //通道数量
    uint SampleSizes;           //采样位数
    quint64 frameCount;         //总帧数
    quint64 showFrameCount;     //展示数
    uint drawInterval;          //音波绘制间隔
    quint64 duration;           //播放时间  单位:微秒
    double timeScale;           //1微秒移动的位置
};

class QWChartView : public QChartView
{
    Q_OBJECT
private:
    QPoint  beginPoint;     //选择矩形区的起点
    QPoint  endPoint;       //选择矩形区的终点
    QChart  *m_chart;       //展示图表
    QLineSeries *m_series1;   //展示序列1
    QLineSeries *m_series2;   //展示序列2
    QLineSeries *m_series3;   //展示序列3 播放轨迹
    QCategoryAxis *m_axisX;     //X轴
    QCategoryAxis *m_axisY;     //Y轴
	QScatterSeries* m_timeNode;	//时间节点坐标点 － 散点序列
    AxisAndSeriesNorm m_norm; //图表规范
	QLabel* m_mouseLabel;		//鼠标跟踪标签
protected:
	void enterEvent(QEvent  *event);						//鼠标进入事件
	void leaveEvent(QEvent *event);							//鼠标离开事件
    void mousePressEvent(QMouseEvent *event);               //鼠标左键按下
    void mouseMoveEvent(QMouseEvent *event);                //鼠标移动
    void mouseReleaseEvent(QMouseEvent *event);             //鼠标释放左键
	void wheelEvent(QWheelEvent*event);						//滚轮事件
    void keyPressEvent(QKeyEvent *event);                   //按键事件
public:
    explicit QWChartView(QWidget *parent = 0);
    ~QWChartView();
    void resetAxisAndSeries(const AxisAndSeriesNorm& norm); //重置音波图表规范-序列和轴
    void axisXPlaying(qint64 duration);						//播放序列运行
    template<typename Customtype>							//添加序列坐标
    void addSeriesPoint(const Customtype * data,qint64 maxSize);
	void addTimeNode(const QVector<quint64>& timeNode);		//添加时间节点序列		
	void clearSeriesPoint();								//清理序列
private:
    void initSeries();      //初始化序列
    void initAxis();        //初始化XY轴
	void initMouseLabel();	//初始化鼠标跟踪标签
    void addAxisAndSeries();//添加序列和轴到图表中
    void prepareData();     //初始化序列数据
    void calcAxisX();       //根据音频文件排布X轴
signals:
    void mouseMovePoint(QPoint point); //鼠标移动信号，在mouseMoveEvent()事件中触发
    void updateMusicWaveFinished();
};
#endif // QWCHARTVIEW_H
