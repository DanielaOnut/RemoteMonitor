#include "KillProcess.h"
#include "ui_killprocess.h"

KillProcess::KillProcess(QWidget *parent, QString procName, int pid)
    : QDialog(parent), ui(new Ui::KillProcess)
{
    this->ui->setupUi (this);
    this->pid = pid;

    QString text = this->ui->textArea->placeholderText();
    text.replace("docker", procName);
    text.replace("3141", QString::number(pid));
    this->ui->textArea->setPlaceholderText(text);
    
    connect (this->ui->cancelBtn, &QPushButton::clicked, [this] { this->close(); } );
    connect (this->ui->killBtn , &QPushButton::clicked, [this] {  
        emit this->requestProcessKill(this->pid);
    });
}

KillProcess::~KillProcess()
{
    delete this->ui;
}