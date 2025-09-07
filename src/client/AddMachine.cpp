#include "AddMachine.h"
#include "ui_addmachine.h"

AddMachine::AddMachine(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddMachine)
{
    ui->setupUi(this);

    this->connectComponents();
    this->styleComponents();
}

bool AddMachine::isInputOk () {
    bool ok = true;

    QString style = this->ui->machineNameEdit->styleSheet();
    style.replace("border: none;", "border: 1px solid #f20d0d;");
    style += " QLineEdit:focus { border: 1px solid #f20d0d; }";

    if (this->ui->machineNameEdit->text().isEmpty()) {
        this->ui->machineNameEdit->setStyleSheet (style);
        ok = false;
    }
    if (this->ui->ipaddressEdit->text().isEmpty()) {
        this->ui->ipaddressEdit->setStyleSheet (style);
        ok = false;
    }

    return ok;
}

void AddMachine::connectComponents () {
    for (int i = 0; i < this->ui->quickaddLayout->count(); i++)
    {
        QWidget * widget = this->ui->quickaddLayout->itemAt(i)->widget();
        QPushButton *button = qobject_cast<QPushButton*>(widget);
        
        connect (button, &QPushButton::clicked, [this, button] {
            QString s = button->text();
            int openParen = s.indexOf('(');
            int closeParen = s.indexOf(')');

            if (openParen != -1 && closeParen != -1) {
                QString name = s.left(openParen).trimmed();
                QString ip = s.mid(openParen + 1, closeParen - openParen - 1);

                this->ui->machineNameEdit->setText(name);
                this->ui->ipaddressEdit->setText(ip);
            }
                });
            }
    

    connect (this->ui->cancelBtn, &QPushButton::clicked, [this] { this->close(); } );

    connect (this->ui->addMachineBtn, &QPushButton::clicked, [this] {
        if (!this->isInputOk ())
            return;
        
        emit this->addMachine(this->ui->machineNameEdit->text(), this->ui->ipaddressEdit->text());
    });
}

void AddMachine::styleComponents () {
    this->ui->failedLabel->setVisible (false);

    this->ui->machineNameEdit->setPlaceholderText("e.g., Production Server, Development Box");
    this->ui->ipaddressEdit->setPlaceholderText("e.g., 192.168.1.100");
}

void AddMachine::showConnFailedLabel () {
    this->ui->failedLabel->setStyleSheet("color: rgb(255, 0, 0);");
    this->ui->failedLabel->setText("Connection failed :(");
    
    this->ui->failedLabel->setVisible(true);
}

void AddMachine::showConnSuccessLabel () {
    this->ui->failedLabel->setStyleSheet("color: rgb(0, 170, 0);");
    this->ui->failedLabel->setText("Connection successful!");
    
    this->ui->failedLabel->setVisible(true);
}

void AddMachine::showConnectingLabel () {
    this->ui->failedLabel->setStyleSheet("color: rgba(63, 108, 199, 1);");
    this->ui->failedLabel->setText("Connecting...please wait");
    
    this->ui->failedLabel->setVisible(true);
}

void AddMachine::showNameTakenLabel () {
    this->ui->failedLabel->setStyleSheet("color: rgb(255, 0, 0);");
    this->ui->failedLabel->setText("Name is already taken!");
    
    this->ui->failedLabel->setVisible(true);
}

AddMachine::~AddMachine()
{
    delete ui;
}