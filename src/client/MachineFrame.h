#ifndef MACHINEFRAME_H
#define MACHINEFRAME_H

#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <ctime>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <iomanip> 

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
    QTimer * updateTimer;
    bool machineIsOn = true;

    long long totalCpu = 0;
    long long workCpu = 0;
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
    void handleErrOccurred (const QNetworkReply *);
    bool isMachineOn () { return this->machineIsOn; };

    ~MachineFrame();

signals:
    void connRefused();
    void connReestablished();
};

#endif // MACHINEFRAME_H