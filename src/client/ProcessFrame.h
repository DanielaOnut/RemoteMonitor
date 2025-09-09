#ifndef PROCESSFRAME_H
#define PROCESSFRAME_H

#include <QWidget>
#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class ProcessFrame; }
QT_END_NAMESPACE

class ProcessFrame : public QFrame
{
    Q_OBJECT

private:
    Ui::ProcessFrame *ui;

public:
    explicit ProcessFrame(QWidget *parent);

};

#endif // PROCESSFRAME_H