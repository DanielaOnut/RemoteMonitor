#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <exception>
#include <map>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QFile>
#include <QSslKey>
#include <QUuid>
#include <QString>
#include <QThreadPool>
#include <QReadWriteLock>
#include <jwt-cpp/jwt.h>

class Server : public QTcpServer 
{
    Q_OBJECT

private:
    QSslConfiguration sslConfig;
    std::map <QString, QString> clientsMap; 
    mutable QReadWriteLock clientsMapLock;

public:
    explicit Server(QObject *parent = nullptr);

    void updateClientsMap (QString, QString);
    bool clientExists (const std::string);
    QString & getDeviceNameByClient (const char *);
    QByteArray getHttpResponse (const char *, const char *);
    void sendResponse (QSslSocket *, const QByteArray &);

    ~Server();

public slots:
    void incomingConnection(qintptr);
};

#endif // SERVER_H