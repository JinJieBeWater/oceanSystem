#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    m_tcpSocket = new QTcpSocket(this);

    connect(m_tcpSocket, &QTcpSocket::connected, this, &TcpClient::connected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &TcpClient::onErrorOccurred);
}

TcpClient::~TcpClient()
{
    // m_tcpSocket will be deleted by QObject parent-child mechanism
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    if (m_tcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        m_tcpSocket->connectToHost(host, port);
    }
}

void TcpClient::disconnectFromServer()
{
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState)
    {
        m_tcpSocket->disconnectFromHost();
    }
}

void TcpClient::sendMessage(const QByteArray &data)
{
    if (isConnected())
    {
        m_tcpSocket->write(data);
    }
    else
    {
        qWarning() << "Socket is not connected.";
    }
}

bool TcpClient::isConnected() const
{
    return m_tcpSocket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::onReadyRead()
{
    QByteArray data = m_tcpSocket->readAll();
    emit dataReceived(data);
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit this->socketError(m_tcpSocket->errorString());
}
