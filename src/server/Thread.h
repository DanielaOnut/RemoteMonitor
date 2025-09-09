#ifndef THREAD_H
#define THREAD_H

#include <iostream>
#include <fstream>
#include <QRunnable>
#include <QObject>
#include <QSslSocket>
#include <jwt-cpp/jwt.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include "Server.h"

typedef std::pair <std::string, const char*> pair;

class Thread : public QRunnable
{

private:
    Server * server;
    QSslSocket * socket;
    QByteArray requestData;

public:
    Thread (Server *, QByteArray &, QSslSocket *);

    pair getJwt (const QByteArray &);
    pair updateJwt (const QByteArray &);
    pair verifyToken (const QByteArray &);
    pair updateStats (const QByteArray &);
    std::string getCpuUsage ();
    std::string getRamUsage ();
    std::string getDiskUsage ();
    pair getSysInfo (const QByteArray &);
   
    void run() override;

    ~Thread();
};

#endif // THREAD_H