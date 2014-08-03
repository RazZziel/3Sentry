#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QHostAddress>

class QTcpSocket;

class HTTPRequest : public QObject
{
    Q_OBJECT
public:
    explicit HTTPRequest(QObject *parent = 0);
    explicit HTTPRequest(QTcpSocket *socket, QObject *parent = 0);

    bool isActive();
    bool isValid();

    QString user();
    QString password();
    QString peer();

    QStringList m_path;
    QVariantMap m_arguments;
    QVariantMap m_headers;
    QHostAddress m_peerAddress;
    quint16 m_peerPort;
    QHostAddress m_localAddress;
    quint16 m_localPort;
    QTcpSocket *m_socket;

private:
    QStringList authData();

signals:
    void disconnected();
};

#endif // HTTPREQUEST_H
