#include "TimeLineView.h"
#include "Core/CoreMinimal.h"
#include <QTimer>
#include <QPainter>

TimeLineView::TimeLineView(QWidget* parent) : QWidget(parent)
{
    m_RefreshTimer = new QTimer(this);
    m_RefreshTimer->setInterval(50);
    connect(m_RefreshTimer, SIGNAL(timeout()), SLOT(repaint()));
}

TimeLineView::~TimeLineView()
{
    delete m_RefreshTimer;
}

void TimeLineView::Start()
{
    m_RefreshTimer->start();
}

void TimeLineView::Stop()
{
    m_RefreshTimer->stop();
}

void TimeLineView::AppendSampleData(int TrackIndex, TimelineSampleData const & Data)
{
    m_Tracks[0][TrackIndex].Push(Data);
}

void TimeLineView::paintEvent(QPaintEvent * event)
{
    int _width = width();
    int _height = height();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.translate(-2.0f, 0.0f);
    QPainterPath path;
    path.moveTo(_width - 20, _height);
    path.lineTo(_width - 20, 1.0f *_height * 0.2);
    path.lineTo(_width, 1.0f *_height * 0.5);
    path.lineTo(_width, _height);
    painter.setBrush(QBrush(QColor(0, 156, 130, 128)));
    painter.drawPath(path);
    painter.restore();
}
