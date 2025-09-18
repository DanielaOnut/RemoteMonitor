#ifndef KILLPROCESS_H
#define KILLPROCESS_H

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <sstream>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class KillProcess; }
QT_END_NAMESPACE

class KillProcess : public QDialog
{
    Q_OBJECT

private:
    Ui::KillProcess *ui;

    int pid;
public:
    explicit KillProcess(QWidget *parent, QString, int);

    ~KillProcess();

signals:
    void requestProcessKill (int);
};

#endif // KILLPROCESS_H