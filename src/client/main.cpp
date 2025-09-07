#include <QApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon appIcon("../resources/images/server.png"); 
    a.setWindowIcon(appIcon);

    Client c;
    c.show();
    
    return a.exec();
}