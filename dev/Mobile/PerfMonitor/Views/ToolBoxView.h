#pragma once

#include <QWidget>
#include <QLineEdit>
#include <MobileDeviceBridge.h>

class ToolBoxView : public QWidget
{
    Q_OBJECT
public:

    ToolBoxView(QWidget* parent = nullptr);
    ~ToolBoxView() override;

private slots:
    void SetCurrentDevice(k3d::mobi::IDevice* pDevice);
    void showApkInstallDirectory();
    void pullRemoteToLocal();
    void pushLocalToRemote();
private:
    void CreateUI();

private:
    k3d::mobi::IDevice* m_Device;
    QLineEdit* m_RemoteDir;
    QLineEdit* m_LocalDir;
};