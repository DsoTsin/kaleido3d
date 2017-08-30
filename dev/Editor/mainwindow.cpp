#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include "rendererwidget.h"
#include <QToolbar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDockWidget>
#include <QListWidget>
#include <material/EditorView.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    auto renderWidget = new RendererWidget(this);
    renderWidget->init();
    setCentralWidget(renderWidget);

    createActions();
    createDocks();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
  QToolBar* header = addToolBar(tr("Header"));
  QAction* action = header->addAction(tr("Add"));

  // File menu
  QMenuBar* menu = menuBar();
  auto file = menu->addMenu(tr("File"));
  auto openAction = file->addAction(tr("Open Project"));
  auto saveAction = file->addAction(tr("Save"));
  auto openRecentAction = file->addAction(tr("Recent Projects"));
  auto exitAction = file->addAction(tr("Exit"));
  QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(onExit()));

  // Window menu
  auto window = menu->addMenu(tr("Window"));
  auto materialEditor = window->addAction(tr("Material Editor"));

  // Help
  auto help = menu->addMenu("Help");
  auto about = help->addAction("About");

}

void MainWindow::createDocks()
{
  auto dock = new QDockWidget(tr("Material Editor"), this);
  dock->setWidget(new FlowView(new FlowScene(registerDataModels())));
  addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::onExit()
{
  exit(0);
}