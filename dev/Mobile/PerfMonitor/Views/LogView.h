#pragma once

#include "Core/CoreMinimal.h"
#include <QWidget>
#include <QSyntaxHighlighter>
#include <QContextMenuEvent>
#include <QList>
#include <QVector>
#include <QMutex>

class QLineEdit;
class QTimer;
class QRegExp;
class QToolButton;
class QTextEdit;

namespace k3d
{
    namespace mobi
    {
        class IDevice;
    }
}

class Logger : public k3d::ILogger
{
public:
    Logger();
    virtual ~Logger() override;

    struct LogEntry
    {
        k3d::ELogLevel Lv;
        k3d::String Line;
    };

    // -- ILogger
    virtual void Log(k3d::ELogLevel const & logLevel, const char * tag, const char *message) override;
    // -- ILogger

    bool        IsEmpty();
    LogEntry    PeekLine();

private:
    k3d::LockFreeQueue<LogEntry> m_Lines;
};

class LogHighLighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LogHighLighter(QTextEdit* parent);
    ~LogHighLighter();
    void highlightBlock(const QString &text) override;       
    void setFilterRegExp(const QRegExp* filterRegExp);
private:const 
    QRegExp*        m_FilterRegExp;                   ///< Current filter regexp
    QTextCharFormat m_FilterMatchFormat;
};

class LogViewer;

class LogView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int RefreshInterval READ getRefreshInterval WRITE setRefreshInterval)
    Q_PROPERTY(int maxBufferedLines READ maxBufferedLines WRITE setMaxBufferedLines)
public:
    LogView(QWidget* parent);
    ~LogView() override;

    int     getRefreshInterval() const;
    void    setRefreshInterval(int interval);

    int     maxBufferedLines() const;
    void    setMaxBufferedLines(int maxLines);

    Logger* GetLogReceiver() const;

public slots:
    void    onDeviceSelected(k3d::mobi::IDevice* device);
private slots:
    void    onRefreshLog();
    void    onLogPaused(bool bPaused);
    void    filterLogMessages(const QString&);

protected:
    void    CreateUI();

private:
    int                 m_RefreshInterval;
    int                 m_MaxBufferLines;

    QTimer*             m_RefreshTimer;
    QToolButton*        m_TbStart;
    QTextEdit*          m_Display;
    LogViewer*          m_Viewer;
    QLineEdit*          m_Input;
    Logger*             m_Logger;
    QRegExp*            m_FilterRegExp;
    LogHighLighter*     m_Highlighter;
};