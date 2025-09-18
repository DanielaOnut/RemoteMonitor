#include "MachineFrame.h"
#include "ui_MachineFrame.h"

MachineFrame::MachineFrame(QWidget *parent, const QString & machineName, const QString & ipaddress)
    : QFrame(parent)
    , ui(new Ui::MachineFrame) 
{
    ui->setupUi(this);

    this->manager = new QNetworkAccessManager(this);
    this->overviewTimer = new QTimer (this);
    this->procsOrderBtn = this->ui->cpuBtn;

    this->overviewTimer->setInterval(2000);
    this->overviewTimer->start();
    this->ui->machineName->setText(machineName);
    this->ui->ipaddressLabel->setText(ipaddress);

    this->setUpdateLabelTime();
    this->connectComponents();
}

void MachineFrame::connectComponents() {
    connect (this->ui->overviewBtn, &QPushButton::clicked, [this] {
        this->ui->overviewBtn->setStyleSheet("background-color: #ffffff; border-radius: 10px;");
        this->ui->processesBtn->setStyleSheet("background-color: #ececf0; border-radius: 10px;");

        this->ui->stackedWidget->setCurrentIndex(0);
    });

    connect (this->ui->processesBtn, &QPushButton::clicked, [this] {
        this->ui->processesBtn->setStyleSheet("background-color: #ffffff; border-radius: 10px;");
        this->ui->overviewBtn->setStyleSheet("background-color: #ececf0; border-radius: 10px;");

        this->ui->stackedWidget->setCurrentIndex(1);
    });

    connect (this->ui->refreshBtn, &QToolButton::clicked, [this] { 
        this->sendUpdateRequest("updateall"); 
    });

    connect (this->ui->deleteBtn, &QToolButton::clicked, [this] {
        this->overviewTimer->stop();
        emit this->machineDeleted(this);
    });

    connect (this->overviewTimer, &QTimer::timeout, [this] {
        this->sendUpdateRequest("updateall"); 
    });

    connect(this->ui->cpuBtn, &QPushButton::clicked, [this] { 
        this->setBtnsStyles(this->ui->cpuBtn);
        this->sortProcsByBtn();
    });

    connect(this->ui->memoryBtn, &QPushButton::clicked, [this] { 
        this->setBtnsStyles(this->ui->memoryBtn);
        this->sortProcsByBtn();
    });

    connect(this->ui->nameBtn, &QPushButton::clicked, [this] { 
        this->setBtnsStyles(this->ui->nameBtn);
        this->sortProcsByBtn();
    });
}

void MachineFrame::setJwtToken(const QByteArray & token) {
    this->jwtToken = new QString(token);
    this->getOSInfo();
    this->sendUpdateRequest("updateall");
}

void MachineFrame::sendUpdateRequest (const QString route) {
    if (this->jwtToken == nullptr)
        return;
   
    QString url = "https://" + this->ui->ipaddressLabel->text() + ":2908/" + route;
    
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QString authHeader = "Bearer " + *this->jwtToken;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    QNetworkReply *reply;
    if (this->processesList.empty())
        reply = this->manager->get(request);
    else {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QByteArray body = this->getJsonProcList();
        reply = this->manager->post(request, body);
    }

    connect (reply, &QNetworkReply::readyRead, [this, reply, request, url] {
        QByteArray response = reply->readAll();
        std::cout << response.toStdString() << '\n';
        
        if (response.contains("token expired")) 
            this->getNewJwtToken(reply, request);
        else if (!response.contains("Failed")) {
            /// TODO: to keep a list of clients connected in the past (id based)
            /// to set the server online after it's up 

            if (!this->machineIsOn) {
                this->overviewTimer->start();
                this->machineIsOn = true;
                this->ui->statusLabel->setStyleSheet("background-color: rgba(220,252,231,255); color: rgb(1, 102, 48);");
                this->ui->statusLabel->setText("Online");

                QPixmap image (":images/images/onlinePC.png");
                this->ui->serverImage->setPixmap(image);
                this->ui->frame->show();
                this->ui->stackedWidget->show();
                emit this->connReestablished();
            }
        
            this->setUpdateLabelTime();
            QByteArrayList infoList = response.split('\n');
            this->setCpuUsage(infoList[0]);
            this->setRamUsage(infoList[1]);
            this->setDiskUsage(infoList[2]);
            this->createProcList(infoList);
            int procNo = this->processesList.size();
            QString text = "Running processes (";
            text.append(QString::number(procNo)).append(")");
            this->ui->proccessesNoLabel->setText(text);
        }

        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors){ reply->ignoreSslErrors(); });

    connect(reply, &QNetworkReply::errorOccurred, [this, reply] {
        this->handleErrOccurred(reply);
        reply->deleteLater();
    });
}

void MachineFrame::getNewJwtToken(QNetworkReply * reply, QNetworkRequest request) {
    request.setUrl(QUrl("https://" + this->ui->ipaddressLabel->text() + ":2908/updatejwt"));
    std::cout << "expired token\n";
    reply = this->manager->get(request);
    connect (reply, &QNetworkReply::readyRead, [this, reply] {
        QByteArray response = reply->readAll();
        if (response.contains("Failed")) {
            std::cout << response.toStdString() << '\n';
            if (this->machineIsOn) {
                this->ui->statusLabel->setStyleSheet("background-color: #ffe2e2; color: #9f0819;");
                this->ui->statusLabel->setText("Offline");

                QPixmap image (":images/images/offlinePC.png");
                this->ui->serverImage->setPixmap(image);
                this->ui->frame->hide();
                this->ui->stackedWidget->hide();
                this->machineIsOn = false;
            }
        }
        else
            *this->jwtToken = response;
    });

    connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors){ reply->ignoreSslErrors(); });
}

void MachineFrame::setUpdateLabelTime () {
    time_t timePtr = time(NULL); 
    tm* localTime = localtime(&timePtr);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%H:%M:%S");

    std::string text = "Last updated: " + oss.str();
    this->ui->lastUpdatedLabel->setText(text.c_str());
}

void MachineFrame::getOSInfo() {
    QString url = "https://" + this->ui->ipaddressLabel->text() + ":2908/sysinfo";
    
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QString authHeader = "Bearer " + *this->jwtToken;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    QNetworkReply * reply = this->manager->get(request);
    connect (reply, &QNetworkReply::readyRead, [this, reply, request, url] {
        QByteArray response = reply->readAll();
        
        if (response.contains("token expired")) 
            this->getNewJwtToken(reply, request);
        else {
            QByteArrayList osInfoList = response.split('\n');
            QByteArray osName = osInfoList[0].split('=')[1];
            QByteArray osVersion = osInfoList[1].split('=')[1];
            QByteArray arch = osInfoList[2];
            
            osName = osName.mid(1, osName.size() - 2);
            osVersion = osVersion.mid(1, osVersion.size() - 2);
            this->ui->osLabel->setText(osName + " " + osVersion);
            this->ui->archLabel->setText(arch);
        }

        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors){ reply->ignoreSslErrors(); });
    
    connect(reply, &QNetworkReply::errorOccurred, [this, reply] {
        this->handleErrOccurred(reply);
        reply->deleteLater();
    });
}

void MachineFrame::setCpuUsage(const QByteArray & response) {
    if (!response.contains("CPU: "))
        return;

    long long newTotalCpu = response.split(' ')[1].toLongLong();
    long long newWorkCpu = response.split(' ')[2].toLongLong();
    if (this->totalCpu == 0 && this->workCpu == 0) {
        this->totalCpu = newTotalCpu;
        this->workCpu = newWorkCpu;
        float usage = (float) this->workCpu / this->totalCpu * 100;
        this->ui->cpuUsageLabel->setText(QString::fromStdString(std::to_string((int)usage)).append(" %"));
        this->ui->cpuProgBar->setValue((int)usage);
        return;
    }

    long long totalDelta = newTotalCpu - this->totalCpu;
    long long workDelta = newWorkCpu - this->workCpu;
    float usage = (float) workDelta / totalDelta * 100;
    this->ui->cpuUsageLabel->setText(QString::number(usage, 'f', 1).append(" %"));
    this->ui->cpuProgBar->setValue((int)usage);

    this->totalCpu = newTotalCpu;
    this->workCpu = newWorkCpu;
}

void MachineFrame::setRamUsage(const QByteArray & response) {
    if (!response.contains("RAM: "))
        return;

    const int conversionVal = 1048576;
    double totalRam = response.split(' ')[1].toDouble();
    double availableRam = response.split(' ')[2].toDouble();
    double occupiedRam = totalRam - availableRam;
    totalRam /= conversionVal; // printing in GB 
    occupiedRam /= conversionVal;

    QString text = QString::number(occupiedRam, 'f', 1);
    text.append(" GB / ");
    text.append(QString::number(totalRam, 'f', 1));
    text.append(" GB");

    this->ui->ramUsageLabel->setText(text);
    this->ui->ramProgBar->setRange(0, (int)totalRam);
    this->ui->ramProgBar->setValue((int)(occupiedRam));
}

void MachineFrame::setDiskUsage(const QByteArray & response) {
    if (!response.contains("Disk: "))
        return;

    QByteArray totalSpace = response.split(' ')[1];
    QByteArray usedSpace = response.split(' ')[2];
    QString text = usedSpace;
    text.append(" GB / ");
    text.append(totalSpace);
    text.append(" GB");
   
    this->ui->diskUsageLabel->setText(text);
    this->ui->diskProgBar->setRange(0, (int)totalSpace.toDouble());
    this->ui->diskProgBar->setValue((int)usedSpace.toDouble());
}

void MachineFrame::createProcList(const QByteArrayList & procList) {
    for (auto & procLine : procList) 
        if (procLine.contains("Proc:")) {
            QByteArrayList procInfo = procLine.split(' ');
            if (procInfo.size() < 6) continue;

            int pid = procInfo[1].toInt();
            long long procTime = procInfo[3].toLongLong() + procInfo[4].toLongLong();
            float ramUsed = procInfo[5].toInt() / 1024.0;
            
            ProcessFrame * proc = this->procExists(pid);
            if (proc == nullptr) {
                std::string procName = procInfo[2].toStdString();
                
                ProcessFrame *newProc = new ProcessFrame (this, pid, procName, procTime, this->totalCpu, ramUsed);
                connect (newProc, &ProcessFrame::sendKillProcReq, this, &MachineFrame::sendKillProcReq);

                this->processesList.push_back(newProc);
            }
            else {
                proc->updateCpuUsage(procTime, this->totalCpu);
                if (ramUsed > 0)
                    proc->updateRamUsage(ramUsed);
            }
        }
    this->sortProcsByBtn();

    this->ui->topProcNameLabel->setText(this->processesList[0]->getName());
    this->ui->topProcCpuLabel->setText(this->processesList[0]->getCpuLabel());
    this->ui->topProcMemLabel->setText(this->processesList[0]->getRamLabel());
}

ProcessFrame * MachineFrame::procExists (int pid) {
    for (auto proc : this->processesList)
        if (proc->getPid() == pid)
            return proc;
    
    return nullptr;
}

QByteArray MachineFrame::getJsonProcList () {
    std::vector <ProcessFrame *> procListCopy = this->processesList;
    std::sort(procListCopy.begin(), procListCopy.end(),
              [](ProcessFrame *p1, ProcessFrame *p2) {
                  return p1->getCpuUsage() > p2->getCpuUsage();
              });

    QJsonArray procs;
    QJsonObject json;
    int counter = 0;
    for (auto & proc : procListCopy) {
        procs << proc->getPid();
        counter++;
        if (counter == 20)
            break;
    }
    
    json["importantProcs"] = procs;
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

void MachineFrame::handleErrOccurred(const QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::ConnectionRefusedError
    || reply->error() == QNetworkReply::HostNotFoundError
    || reply->error() == QNetworkReply::TimeoutError) 
        if (this->machineIsOn) {
            std::cout << reply->errorString().toStdString() << '\n';
            this->ui->statusLabel->setStyleSheet("background-color: #ffe2e2; color: #9f0819;");
            this->ui->statusLabel->setText("Offline");

            QPixmap image (":images/images/offlinePC.png");
            this->ui->serverImage->setPixmap(image);
            this->ui->frame->hide();
            this->ui->stackedWidget->hide();
            emit this->connRefused();
            this->machineIsOn = false;
            this->overviewTimer->stop();
        }
}

QString MachineFrame::getMachineName () {
    return this->ui->machineName->text();
}

void MachineFrame::setBtnsStyles(QPushButton * clickedBtn) {
    if (this->procsOrderBtn != clickedBtn) {
        const char *style = "border : 2px solid #ececf0;";
        this->ui->memoryBtn->setStyleSheet(style);
        this->ui->cpuBtn->setStyleSheet(style);
        this->ui->nameBtn->setStyleSheet(style);
        clickedBtn->setStyleSheet("border : 2px solid rgb(0, 0, 0);");
        this->procsOrderBtn = clickedBtn;
    }
    else {
        if (!procsOrder) {
            QPixmap image(":images/images/up.png");
            this->ui->orderLabel->setPixmap(image);
            procsOrder = 1;
        }
        else {
            QPixmap image(":images/images/down.png");
            this->ui->orderLabel->setPixmap(image);
            procsOrder = 0;
        }
    }

}

void MachineFrame::sortProcsByBtn() {
    if (this->procsOrderBtn == this->ui->cpuBtn) {
        if (!procsOrder)
            std::sort (this->processesList.begin(), this->processesList.end(), 
                    [](ProcessFrame * p1, ProcessFrame * p2) {
                        return p1->getCpuUsage() > p2->getCpuUsage();
                    });
        else
            std::sort(this->processesList.begin(), this->processesList.end(),
                      [](ProcessFrame *p1, ProcessFrame *p2) {
                          return p1->getCpuUsage() < p2->getCpuUsage();
                      });
    }
    else if (this->procsOrderBtn == this->ui->memoryBtn) {
        if (!procsOrder)
            std::sort (this->processesList.begin(), this->processesList.end(), 
                    [](ProcessFrame * p1, ProcessFrame * p2) {
                        return p1->getRamUsage() > p2->getRamUsage();
                    });
        else
            std::sort(this->processesList.begin(), this->processesList.end(),
                      [](ProcessFrame *p1, ProcessFrame *p2) {
                          return p1->getRamUsage() < p2->getRamUsage();
                      });
    }
    else if (this->procsOrderBtn == this->ui->nameBtn) {
        if (!procsOrder)
            std::sort (this->processesList.begin(), this->processesList.end(), 
                    [](ProcessFrame * p1, ProcessFrame * p2) {
                        return p1->getName() > p2->getName();
                    });
        else
            std::sort(this->processesList.begin(), this->processesList.end(),
                      [](ProcessFrame *p1, ProcessFrame *p2) {
                          return p1->getName() < p2->getName();
                      });
    }

    for (auto &proc : this->processesList)
        this->ui->verticalLayout_10->addWidget(proc);
}

void MachineFrame::sendKillProcReq (int pid) {
    QString url = "https://" + this->ui->ipaddressLabel->text() + ":2908/killproc";

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QString authHeader = "Bearer " + *this->jwtToken;
    request.setRawHeader("Authorization", authHeader.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["process"] = pid;
    QJsonDocument doc(json);
    QByteArray body = doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = this->manager->post(request, body);
    connect(reply, &QNetworkReply::readyRead, [this, reply, request, url, pid] {
        QByteArray response = reply->readAll();
        
        if (response.contains("token expired")) 
            this->getNewJwtToken(reply, request);
        else if (response.contains("Process killed successfully")) {
            std::cout << response.toStdString() << '\n';
            emit this->closeKillProcDialog();
            this->removeProcFromList(pid);
        }

        reply->deleteLater(); 
    });

    connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors) { 
        reply->ignoreSslErrors(); 
    });

    connect(reply, &QNetworkReply::errorOccurred, [this, reply] {
        this->handleErrOccurred(reply);
        reply->deleteLater(); 
    });
}

void MachineFrame::removeProcFromList (int pid) {
    auto it = std::remove_if(this->processesList.begin(), this->processesList.end(),
                             [pid](ProcessFrame *proc) {
                                 if (proc->getPid() == pid) {
                                     delete proc; 
                                     return true; 
                                 }
                                 return false;
                             });
    this->processesList.erase(it, this->processesList.end());
}

MachineFrame::~MachineFrame() {
    delete this->ui;
    delete this->manager;
    delete this->overviewTimer;

    if (this->jwtToken != nullptr)
        delete this->jwtToken;
}