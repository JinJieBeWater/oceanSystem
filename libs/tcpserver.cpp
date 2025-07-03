#include "tcpserver.h"
#include <QDebug>

TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
}

TcpServer::~TcpServer()
{
    stopServer();
}

bool TcpServer::startServer(quint16 port)
{
    if (m_tcpServer->listen(QHostAddress::Any, port)) {
        qInfo() << "Server started on port" << port;
        return true;
    } else {
        emit serverError(m_tcpServer->errorString());
        qWarning() << "Server failed to start:" << m_tcpServer->errorString();
        return false;
    }
}

void TcpServer::stopServer()
{
    for (QTcpSocket *client : qAsConst(m_clients)) {
        client->disconnectFromHost();
    }
    m_tcpServer->close();
    qDeleteAll(m_clients);
    m_clients.clear();
    qInfo() << "Server stopped.";
}

void TcpServer::sendMessageToClient(QTcpSocket *client, const QByteArray &data)
{
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        client->write(data);
    }
}

void TcpServer::broadcastMessage(const QByteArray &data)
{
    for (QTcpSocket *client : qAsConst(m_clients)) {
        sendMessageToClient(client, data);
    }
}

void TcpServer::onNewConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
        if (clientSocket) {
            m_clients.append(clientSocket);
            connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);
            connect(clientSocket, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);
            emit newClientConnected(clientSocket);
        }
    }
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        QByteArray data = clientSocket->readAll();
        emit dataReceived(clientSocket, data);
    }
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        m_clients.removeAll(clientSocket);
        emit clientDisconnected(clientSocket);
        clientSocket->deleteLater();
    }
}
