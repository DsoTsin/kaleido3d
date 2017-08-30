#include "MainFrame.h"
#include <QApplication>

#include <MobileDeviceBridge.h>

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); 
    qRegisterMetaType<k3d::mobi::Processes>("k3d::mobi::Processes");
    MainFrame mf;
    mf.show();
    return a.exec();
}
