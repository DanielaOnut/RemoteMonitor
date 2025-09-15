#ifndef MACHINEFRAME_H
#define MACHINEFRAME_H

#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <ctime>
#include <vector>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <iomanip> 
#include "ProcessFrame.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MachineFrame; }
QT_END_NAMESPACE

class MachineFrame : public QFrame
{
    Q_OBJECT

private:
    Ui::MachineFrame *ui;

    QNetworkAccessManager * manager;
    QString * jwtToken = nullptr;
    QTimer * overviewTimer;
    bool machineIsOn = true;

    long long totalCpu = 0;
    long long workCpu = 0;

    std::vector<ProcessFrame *> processesList;
    ProcessFrame * procExists (int);
    QByteArray getJsonProcList ();

public:
    explicit MachineFrame(QWidget *parent, const QString & machineName, const QString & ipaddress);

    void connectComponents ();
    void setJwtToken (const QByteArray &);
    void sendUpdateRequest (const QString);
    void getNewJwtToken (QNetworkReply *, QNetworkRequest);
    void setUpdateLabelTime (); 
    void getOSInfo ();
    void setCpuUsage (const QByteArray &);
    void setRamUsage (const QByteArray &);
    void setDiskUsage (const QByteArray &);
    void createProcList(const QByteArrayList &);
    void handleErrOccurred(const QNetworkReply *);
    bool isMachineOn () { return this->machineIsOn; };
    QString getMachineName ();
    
    ~MachineFrame();

signals:
    void connRefused();
    void connReestablished();
    void machineDeleted(MachineFrame *);
};

#endif // MACHINEFRAME_H