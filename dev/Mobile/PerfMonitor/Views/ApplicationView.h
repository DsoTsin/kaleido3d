#pragma once

#include <QWidget>
#include <QThread>
#include <MobileDeviceBridge.h>

namespace k3d
{
    namespace mobi
    {
        class IDevice;
    }
}

#include <QMetaType>
Q_DECLARE_METATYPE(k3d::mobi::Processes);

class ProcessListQueryThread : public QThread
{
    Q_OBJECT
public:
    ProcessListQueryThread(QObject* parent = 0);
    void SetDevice(k3d::mobi::IDevice* pDevice);
    virtual void run() override;
signals:
    void onQueryFinish(k3d::mobi::Processes procs);

private:
    k3d::mobi::IDevice* m_Device;
};

class ApplicationView : public QWidget
{
    Q_OBJECT
public:
    ApplicationView(QWidget* parent);
    ~ApplicationView() override;
    void SetCurrentDevice(k3d::mobi::IDevice* pDevice);
public slots:
    void onSetCurrentDevice(k3d::mobi::IDevice* pDevice);
private slots:
    void onReceiveApplist(k3d::mobi::Processes apps);
private:
    void CreateUI();
private:
    k3d::mobi::IDevice*     m_CurDevice;
    ProcessListQueryThread* m_QueryThread;
};

