#include "LogViewer.h"
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>
#include <QMenu>
#include <QSize>
#include <QBrush>
#include <QColor>
#include <QAction>
#include <QMutexLocker>

class LogViewerPrivate
{
public:
    explicit LogViewerPrivate(LogViewer* viewer);

    QVector<LogViewer::QLogLine> m_LogLines;
    int currentTopIndex = 0;
    int visibleLines = 0;
    int currentMaxWidth = 0;
    int maxLines = 500;/* twice of the num will trigger removing */
    int lineSpace = 0;
    int vOffset = 0;
    int hOffset = 0;
    bool bScrollingX = false;
    bool bScrollingY = false;
    bool filtering = false;
    QString filterText;
    QMutex  logMutex;
    /* paint */
    void paint(QPainter* painter, QPoint const& pos);
    void measure(int width, int height);
    void scrollUp(int lines);
    void scrollDown(int lines);
    void scrollHorizontal(int offSet);
    void scrollVertical(int offSet);
    void clearAll();

    void setFilter(QString const& str, bool regEx);
    void insertLine(LogViewer::QLogLine& line);

    //input event
    void mousePressEvent(QMouseEvent * event); 
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void contextMenuEvent(QContextMenuEvent *event);
private:
    bool isInVsb(QPoint const& pos);
    bool isInHsb(QPoint const& pos);
    QPoint lastPressed;
    bool bHasSelectedLines = false;

    QAction* aCopy = nullptr;
    QAction* aClear = nullptr;

    LogViewer * thiz = nullptr;
};

/// Log Viewer

LogViewer::LogViewer(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(60);
    setMinimumWidth(80);
    m_LogContent = new LogViewerPrivate(this);
}

LogViewer::~LogViewer()
{
    delete m_LogContent;
}

int LogViewer::getRefreshInterval() const
{
    return m_RefreshInterval;
}

void LogViewer::setRefreshInterval(int interval)
{
    m_RefreshInterval = interval;
}

int LogViewer::maxBufferedLines() const
{
    return m_LogContent->maxLines;
}

void LogViewer::setMaxBufferedLines(int maxLines)
{
    m_LogContent->maxLines = maxLines;
}

bool LogViewer::scrollToEnd() const
{
    return m_ScrollToEnd;
}

void LogViewer::setScrollToEnd(bool autoScrollToEnd)
{
    m_ScrollToEnd = autoScrollToEnd;
}

void LogViewer::appendLog(QtMsgType logType, const QString & log)
{
    QLogLine line;
    line.type = logType;
    line.str = log;
    m_LogContent->insertLine( line );
}

void LogViewer::notifyLogChanged()
{
    if (m_ScrollToEnd && m_LogContent->m_LogLines.size() > 0)
    {
        m_LogContent->currentTopIndex = m_LogContent->m_LogLines.size() - height() / fontMetrics().height() - 2;
    }
    repaint();
}

void LogViewer::applyLogFilter(QString const & exp, bool isRegex)
{
    m_LogContent->setFilter(exp, isRegex);
}

QSize LogViewer::sizeHint() const
{
    return QSize(width(), height());
}

QSize LogViewer::minimumSizeHint() const
{
    return QSize(width(), height());
}

void LogViewer::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    QMutexLocker lock(&m_LogContent->logMutex);
    // measure log content
    measureContent();
    m_LogContent->paint(&painter, QPoint());
}

void LogViewer::contextMenuEvent(QContextMenuEvent * event)
{
    m_LogContent->contextMenuEvent(event);
}

#if QT_CONFIG(wheelevent)
void LogViewer::wheelEvent(QWheelEvent * event)
{
    QPoint delta = event->angleDelta();
    if (delta.y() > 0)
    {
        m_LogContent->scrollUp(2);
    }
    else
    {
        m_LogContent->scrollDown(2);
    }
    repaint();
}
void LogViewer::mousePressEvent(QMouseEvent * event)
{
    m_LogContent->mousePressEvent(event);
}
void LogViewer::mouseReleaseEvent(QMouseEvent * event)
{
    if (m_LogContent->bScrollingX)
    {
        m_LogContent->bScrollingX = false;
        m_ScrollToEnd = true;
    }
    m_LogContent->mouseReleaseEvent(event);
}
void LogViewer::mouseMoveEvent(QMouseEvent * event)
{
    m_LogContent->mouseMoveEvent(event);
}
#endif

bool LogViewer::measureContent()
{
    m_LogContent->measure(width(), height());
    return false;
}

void LogViewer::onCopy(QString const & msg)
{
}

void LogViewer::onClear()
{
}

LogViewerPrivate::LogViewerPrivate(LogViewer * viewer)
    : thiz(viewer)
{
    aCopy = new QAction(thiz->tr("Copy"), thiz);
    aClear = new QAction(thiz->tr("Clear"), thiz);
    QObject::connect(aClear, SIGNAL(triggered()), thiz, SLOT(onClear()));
    QObject::connect(aCopy, SIGNAL(triggered()), thiz, SLOT(onClear()));
}

void LogViewerPrivate::paint(QPainter * painter, QPoint const& pos)
{
    int fH = thiz->fontMetrics().height();
    painter->setPen(QPen(QColor(Qt::GlobalColor::green)));
    painter->setBrush(QBrush(Qt::GlobalColor::black));
    painter->drawRect(QRect(0, 0, thiz->width(), thiz->height()));
    int maxVisibleWidth = 0;

    if (!filtering)
    {
        for (int i = currentTopIndex;
            i < currentTopIndex + visibleLines && i < m_LogLines.size();
            i++)
        {
            if (m_LogLines[i].measuredWidth > maxVisibleWidth)
            {
                maxVisibleWidth = m_LogLines[i].measuredWidth;
            }
            switch (m_LogLines[i].type)
            {
            case QtMsgType::QtDebugMsg:
                painter->setPen(QPen(QColor(Qt::GlobalColor::white)));
                break;
            case QtMsgType::QtFatalMsg:
                painter->setPen(QPen(QColor(Qt::GlobalColor::red)));
                break;
            case QtMsgType::QtWarningMsg:
                painter->setPen(QPen(QColor(Qt::GlobalColor::yellow)));
                break;
            case QtMsgType::QtInfoMsg:
            default:
                painter->setPen(QPen(QColor(Qt::GlobalColor::green)));
                break;
            }
            painter->drawText(QPoint(hOffset, (i - currentTopIndex) * (fH + lineSpace)), m_LogLines[i].str);
        }
    }
    else
    {

    }
        
    painter->setPen(QPen(QColor(Qt::GlobalColor::green)));
    currentMaxWidth = maxVisibleWidth;
    int hbarWidth = (int)(1.0f * thiz->width() / currentMaxWidth * (thiz->width() - thiz->vsbWidth));
    // draw horizontal scroll
    painter->drawRoundedRect(0, thiz->height() - thiz->hsbHeight - 2, thiz->width() - thiz->vsbWidth, thiz->hsbHeight, thiz->sbRadius, thiz->sbRadius);
    // draw vertical scroll
    painter->drawRoundedRect(thiz->width() - thiz->vsbWidth - 2, 0, thiz->vsbWidth, thiz->height() - thiz->hsbHeight, thiz->sbRadius, thiz->sbRadius);

    painter->setBrush(QBrush(Qt::GlobalColor::green));
    painter->drawRoundedRect(-hOffset, thiz->height() - thiz->hsbHeight - 2, hbarWidth, thiz->hsbHeight, thiz->sbRadius, thiz->sbRadius);
}

bool LogViewerPrivate::isInVsb(QPoint const & pos)
{
    return pos.x() > thiz->width() - thiz->vsbWidth - 2 && pos.x() < thiz->width()
        && pos.y() > 0 && pos.y() < thiz->height() - thiz->hsbHeight;
}

bool LogViewerPrivate::isInHsb(QPoint const & pos)
{
    return pos.x() > 0 && pos.x() < thiz->width() - thiz->vsbWidth
        && pos.y() > thiz->height() - thiz->hsbHeight - 2 && pos.y() < thiz->height();
}

void LogViewerPrivate::mousePressEvent(QMouseEvent * event)
{
    lastPressed = event->pos();
    if (isInHsb(event->pos()))
    {
        bScrollingX = true;
        thiz->m_ScrollToEnd = false;
    }
    else if (isInVsb(event->pos()))
    {
        bScrollingY = true;
    }
    else
    {

    }
}

void LogViewerPrivate::mouseMoveEvent(QMouseEvent * event)
{
    if (bScrollingX)
    {
        scrollHorizontal(-1 * (event->x() - lastPressed.x()));
        thiz->repaint();
    }
}

void LogViewerPrivate::mouseReleaseEvent(QMouseEvent * event)
{
    if (bScrollingY)
        bScrollingY = false;
    if (isInHsb(event->pos()))
    {
        bScrollingX = false;
    }
    else if (isInVsb(event->pos()))
    {
        bScrollingY = false;
    }
    else
    {

    }
}

void LogViewerPrivate::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu menu(thiz);
    menu.addAction(aCopy);
    menu.addAction(aClear);
    menu.exec(event->globalPos());
}

void LogViewerPrivate::measure(int width, int height)
{
    int fH = thiz->fontMetrics().height(); + lineSpace;
    visibleLines = (height - thiz->hsbHeight) / fH + 1;

    for (int i = currentTopIndex; i < currentTopIndex + visibleLines && i < m_LogLines.size(); i++)
    {
        if (!m_LogLines[i].bMeasured)
        {
            int width = thiz->fontMetrics().width(m_LogLines[i].str);
            m_LogLines[i].measuredWidth = width;
            m_LogLines[i].bMeasured = true;
        }
    }
}

void LogViewerPrivate::scrollUp(int lines)
{
    currentTopIndex -= lines;
    if (currentTopIndex <= 0)
        currentTopIndex = 0;
}

void LogViewerPrivate::scrollDown(int lines)
{
    currentTopIndex += lines;
}

void LogViewerPrivate::scrollHorizontal(int offSet)
{
    hOffset = offSet;
}

void LogViewerPrivate::scrollVertical(int offSet)
{
}

void LogViewerPrivate::clearAll()
{
    m_LogLines.clear();
}

void LogViewerPrivate::setFilter(QString const & str, bool regEx)
{
    filterText = str;
    filtering = true;
}

void LogViewerPrivate::insertLine(LogViewer::QLogLine& line)
{
    logMutex.lock();
    if (m_LogLines.size() <= maxLines * 2)
    {
        if (filtering)
        {
            line.filterStart = line.str.indexOf(filterText);
            line.filtered = true;
        }
        m_LogLines << line;
    }
    if (m_LogLines.size() > maxLines * 2)
    {
        for(int i = 0; i< (int)(1.75f* maxLines); i++)
            m_LogLines.pop_front();
    }
    logMutex.unlock();
}
