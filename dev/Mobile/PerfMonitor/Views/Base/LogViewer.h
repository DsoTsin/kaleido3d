#pragma once

#include <QWidget>
#include <QMutex>

class LogViewerPrivate;
/*
 * High Performance Log Viewer
 */
class LogViewer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int RefreshInterval READ getRefreshInterval WRITE setRefreshInterval)
    Q_PROPERTY(int maxBufferedLines READ maxBufferedLines WRITE setMaxBufferedLines)
    Q_PROPERTY(bool scrollToEnd READ scrollToEnd WRITE setScrollToEnd)
public:
    LogViewer(QWidget* widget);
    virtual ~LogViewer() override;

    int         getRefreshInterval() const;
    void        setRefreshInterval(int interval);

    int         maxBufferedLines() const;
    void        setMaxBufferedLines(int maxLines);

    bool        scrollToEnd() const;
    void        setScrollToEnd(bool autoScrollToEnd);

    void        appendLog(QtMsgType logType, const QString& log);
    // shouldn't be called frequently, try to batch it
    void        notifyLogChanged();

    void        applyLogFilter(QString const& exp, bool isRegex);

protected:
    // @see http://doc.qt.io/qt-5/layout.html#custom-widgets-in-layouts
    // layout 
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    // events
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
#if QT_CONFIG(wheelevent)
    virtual void wheelEvent(QWheelEvent* event) override;
#endif
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    //virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    // measure
    virtual bool measureContent();

    struct QLogLine
    {
        int     no = 0;
        QtMsgType type = QtMsgType::QtInfoMsg;
        QString str;
        bool    bMeasured = false;
        bool    bNeedInvalidate = true;
        bool    bVisibleInRect = false;
        bool    bHasFilter = false;
        bool    filtered = false;
        int     filterStart = 0;
        int     measuredWidth = 0;
        int     measuredHeight = 0;
    };

private slots:
    void onClear();
    void onCopy(QString const& msg);

private:
    friend class LogViewerPrivate;
    LogViewerPrivate* m_LogContent;

    int     m_RefreshInterval;
    int     m_LineSpacing = 0;
    // horizontal scroll bar height
    int     hsbHeight = 10;
    // vertical scrollbar width
    int     vsbWidth = 10;
    qreal   sbRadius = 4.0;
    bool    m_ScrollToEnd = true;
};