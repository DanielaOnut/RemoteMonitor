#pragma once 
#ifndef PROCESSFRAME_H
#define PROCESSFRAME_H

#include <QWidget>
#include <QFrame>
#include <sstream>
#include <iostream>

class MachineFrame;

QT_BEGIN_NAMESPACE
namespace Ui { class ProcessFrame; }
QT_END_NAMESPACE

class ProcessFrame : public QFrame
{
    Q_OBJECT

private:
    Ui::ProcessFrame *ui;
    MachineFrame *parentFrame;

    int pid;
    long long cpuTime = 0;
    long long procTime = 0;

public:
    explicit ProcessFrame(MachineFrame *, int, std::string, long long, long long, float);

    void updateCpuUsage(long long, long long);
    void updateRamUsage (float);
    float getCpuUsage ();
    float getRamUsage ();
    int getPid ();
    QString getName();
    QString getCpuLabel();
    QString getRamLabel();

    ~ProcessFrame();

signals:
    void sendKillProcReq (int);
};

#endif // PROCESSFRAME_H