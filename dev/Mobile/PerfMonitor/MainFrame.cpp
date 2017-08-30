#include "MainFrame.h"
#include <MobileDeviceBridge.h>

#include <QMenuBar>

#include "Views/DeviceView.h"
#include "Views/GPUView.h"
#include "Views/LogView.h"
#include "Views/ApplicationView.h"
#include "Views/ToolBoxView.h"
#include "Views/Base/TimeLineView.h"

MainFrame::MainFrame(QWidget *parent) : QMainWindow(parent)
{
    setObjectName(tr("MainFrame"));
    resize(1000, 618);
    CreateMenuBar();
    CreateSubDocks();
    auto TLV = new TimeLineView(this);
    TLV->Start();
    setCentralWidget(TLV);;
}

MainFrame::~MainFrame()
{
}

LogView * MainFrame::PeekLogView()
{
    return m_LogView;
}

void MainFrame::AddDock(QWidget * w, QString const & Name, Qt::DockWidgetArea area)
{
    QDockWidget* DockView = new QDockWidget(Name, this);
    DockView->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
    DockView->setWidget(w);
    addDockWidget(area, DockView);
}

void MainFrame::CreateMenuBar()
{
    m_Menu = new QMenuBar(this);
    m_Menu->setObjectName(QStringLiteral("menuBar"));
    m_Menu->setGeometry(QRect(0, 0, 909, 22)); 
    auto menuFiles = new QMenu(tr("Files"), m_Menu);
    menuFiles->setObjectName(QStringLiteral("menuFiles"));
    m_Menu->addAction(menuFiles->menuAction());
    setMenuBar(m_Menu);
}

void MainFrame::CreateSubDocks()
{
    auto& Bridge = k3d::GetMobileDeviceBridge();
    m_DeviceView = new DeviceView(&Bridge, this);
    m_DeviceView->setObjectName("deviceView");
    AddDock(m_DeviceView, tr("Devices"), Qt::DockWidgetArea::RightDockWidgetArea);

    m_LogView = new LogView(this);
    m_LogView->setObjectName("logView");
    AddDock(m_LogView, tr("Log"), Qt::DockWidgetArea::BottomDockWidgetArea);

    ApplicationView* appView = new ApplicationView(this);
    appView->setObjectName("applicationView");
    AddDock(appView, tr("Applications"), Qt::DockWidgetArea::RightDockWidgetArea);

    m_GpuView = new GPUView(this);
    m_GpuView->setObjectName("gpuView");
    AddDock(m_GpuView, tr("GPU View"), Qt::DockWidgetArea::LeftDockWidgetArea);

    m_ToolView = new ToolBoxView(this);
    m_ToolView->setObjectName("toolBoxView");
    AddDock(m_ToolView, tr("Tool View"), Qt::DockWidgetArea::LeftDockWidgetArea);

    connect(m_DeviceView, SIGNAL(onDeviceSelected(k3d::mobi::IDevice*)), appView, SLOT(onSetCurrentDevice(k3d::mobi::IDevice*)));
    connect(m_DeviceView, SIGNAL(onDeviceSelected(k3d::mobi::IDevice*)), m_LogView, SLOT(onDeviceSelected(k3d::mobi::IDevice*)));
    connect(m_DeviceView, SIGNAL(onDeviceSelected(k3d::mobi::IDevice*)), m_ToolView, SLOT(SetCurrentDevice(k3d::mobi::IDevice*)));
}
