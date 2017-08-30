#pragma once

#include <QMainWindow>
#include <QDockWidget>

class DeviceView;
class LogView;
class ApplicationView;
class GPUView;
class ToolBoxView;
class TimeLineView;


class QMenuBar;

class MainFrame : public QMainWindow
{
    Q_OBJECT
public:
    MainFrame(QWidget *parent = Q_NULLPTR);
    ~MainFrame() override;

    LogView* PeekLogView();

private:
    void CreateMenuBar();
    void CreateSubDocks();
    void AddDock(QWidget* w, QString const& Name, Qt::DockWidgetArea area);
    
    DeviceView*         m_DeviceView;
    ApplicationView*    m_AppsView;
    GPUView*            m_GpuView;
    LogView*            m_LogView;
    TimeLineView*       m_TimeLineView;
    ToolBoxView*        m_ToolView;
    QMenuBar*           m_Menu;

};