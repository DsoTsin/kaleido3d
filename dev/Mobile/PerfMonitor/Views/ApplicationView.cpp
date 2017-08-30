#include "ApplicationView.h"
#include <QBoxLayout>
#include <QLabel>

ProcessListQueryThread::ProcessListQueryThread(QObject* parent)
    : QThread(parent)
    , m_Device(nullptr)
{
}

void ProcessListQueryThread::SetDevice(k3d::mobi::IDevice * pDevice)
{
    m_Device = pDevice;
}

void ProcessListQueryThread::run()
{
    if (m_Device)
    {
        k3d::mobi::Processes procs = m_Device->ListAllRunningProcess();
        emit onQueryFinish(procs);
    }
}

ApplicationView::ApplicationView(QWidget* parent)
    : QWidget(parent)
    , m_CurDevice(nullptr)
    , m_QueryThread(nullptr)
{
    CreateUI();

    m_QueryThread = new ProcessListQueryThread(this);
    connect(m_QueryThread, SIGNAL(onQueryFinish(k3d::mobi::Processes)), SLOT(onReceiveApplist(k3d::mobi::Processes)));
}

ApplicationView::~ApplicationView()
{
    if (m_QueryThread)
    {
        m_QueryThread->terminate();
        delete m_QueryThread;
    }
}

void ApplicationView::SetCurrentDevice(k3d::mobi::IDevice* pDevice)
{
    m_CurDevice = pDevice;
}

void ApplicationView::onSetCurrentDevice(k3d::mobi::IDevice* pDevice)
{
    if (pDevice != m_CurDevice)
    {
        SetCurrentDevice(pDevice);
        m_QueryThread->SetDevice(pDevice);
        m_QueryThread->start();
    }
}

void ApplicationView::CreateUI()
{

}

void ApplicationView::onReceiveApplist(k3d::mobi::Processes apps)
{

}