#pragma once
#include <QWidget>

class ProcessView : public QWidget
{
    Q_OBJECT
public:
    ProcessView(QWidget* parent);
    ~ProcessView() override;
};
