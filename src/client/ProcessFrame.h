#ifndef PROCESSFRAME_H
#define PROCESSFRAME_H

#include <QWidget>
#include <QFrame>
#include <sstream>

QT_BEGIN_NAMESPACE
namespace Ui { class ProcessFrame; }
QT_END_NAMESPACE

class ProcessFrame : public QFrame
{
    Q_OBJECT

private:
    Ui::ProcessFrame *ui;

    int pid;
    long long cpuTime = 0;
    long long procTime = 0;

public:
    explicit ProcessFrame(QWidget *parent, int, std::string, long long, long long, float);

    void updateCpuUsage(long long, long long);
    void updateRamUsage (float);
    float getCpuUsage ();
    float getRamUsage ();
    int getPid ();
    QString getName();
    QString getCpuLabel();
    QString getRamLabel();

    ~ProcessFrame();
};

#endif // PROCESSFRAME_H