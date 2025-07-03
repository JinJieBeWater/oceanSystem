#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    bool startServer(quint16 port);
    void stopServer();
    void sendMessageToClient(QTcpSocket *client, const QByteArray &data);
    void broadcastMessage(const QByteArray &data);

signals:
    void newClientConnected(QTcpSocket *clientSocket);
    void clientDisconnected(QTcpSocket *clientSocket);
    void dataReceived(QTcpSocket *clientSocket, const QByteArray &data);
    void serverError(const QString &errorString);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_tcpServer;
    QList<QTcpSocket*> m_clients;
};

#endif // TCPSERVER_H
