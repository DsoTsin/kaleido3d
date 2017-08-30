#include "LogView.h"

#include <QListView>
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <qplaintextedit.h>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTimer>

#include "Base/LogViewer.h"

static QIcon GetLogStartIcon(bool bStart = true)
{
    static QIcon start(":/Images/start.png");
    static QIcon pause(":/Images/pause.png");
    return bStart ? start : pause;
}

LogHighLighter::LogHighLighter(QTextEdit* parent)
    : QSyntaxHighlighter(parent)
    , m_FilterRegExp(nullptr)
{
    QBrush fgBrush = QBrush(Qt::white);
    QBrush bgBrush = QBrush(QColor(56, 216, 120));

    m_FilterMatchFormat.setBackground(bgBrush);
    m_FilterMatchFormat.setForeground(fgBrush);

}

LogHighLighter::~LogHighLighter()
{
    setFilterRegExp(nullptr);
}

void LogHighLighter::highlightBlock(const QString & text)
{
    if (!m_FilterRegExp || text.length() == 0) return;
    int pos = 0;
    while ((pos = m_FilterRegExp->indexIn(text, pos)) != -1) 
    {
        int matchedLength = m_FilterRegExp->matchedLength();
        setFormat(pos, matchedLength, m_FilterMatchFormat);
        pos += matchedLength;
    }
}

void LogHighLighter::setFilterRegExp(const QRegExp * filterRegExp)
{
    if (m_FilterRegExp) 
    {
        delete m_FilterRegExp;
    }
    m_FilterRegExp = filterRegExp;
}

Logger::Logger()
{
}

Logger::~Logger()
{
}

void Logger::Log(k3d::ELogLevel const &level, const char * tag, const char *message)
{
    m_Lines.Enqueue({level, message});
}

bool Logger::IsEmpty()
{
    return m_Lines.IsEmpty();
}

Logger::LogEntry Logger::PeekLine()
{
    Logger::LogEntry line;
    m_Lines.Dequeue(line);
    return line;
}

LogView::LogView(QWidget* parent) 
    : QWidget(parent)
    , m_RefreshInterval(1000)
    , m_MaxBufferLines(100)
    , m_FilterRegExp(nullptr)
    , m_Highlighter(nullptr)
{
    CreateUI();

    m_RefreshTimer = new QTimer(this);
    m_RefreshTimer->setInterval(m_RefreshInterval);
    connect(m_RefreshTimer, SIGNAL(timeout()), SLOT(onRefreshLog()));
    m_RefreshTimer->start();

    m_Logger = new Logger();
}

void LogView::CreateUI()
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);

    QHBoxLayout* filterLayout = new QHBoxLayout;
    m_TbStart = new QToolButton(this);
    connect(m_TbStart, SIGNAL(toggled(bool)), SLOT(onLogPaused(bool)));
    m_TbStart->setIcon(GetLogStartIcon(false));
    filterLayout->addWidget(m_TbStart);
    QComboBox* filterLevCb = new QComboBox(this);
    filterLevCb->addItem(tr("Verbose"));
    filterLevCb->addItem(tr("Debug"));
    filterLevCb->addItem(tr("Info"));
    filterLevCb->addItem(tr("Error"));
    filterLayout->addWidget(filterLevCb);
    QLabel* filterLabel = new QLabel(tr("Filter"), this);
    filterLayout->addWidget(filterLabel);
    m_Input = new QLineEdit(this);
    filterLayout->addWidget(m_Input);
    rootLayout->addLayout(filterLayout);

    m_Viewer = new LogViewer(this);

    rootLayout->addWidget(m_Viewer);   
    setLayout(rootLayout);
}

LogView::~LogView()
{
    if (m_Logger)
    {
        delete m_Logger;
    }
}

void LogView::onLogPaused(bool bPaused)
{
    m_TbStart->setIcon(GetLogStartIcon(!bPaused));
}

void LogView::filterLogMessages(const QString & regText)
{
    if (regText.length() == 0)
    {
        m_FilterRegExp = nullptr;
    }
    else
    {
        m_FilterRegExp = new QRegExp(regText, Qt::CaseInsensitive, QRegExp::FixedString);
    }
    m_Highlighter->setFilterRegExp(m_FilterRegExp);
}

int LogView::getRefreshInterval() const
{
    return m_RefreshInterval;
}

void LogView::setRefreshInterval(int interval)
{
    m_RefreshInterval = interval;
    m_RefreshTimer->setInterval(m_RefreshInterval);
}

int LogView::maxBufferedLines() const
{
    return m_MaxBufferLines;
}

void LogView::setMaxBufferedLines(int maxLines)
{
    m_MaxBufferLines = maxLines;
}

Logger* 
LogView::GetLogReceiver() const
{
    return m_Logger;
}

void LogView::onDeviceSelected(k3d::mobi::IDevice* device)
{

}

void LogView::onRefreshLog()
{
    QString textLog;
    while (!m_Logger->IsEmpty())
    {
        auto line = m_Logger->PeekLine();
        textLog += *(line.Line + "\n");
        QTextCharFormat fmt;
        QtMsgType type = QtMsgType::QtInfoMsg;
        switch (line.Lv)
        {
        case k3d::ELogLevel::Info:
            break;
        case k3d::ELogLevel::Debug:
            type = QtMsgType::QtDebugMsg;
            break;
        case k3d::ELogLevel::Warn:
            type = QtMsgType::QtWarningMsg;
            break;
        case k3d::ELogLevel::Error:
            type = QtMsgType::QtFatalMsg;
            break;
        }
        m_Viewer->appendLog(type, *(line.Line));
    }
    m_Viewer->notifyLogChanged();
}
