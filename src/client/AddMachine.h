#ifndef ADDMACHINE_H
#define ADDMACHINE_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QString>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class AddMachine; }
QT_END_NAMESPACE

class AddMachine : public QDialog
{
    Q_OBJECT

private:
    Ui::AddMachine *ui;

public:
    explicit AddMachine(QWidget *parent = nullptr);
    
    void connectComponents ();
    void styleComponents ();
    bool isInputOk (); 
    
    ~AddMachine();

signals:
    void addMachine (QString, QString);

public slots:
    void showConnFailedLabel();
    void showConnSuccessLabel();
    void showConnectingLabel();
    void showNameTakenLabel();
};

#endif // ADDMACHINE_H