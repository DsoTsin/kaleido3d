#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include "rendererwidget.h"
#include <QToolbar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    auto renderWidget = new RendererWidget(this);
    renderWidget->init();
    setCentralWidget(renderWidget);
    QToolBar* header = addToolBar("Header");
    QAction* action = header->addAction("Add");
    
    QMenuBar* menu = menuBar();
    auto file = menu->addMenu("File");
    auto exitAction = file->addAction("Exit");
    QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(onExit()));

    auto help = menu->addMenu("Help");
    auto about = help->addAction("About");

    //QMenu * menu = test->addAction("File");
}

MainWindow::~MainWindow()
{
}

void MainWindow::onExit()
{
  exit(0);
}