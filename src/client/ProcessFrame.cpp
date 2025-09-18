#include "ProcessFrame.h"
#include "ui_processframe.h"
#include "KillProcess.h"
#include "MachineFrame.h"

ProcessFrame::ProcessFrame(MachineFrame *parent, int pid, std::string procName, long long procTime, long long cpuTime, float ramUsage)
    : QFrame(parent), parentFrame(parent), ui(new Ui::ProcessFrame)
{
    ui->setupUi (this);

    this->pid = pid;
    this->ui->pidLabel->setText("PID: " + QString::number(pid));
    this->ui->procNameLabel->setText(procName.c_str());
    this->ui->ramLabel->setText(QString::number(ramUsage, 'f', 1).append(" MB"));

    this->cpuTime = cpuTime;
    this->procTime = procTime;
    float usage = (float)this->procTime / this->cpuTime * 100;
    this->ui->cpuLabel->setText(QString::number(usage, 'f', 1).append(" % CPU"));

    connect (this->ui->killBtn, &QPushButton::clicked, [this] {
        KillProcess * dialog = new KillProcess(this, this->getName(), this->pid);
       
        connect (dialog, &KillProcess::requestProcessKill, [this] (int pid) {
            emit this->sendKillProcReq (pid);
        });

        connect(parentFrame, &MachineFrame::closeKillProcDialog,
                [dialog] { 
                    if (dialog != nullptr) 
                        dialog->close(); 
                });

        dialog->exec();
    });
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
    this->ui->ramLabel->setText(QString::number(ramUsage, 'f', 1).append(" MB RAM"));
}

float ProcessFrame::getCpuUsage() {
    std::string usageLabel = this->ui->cpuLabel->text().toStdString();
    float value;
    std::istringstream iss(usageLabel);
    iss >> value;
    return value;
}

float ProcessFrame::getRamUsage() {
    std::string usageLabel = this->ui->ramLabel->text().toStdString();
    float value;
    std::istringstream iss(usageLabel);
    iss >> value;
    return value;
}

int ProcessFrame::getPid() {
    return this->pid;
}

QString ProcessFrame::getName() {
    return this->ui->procNameLabel->text();
}

QString ProcessFrame::getCpuLabel() {
    return this->ui->cpuLabel->text();
}

QString ProcessFrame::getRamLabel() {
    return this->ui->ramLabel->text();
}

ProcessFrame::~ProcessFrame() {
    delete this->ui;
}