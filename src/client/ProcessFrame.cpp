#include "ProcessFrame.h"
#include "ui_processframe.h"

ProcessFrame::ProcessFrame(QWidget *parent, int pid, std::string procName, long long procTime, long long cpuTime, float ramUsage)
    : QFrame(parent), ui(new Ui::ProcessFrame)
{
    ui->setupUi (this);

    this->ui->pidLabel->setText("PID: " + QString::number(pid));
    this->ui->procNameLabel->setText(procName.c_str());
    this->ui->ramLabel->setText(QString::number(ramUsage, 'f', 1).append(" MB"));

    this->cpuTime = cpuTime;
    this->procTime = procTime;
    float usage = (float)this->procTime / this->cpuTime * 100;
    this->ui->cpuLabel->setText(QString::number(usage, 'f', 1).append(" % CPU"));
}

void ProcessFrame::updateCpuUsage(long long newProcTime, long long newCpuTime) {
    if (newProcTime <= this->procTime || newCpuTime <= this->procTime)
        return;

    long long procDelta = newProcTime - this->procTime; 
    long long cpuDelta = newCpuTime - this->cpuTime; 
    float usage = (float) procDelta / cpuDelta * 100;
    this->ui->cpuLabel->setText(QString::number(usage, 'f', 1).append(" % CPU"));
}

void ProcessFrame::updateRamUsage(float ramUsage) {
    this->ui->ramLabel->setText(QString::number(ramUsage, 'f', 1).append(" MB"));
}

ProcessFrame::~ProcessFrame() {
    delete this->ui;
}