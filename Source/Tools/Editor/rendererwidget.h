#ifndef RENDERERWIDGET_H
#define RENDERERWIDGET_H

#include <QWidget>
#include <RHI/IRHI.h>

using namespace rhi;

class RendererWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RendererWidget(QWidget *parent = 0);

signals:

public slots:

private:
    IDevice * device;
    IRenderViewport * viewport;
};

#endif // RENDERERWIDGET_H
