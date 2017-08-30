#pragma once

#include <QListWidget>
#include <MobileDeviceBridge.h>

namespace k3d {
    class MobileDeviceBridge;
}
class QTimer;

class DeviceItemWidget : public QWidget
{
    Q_OBJECT
public:
    DeviceItemWidget(k3d::mobi::IDevice* InDevice, QWidget* parent = 0);
    ~DeviceItemWidget() override;

    k3d::mobi::IDevice* GetDevice() const { return m_Device; }
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void OpenLogView();

private:
    QAction*            m_OpenLogView;
    k3d::mobi::IDevice* m_Device;
};

class DeviceView : public QListWidget
{
    Q_OBJECT
public:
    DeviceView(k3d::MobileDeviceBridge* InMDB, QWidget* parent);
    ~DeviceView() override;

signals:
    void onDeviceSelected(k3d::mobi::IDevice*);
    void onApplicationListed(QStringList const& apps);

private slots:
    void onTimeoutRefresh();
    void onItemPressed(QListWidgetItem*);

private:
    k3d::MobileDeviceBridge*            m_Bridge;
    QTimer*                             m_Timer;
    QMap<QString, DeviceItemWidget*>    m_DeviceMap;
    QMap<QString, QListWidgetItem*>     m_ItemMap;
};