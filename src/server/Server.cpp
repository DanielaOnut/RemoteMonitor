#include "Server.h"
#include "Thread.h"

static constexpr int PORT = 2908;

Server::Server(QObject *parent) : QTcpServer(parent) {
    if(!this->listen(QHostAddress::Any, PORT)) {
        std::cout << "Server could not start!";
    }

    sslConfig.setLocalCertificate(QSslCertificate::fromPath("../resources/server.crt").first());
    QFile keyFile("../resources/newserver.key");
    if (!keyFile.open(QIODevice::ReadOnly)) {
        std::cout << "Failed to open private key file!";
        return;
    } else {
        QByteArray keyData = keyFile.readAll();
        keyFile.close();

        QSslKey key(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "remotemonitor");
        if (key.isNull()) {
            std::cout << "Private key is null!\n";
            return;
        }
        sslConfig.setPrivateKey(key);
    }
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);  
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QSslSocket *socket = new QSslSocket(this);

    socket->setSocketDescriptor(socketDescriptor);
    socket->setSslConfiguration(sslConfig);
    socket->startServerEncryption();

    connect(socket, &QSslSocket::readyRead, [this, socket, socketDescriptor]() {
        QByteArray request = socket->readAll();
        
        Thread * thread = new Thread (this, request, socket);
        QThreadPool::globalInstance()->start(thread);
    });

    connect(socket, &QSslSocket::sslErrors, [socket](const QList<QSslError> &errors) {
        for (auto &err : errors)
            std::cout << "Server SSL error: " << err.errorString().toStdString() << "\n";
        socket->disconnectFromHost();
        socket->deleteLater();
    });

    connect (socket, &QSslSocket::errorOccurred, [socket] {
        std::cout << socket->errorString().toStdString() << '\n';
        socket->disconnectFromHost();
        socket->deleteLater();
    });
}

void Server::updateClientsMap (QString id, QString deviceName) {
    QWriteLocker locker(&clientsMapLock);
    this->clientsMap[id] = deviceName;
}

QByteArray Server::getHttpResponse(const char * msg, const char * status) {
    QByteArray response;
    response =
        "HTTP/1.1 " + QByteArray::fromStdString(status) + " \r\n" 
        "Content-Type: text/plain\r\n"
        "Content-Length: " + QByteArray::number(strlen(msg)) +" \r\n"
        "Connection: close\r\n\r\n"
        + msg;

    return response;
}

bool Server::clientExists(const std::string id) {
    QString qid = QString::fromStdString(id);
    QReadLocker locker(&clientsMapLock);
    return clientsMap.contains(qid); 
}

QString & Server::getDeviceNameByClient(const char * id) {
    return this->clientsMap[id];
}

void Server::sendResponse(QSslSocket * socket, const QByteArray & response) {
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
    socket->deleteLater();
}


Server::~Server () {

}