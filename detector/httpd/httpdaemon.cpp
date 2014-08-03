#include <QRegExp>
#include <QTcpSocket>
#include <QTcpServer>
#include <QStringList>
#include <QCoreApplication>
#include <QDateTime>

#include "httpdaemon.h"
#include "httprequest.h"
#include "httpreply.h"

HttpDaemon::HttpDaemon(quint16 port, QObject* parent) :
    QObject(parent),
    m_tspServer(new QTcpServer(this)),
    m_port(port)
{
    // Threading is disabled for now, because this HttpDaemon may have a parent, so moveToThread will fail
    // Threading should be handled in a HttpDaemon wrapper, like WebAdmin and WebAdminDaemon
    //m_thread.start();
    //moveToThread(&m_thread);

    connect(m_tspServer, SIGNAL(newConnection()),  SLOT(onNewConnection()), Qt::DirectConnection);
}

HttpDaemon::~HttpDaemon()
{
    //m_thread.quit();
    //m_thread.wait();
}


void HttpDaemon::onNewConnection()
{
    // When a new client connects, the server constructs a QTcpSocket and all
    // communication with the client is done over this QTcpSocket. QTcpSocket
    // works asynchronously, this means that all the communication is done
    // in the two slots readClient() and discardClient().

    QTcpSocket *clientSocket = m_tspServer->nextPendingConnection();
    if (!clientSocket)
    {
        qWarning() << "Could not get client socket";
        return;
    }

    connect(clientSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(disconnected()), SLOT(onDisconnected()));

    //qDebug() << "New Connection" << clientSocket->peerAddress() << clientSocket->peerPort();
}

void HttpDaemon::init()
{
    resume();
}

void HttpDaemon::pause()
{
    m_tspServer->close();
}

void HttpDaemon::resume()
{
    if (QThread::currentThread() != m_tspServer->thread())
    {
        QMetaObject::invokeMethod(this, __FUNCTION__,
                                  Qt::QueuedConnection);
        return;
    }

    if (m_tspServer->listen(QHostAddress::Any, m_port))
    {
        qDebug() << "HTTP daemon bound to port" << m_port;
    }
    else
    {
        qCritical() << "Could not start HTTP daemon:" << m_tspServer->errorString();
    }
}

void HttpDaemon::onReadyRead()
{
    // This slot is called when the client sent data to the server. The
    // server looks if it was a get request and sends a very simple HTML
    // document back.

    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    m_connectionsInProgress.insert(socket);

    if (socket == NULL)
    {
        qCritical() << "Null socket";
        return;
    }

    HTTPRequest request(socket);

    if (request.isValid())
    {
        HTTPReply reply(socket);

        if (authenticateHTTPRequest(request))
        {
            processHTTPRequest(request, reply);
        }
        else
        {
            QVariantMap headers;
            headers.insert("WWW-Authenticate",
                           QString("Basic realm=\"%1\"").arg(QCoreApplication::applicationName()));
            reply.sendHeaders(401, "text/plain", headers);
            reply.stream() << tr("Not authorized");
        }
    }


    m_connectionsInProgress.remove(socket);

    socket->close();
}

bool HttpDaemon::authenticateHTTPRequest(HTTPRequest &request)
{
    Q_UNUSED(request);
    return true;
}

void HttpDaemon::processHTTPRequest(HTTPRequest &request, HTTPReply &reply)
{
    qDebug() << "Path:" << request.m_path;
    qDebug() << "Arguments:";

    foreach (const QString &key, request.m_arguments.keys())
    {
        qDebug() << "  " << qPrintable(key)
                 << "=" << qPrintable(request.m_arguments.value(key).toString());
    }

    reply.sendHeaders(200, "text/html");
    reply.stream() << "<h1>Nothing to see here</h1>\n"
                   << QDateTime::currentDateTime().toString()
                   << "\n";
}

void HttpDaemon::onDisconnected()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());

    socket->close();

    if (m_connectionsInProgress.contains(socket))
    {
        qDebug() << "Disconnected socket before a reply was sent";
        return;
    }

    socket->deleteLater();

    //qDebug() << "Connection closed";
}
