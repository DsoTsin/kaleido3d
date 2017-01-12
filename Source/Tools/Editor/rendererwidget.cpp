#include "rendererwidget.h"

RendererWidget::RendererWidget(QWidget *parent)
    : QWidget(parent)
    , device(nullptr)
    , viewport(nullptr)
{
    GfxSetting setting;
    viewport = device->NewRenderViewport((void*)winId(), setting);
}
