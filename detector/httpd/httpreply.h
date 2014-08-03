#ifndef HTTPREPLY_H
#define HTTPREPLY_H

#include <QObject>
#include <QTextStream>
#include <QDataStream>

class QTcpSocket;

class HTTPReply : public QObject
{
    Q_OBJECT
public:
    explicit HTTPReply(QTcpSocket *socket, QObject *parent = 0);

    void sendHeaders(uint responseCode, const QString &mimeType, const QVariantMap &headers = QVariantMap());

    QTextStream &stream() { return m_textStream; }
    QDataStream &binStream() { return m_binStream; }

private:
    void sendHeaders(uint responseCode, const QString &mimeType, const QStringList &headers);

    QTextStream m_textStream;
    QDataStream m_binStream;

    bool m_sentHeaders;
};

#endif // HTTPREPLY_H
