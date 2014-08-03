#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QObject>
#include <QVariant>
#include <QThread>
#include <QSet>

class QTcpSocket;
class QTcpServer;
class QHostAddress;
class HTTPRequest;
class HTTPReply;

class HttpDaemon : public QObject
{
    Q_OBJECT
public:
    HttpDaemon(quint16 port, QObject* parent = 0);
    ~HttpDaemon();

public slots:
    virtual void init();
    virtual void resume();
    virtual void pause();

protected:
    virtual bool authenticateHTTPRequest(HTTPRequest &request);
    virtual void processHTTPRequest(HTTPRequest &request, HTTPReply &reply);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

protected:
    //QThread m_thread;
    QTcpServer *m_tspServer;
    quint16 m_port;
    QSet<QTcpSocket*> m_connectionsInProgress;
};

#endif // HTTPDAEMON_H
