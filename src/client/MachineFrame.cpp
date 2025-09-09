#include "MachineFrame.h"
#include "ui_MachineFrame.h"
#include "ProcessFrame.h"

MachineFrame::MachineFrame(QWidget *parent, const QString & machineName, const QString & ipaddress)
    : QFrame(parent)
    , ui(new Ui::MachineFrame) 
{
    ui->setupUi(this);

    this->manager = new QNetworkAccessManager(this);
    this->updateTimer = new QTimer (this);

    this->updateTimer->setInterval(1000);
    this->updateTimer->start();
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
        this->updateTimer->stop();
        emit this->machineDeleted(this);
    });

    connect(this->ui->memoryBtn, &QPushButton::clicked, [this] {
        ProcessFrame * process = new ProcessFrame (this);
        this->ui->verticalLayout_10->addWidget(process);
     });

    connect (this->updateTimer, &QTimer::timeout, [this] {
        // this->setUpdateLabelTime();
        this->sendUpdateRequest("updateall"); 
    });
}

void MachineFrame::setJwtToken(const QByteArray & token) {
    this->jwtToken = new QString(token);
    this->getOSInfo();
}

void MachineFrame::sendUpdateRequest (const QString route) {
    if (this->jwtToken == nullptr)
        return;
   
    QString url = "https://" + this->ui->ipaddressLabel->text() + ":2908/" + route;
    
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QString authHeader = "Bearer " + *this->jwtToken;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    QNetworkReply * reply = this->manager->get(request);
    connect (reply, &QNetworkReply::readyRead, [this, reply, request, url] {
        QByteArray response = reply->readAll();
        
        if (response.contains("token expired")) 
            this->getNewJwtToken(reply, request);
        else if (!response.contains("Failed")) {
            /// TODO: to keep a list of clients connected in the past (id based)
            /// to set the server online after it's up 
            std::cout << response.toStdString() << '\n';

            if (!this->machineIsOn) {
                this->updateTimer->start();
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
            QByteArrayList statusesList = response.split('\n');
            this->setCpuUsage(statusesList[0]);
            this->setRamUsage(statusesList[1]);
            this->setDiskUsage(statusesList[2]);
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

void MachineFrame::handleErrOccurred(const QNetworkReply * reply) {
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
            this->updateTimer->stop();
        }
}

QString MachineFrame::getMachineName () {
    return this->ui->machineName->text();
}

MachineFrame::~MachineFrame() {
    delete this->ui;
    delete this->manager;
    delete this->updateTimer;

    if (this->jwtToken != nullptr)
        delete this->jwtToken;
}