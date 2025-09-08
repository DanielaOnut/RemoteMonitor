#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <iostream>
#include <QSslSocket>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QScrollBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "MachineFrame.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

private:
    Ui::Client *ui;

    QMap <QString, MachineFrame *> machinesMap;
    QTimer * timer;
    QNetworkAccessManager * manager;

    int totalMachines = 0;
    int onlineMachines = 0;
    int offlineMachines = 0; 

public:
    explicit Client(QWidget *parent = nullptr);

    void connectComponents ();

    ~Client();

public slots:
    void addConnection (QString, QString);
    void machineWentOffline ();
    void machineIsBackOnline ();

signals:
    void closeDialog ();
    void showConnFailedLabel();
    void showConnSuccessLabel();
    void showConnectingLabel();
    void showNameTakenLabel();
};

#endif // CLIENT_H