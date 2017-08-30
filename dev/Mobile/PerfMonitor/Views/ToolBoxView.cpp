#include "ToolBoxView.h"
#include <MobileDeviceBridge.h>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QListWidget>

ToolBoxView::ToolBoxView(QWidget* parent)
    : QWidget(parent)
    , m_Device(nullptr)
{
    CreateUI();
}

ToolBoxView::~ToolBoxView()
{
}

void ToolBoxView::CreateUI()
{
    QVBoxLayout* rootLayout = new QVBoxLayout;
    QHBoxLayout* toolLine = new QHBoxLayout;
    QHBoxLayout* inputLine = new QHBoxLayout;

    QPushButton* btnInstall = new QPushButton(tr("Install App"), this);
    connect(btnInstall, SIGNAL(clicked()), SLOT(showApkInstallDirectory()));

    QPushButton* btnPull = new QPushButton(tr("Pull"), this);
    connect(btnPull, SIGNAL(clicked()), SLOT(pullRemoteToLocal()));

    QPushButton* btnPush = new QPushButton(tr("Push"), this);
    connect(btnPush, SIGNAL(clicked()), SLOT(pushLocalToRemote()));

    toolLine->addWidget(btnInstall);
    toolLine->addWidget(btnPull);
    toolLine->addWidget(btnPush);

    QLabel* localLb = new QLabel(tr("Local Dir:"), this);
    QLabel* remoteLb = new QLabel(tr("Remote Dir:"), this);
    m_LocalDir = new QLineEdit(this);
    m_RemoteDir = new QLineEdit(this);
    inputLine->addWidget(localLb);
    inputLine->addWidget(m_LocalDir);
    inputLine->addWidget(remoteLb);
    inputLine->addWidget(m_RemoteDir);

    rootLayout->addLayout(toolLine);
    rootLayout->addLayout(inputLine);
    setLayout(rootLayout);
}

void ToolBoxView::SetCurrentDevice(k3d::mobi::IDevice * pDevice)
{
    m_Device = pDevice;
}

void ToolBoxView::showApkInstallDirectory()
{
    if (!m_Device)
        return;
    auto plat = m_Device->GetPlatform();
    QFileDialog dialog(this);
    dialog.setWindowTitle(plat == k3d::mobi::EPlatform::Android ? 
        tr("Select App (Android: apk) to Install") : 
        tr("Select App (iOS: ipa) to Install")
    );
    dialog.setDirectory(".");
    dialog.setNameFilter(plat == k3d::mobi::EPlatform::Android ?
        tr("Android App (*.apk)") :
        tr("iOS App (*.ipa)"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }
    if (fileNames.size() == 1)
    {
        QString appPath = fileNames[0];
        m_Device->InstallApp(appPath.toUtf8().data());
        // add recent installed app
    }
}

void ToolBoxView::pullRemoteToLocal()
{
    QString localDir = m_LocalDir->text();
    QString remoteDir = m_RemoteDir->text();
    m_Device->Download(localDir.toUtf8().data(), remoteDir.toUtf8().data());
}

void ToolBoxView::pushLocalToRemote()
{
    QString localDir = m_LocalDir->text();
    QString remoteDir = m_RemoteDir->text();
    m_Device->Upload(localDir.toUtf8().data(), remoteDir.toUtf8().data());
}
