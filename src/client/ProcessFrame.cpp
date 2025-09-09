#include "ProcessFrame.h"
#include "ui_processframe.h"

ProcessFrame::ProcessFrame(QWidget *parent)
    : QFrame (parent)
    , ui (new Ui::ProcessFrame) 
{
    ui->setupUi (this);
}