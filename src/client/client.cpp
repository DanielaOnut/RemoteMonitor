#include "client.h"
#include "ui_client.h"
#include "AddMachine.h"

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    QScrollBar * vBar = this->ui->scrollArea->verticalScrollBar();
    vBar->setStyleSheet(R"(QScrollBar:vertical {
        border: none;
        width: 8px;
        margin: 0 2px 0 0; 
    }

    QScrollBar::handle:vertical {
        background: #cecece;
        min-height: 16px;
        border-radius: 3px;
    })");

    this->timer = new QTimer (this);
    this->timer->setInterval(700);

    this->manager = new QNetworkAccessManager(this);

    this->connectComponents();
}

void Client::connectComponents () {
    connect (this->ui->addMachineBtn, &QPushButton::clicked, [this] {
        AddMachine * dialog = new AddMachine (this);

        connect (dialog, &AddMachine::addMachine, this, &Client::addConnection);
        connect (this, &Client::closeDialog, [dialog] { dialog->close(); });
        connect (this, &Client::showConnFailedLabel, dialog, &AddMachine::showConnFailedLabel);
        connect (this, &Client::showConnSuccessLabel, dialog, &AddMachine::showConnSuccessLabel);
        connect (this, &Client::showConnectingLabel, dialog, &AddMachine::showConnectingLabel);
        connect (this, &Client::showNameTakenLabel, dialog, &AddMachine::showNameTakenLabel);

        dialog->exec ();
    });
}

void Client::addConnection (QString machineName, QString ipaddress) {
    if (this->machinesMap.contains(machineName.trimmed())) {
        emit this->showNameTakenLabel();
        return;
    }
    
    emit this->showConnectingLabel();

    FILE * pipe = popen ("hostname", "r");
    char deviceName[300];
    strcpy (deviceName, "device -> ");
    fgets (deviceName + sizeof ("device -> ") - 1, 289, pipe);
    pclose(pipe);
    strcat(deviceName, "machine -> ");
    QByteArray body = deviceName;
    body.append(machineName.toStdString());

    QNetworkRequest request;
    request.setUrl(QUrl("https://" + ipaddress + ":2908/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant::ByteArray);

    QNetworkReply *reply = this->manager->post(request, body);
    connect (reply, &QNetworkReply::encrypted, [this, machineName, ipaddress, reply] {
        std::cout << "Connected to " << machineName.toStdString() << "!\n";
     
        MachineFrame *frame = new MachineFrame(this, machineName, ipaddress);
        this->ui->verticalLayout_7->addWidget(frame);
        emit this->showConnSuccessLabel();

        connect(frame, &MachineFrame::connRefused, this, &Client::machineWentOffline);
        connect(frame, &MachineFrame::connReestablished, this, &Client::machineIsBackOnline);
        connect(frame, &MachineFrame::machineDeleted, this, &Client::deleteMachine);
        this->machinesMap[machineName] = frame;
        this->totalMachines++;
        this->onlineMachines++;

        this->ui->totalMachinesLabel->setText(QString::number(this->totalMachines));
        this->ui->onlineMachinesLabel->setText(QString::number(this->onlineMachines));
    });

    connect (reply, &QNetworkReply::readyRead, [this, reply, machineName] {
        QByteArray data = reply->readAll();
        this->machinesMap[machineName]->setJwtToken(data);
        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::sslErrors, [reply](const QList<QSslError> &errors){
        reply->ignoreSslErrors();
        // std::cout << "SSL errors!";
    });

    connect(reply, &QNetworkReply::errorOccurred, [this, reply] {
        if (reply->error() == QNetworkReply::ConnectionRefusedError
            || reply->error() == QNetworkReply::HostNotFoundError
            || reply->error() == QNetworkReply::TimeoutError)
            emit this->showConnFailedLabel();
        reply->deleteLater();
    });
}

void Client::machineWentOffline() {
    this->onlineMachines--;
    this->offlineMachines++;

    this->ui->onlineMachinesLabel->setText(QString::number(this->onlineMachines));
    this->ui->offlineMachinesLabel->setText(QString::number(this->offlineMachines));
}

void Client::machineIsBackOnline() {
    this->onlineMachines++;
    this->offlineMachines--;

    this->ui->onlineMachinesLabel->setText(QString::number(this->onlineMachines));
    this->ui->offlineMachinesLabel->setText(QString::number(this->offlineMachines));
}

void Client::deleteMachine(MachineFrame *machine){
    if (machine == nullptr)
        return;

    this->totalMachines--;
    this->ui->totalMachinesLabel->setText(QString::number(this->totalMachines));
    if (machine->isMachineOn()) {
        this->onlineMachines--;
        this->ui->onlineMachinesLabel->setText(QString::number(this->onlineMachines));
    }
    else {
        this->offlineMachines--;
        this->ui->offlineMachinesLabel->setText(QString::number(this->offlineMachines));
    }

    this->ui->verticalLayout_7->removeWidget(machine);
    this->machinesMap.remove(machine->getMachineName());
    delete machine;
}

Client::~Client()
{
    delete ui;
    delete this->timer;
    delete this->manager;
}
