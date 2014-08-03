#include "sentryweb.h"
#include "controller.h"
#include "httpd/httprequest.h"
#include "httpd/httpreply.h"

#define MJPEG_BOUNDARY "--badmotherfucker--"

SentryWeb::SentryWeb(Controller *controller) :
    HttpDaemon(8008, controller),
    m_controller(controller)
{
    connect(m_controller, SIGNAL(newOpenCVFrame(cv::Mat)), SLOT(onNewOpenCVFrame(cv::Mat)));
    init();
}

#include <QEventLoop>
void SentryWeb::processHTTPRequest(HTTPRequest &request, HTTPReply &reply)
{
    QVariantMap headers;
    headers.insert("Cache-Control", "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0");
    headers.insert("Pragma", "no-cache");

    reply.sendHeaders(200, "multipart/x-mixed-replace; boundary=" MJPEG_BOUNDARY, headers);

    while (request.isActive())
    {
        QEventLoop loop;
        QObject::connect(this, SIGNAL(newFrameReady()), &loop, SLOT(quit()));
        QObject::connect(&request, SIGNAL(disconnected()), &loop, SLOT(quit()));
        QTimer::singleShot(5000, &loop, SLOT(quit()));
        loop.exec();

        if (!request.isActive())
        {
            return;
        }

        reply.stream() << MJPEG_BOUNDARY << "\n";
        reply.stream() << "Content-Type: image/jpeg" << "\n";
        reply.stream() << "\n";
        reply.stream().flush();
        reply.binStream().writeRawData(m_imageData.constData(), m_imageData.length());
    }
}

#include <QBuffer>
void SentryWeb::onNewOpenCVFrame(cv::Mat image)
{
    QBuffer buffer(&m_imageData);
    buffer.open(QIODevice::WriteOnly);
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf);
    buffer.write(reinterpret_cast<char*>(&buf[0]), buf.size());
    buffer.close();

    emit newFrameReady();
}
