#include <QTcpSocket>
#include <QUrl>
#include "httprequest.h"

HTTPRequest::HTTPRequest(QObject *parent) :
    QObject(parent),
    m_socket(NULL)
{
}

HTTPRequest::HTTPRequest(QTcpSocket *socket, QObject *parent) :
    QObject(parent)
{
    m_peerAddress = socket->peerAddress();
    m_peerPort = socket->peerPort();
    m_localAddress = socket->localAddress();
    m_localPort = socket->localPort();
    m_socket = socket;

    connect(socket, SIGNAL(disconnected()), SIGNAL(disconnected()));

    while (socket->canReadLine())
    {
        QByteArray line = socket->readLine();

        if (line.startsWith("GET"))
        {
            QStringList requestTokens = QString(line).split(QRegExp("[ \r\n][ \r\n]*"), QString::SkipEmptyParts);

            QStringList url = QUrl::fromEncoded(requestTokens[1].toUtf8()).toString().split('?');
            m_path = url[0].split('/', QString::SkipEmptyParts);
            if (m_path.isEmpty()) m_path << "/";

            if (url.count() > 1)
            {
                foreach (const QString &token, url[1].split('&'))
                {
                    QStringList tokenList = token.split('=', QString::KeepEmptyParts);
                    QString key = tokenList[0].trimmed();
                    if (!key.isEmpty())
                    {
                        QString value;
                        if (tokenList.count() > 1)
                            value = tokenList[1].trimmed();

                        m_arguments.insert(key, value);
                    }
                }
            }
        }
        else
        {
            QStringList headerList = QString(line).split(':', QString::SkipEmptyParts);

            if (!headerList.isEmpty())
            {
                QString key = headerList.value(0);
                QString value;
                if (headerList.length() > 1)
                    value = headerList.value(1);

                m_headers.insert(key, value);
            }
        }
    }
}

bool HTTPRequest::isActive()
{
    return m_socket && m_socket->isOpen();
}

bool HTTPRequest::isValid()
{
    return !m_path.isEmpty();
}

QString HTTPRequest::user()
{
    QStringList userPass = authData();
    if (userPass.length() == 2)
    {
        return userPass[0];
    }

    return QString::null;
}

QString HTTPRequest::password()
{
    QStringList userPass = authData();
    if (userPass.length() == 2)
    {
        return userPass[1];
    }

    return QString::null;
}

QStringList HTTPRequest::authData()
{
    if (m_headers.contains("Authorization"))
    {
        QStringList authList = m_headers.value("Authorization").toString().split(' ', QString::SkipEmptyParts);
        if (authList.length() == 2)
        {
            QString auth = authList[1];
            return QString(QByteArray::fromBase64(auth.toLatin1())).split(':');
        }
    }

    return QStringList();
}

QString HTTPRequest::peer()
{
    return m_peerAddress.toString();
}
