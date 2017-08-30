#include "DeviceView.h"
#include "LogView.h"
#include "../MainFrame.h"
#include <QList>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>

DeviceItemWidget::DeviceItemWidget(k3d::mobi::IDevice* InDevice, QWidget* parent) 
    : QWidget(parent)
    , m_Device(InDevice)
{
    m_OpenLogView = new QAction(tr("Open Log"), this);
    connect(m_OpenLogView, SIGNAL(triggered(bool)), SLOT(OpenLogView()));
}

DeviceItemWidget::~DeviceItemWidget()
{
    if (m_OpenLogView)
    {
        delete m_OpenLogView;
    }
}

void DeviceItemWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu menu(this);
    menu.addAction(m_OpenLogView);
    menu.exec(event->globalPos());
}

void DeviceItemWidget::OpenLogView()
{
    QObject* _parent = parent();
    while (_parent->objectName() != tr("MainFrame"))
    {
        _parent = _parent->parent();
    }
    MainFrame* Main = static_cast<MainFrame*>(_parent);
    m_Device->InstallLogger(Main->PeekLogView()->GetLogReceiver());
    m_Device->StartLogService();
}

DeviceView::DeviceView(k3d::MobileDeviceBridge* InMDB, QWidget* parent) 
    : QListWidget(parent)
    , m_Bridge(InMDB)
{
    m_Timer = new QTimer(this);
    m_Timer->setInterval(2500);
    connect(m_Timer, SIGNAL(timeout()), SLOT(onTimeoutRefresh()));
    m_Timer->start();

    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), SLOT(onItemPressed(QListWidgetItem*)));
}

DeviceView::~DeviceView()
{
    if (m_Timer)
    {
        delete m_Timer;
        m_Timer = nullptr;
    }
}

void DeviceView::onItemPressed(QListWidgetItem* item)
{
    DeviceItemWidget* iw = static_cast<DeviceItemWidget*>(itemWidget(item));
    if (iw)
    {
        emit onDeviceSelected(iw->GetDevice());
    }
}

void DeviceView::onTimeoutRefresh()
{
    static QIcon Icon_Android(":/Images/device_android.png");
    static QIcon Icon_IOS(":/Images/device_ios.png");
    k3d::mobi::DeviceList list;
    m_Bridge->QueryDevices(list);
    if (list.Count() > 0)
    {
        QList<QString> DeviceSerials;
        for (k3d::mobi::IDevice* Device : list)
        {
            QString Serial = *Device->GetSerialId();
            DeviceSerials << Serial;
            if (!m_DeviceMap.contains(Serial))
            {
                QListWidgetItem* Item = new QListWidgetItem(this);
                Item->setText(*Device->GetDesc());
                Item->setIcon(Device->GetPlatform() == k3d::mobi::EPlatform::Android? Icon_Android: Icon_IOS);
                Item->setSizeHint(QSize(50, 20));
                addItem(Item);
                m_ItemMap[Serial] = Item;
                DeviceItemWidget* iWidget = new DeviceItemWidget(Device, this);
                setItemWidget(Item, iWidget);
                m_DeviceMap[Serial] = iWidget;
            }
        }
        for (auto& Key : m_DeviceMap.keys())
        {
            if (!DeviceSerials.contains(Key))
            {
                m_DeviceMap.remove(Key);
                
                removeItemWidget(m_ItemMap[Key]);
                m_ItemMap.remove(Key);
            }
        }
    }
}