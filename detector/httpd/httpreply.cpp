#include <QTcpSocket>
#include <QStringList>
#include "httpreply.h"

HTTPReply::HTTPReply(QTcpSocket *socket, QObject *parent) :
    QObject(parent),
    m_textStream(socket),
    m_binStream(socket),
    m_sentHeaders(false)
{
    m_textStream.setAutoDetectUnicode(true);
    m_binStream.setByteOrder(QDataStream::LittleEndian);
}

void HTTPReply::sendHeaders(uint responseCode, const QString &mimeType, const QVariantMap &headers)
{
    QStringList headerList;
    foreach (const QString &key, headers.keys())
        headerList << QString("%1: %2").arg(key, headers.value(key).toString());

    sendHeaders(responseCode, mimeType, headerList);
}

void HTTPReply::sendHeaders(uint responseCode, const QString &mimeType, const QStringList &headers)
{
    if (m_sentHeaders)
    {
        qWarning() << "Headers already sent";
        return;
    }

    QString responseCodeDescription;
    switch (responseCode)
    {
    case 301: responseCodeDescription = "Moved permanently"; break;
    case 404: responseCodeDescription = "Not found"; break;
    case 200:
    default:
        responseCodeDescription = "Ok";
    }

    m_textStream << QString("HTTP/1.0 %1 %2").arg(responseCode).arg(responseCodeDescription) << "\r\n"
                 << QString("Content-Type: %1; charset=\"%2\"").arg(mimeType).arg("utf-8") << "\r\n"
                 << headers.join("\r\n") << "\r\n"
                 << "\r\n";

    m_sentHeaders = true;
}

#if 0
HTTPReply &HTTPReply::operator<<(QBool b) { m_stream << b; return *this; }
HTTPReply &HTTPReply::operator<<(QChar ch) { m_stream << ch; return *this; }
HTTPReply &HTTPReply::operator<<(char ch) { m_stream << ch; return *this; }
HTTPReply &HTTPReply::operator<<(signed short i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(unsigned short i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(signed int i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(unsigned int i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(signed long i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(unsigned long i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(qlonglong i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(qulonglong i) { m_stream << i; return *this; }
HTTPReply &HTTPReply::operator<<(float f) { m_stream << f; return *this; }
HTTPReply &HTTPReply::operator<<(double f) { m_stream << f; return *this; }
HTTPReply &HTTPReply::operator<<(const QString &s) { m_stream << s.toUtf8(); return *this; }
HTTPReply &HTTPReply::operator<<(const QByteArray &array) { m_stream << array; return *this; }
HTTPReply &HTTPReply::operator<<(const char *c) { m_stream << c; return *this; }
HTTPReply &HTTPReply::operator<<(const void *ptr) { m_stream << ptr; return *this; }
#endif
