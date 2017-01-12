#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rendererwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralWidget = new RendererWidget(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
