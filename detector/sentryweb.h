#ifndef WEBUI_H
#define WEBUI_H

#include <cv.h>
#include "httpd/httpdaemon.h"

class Controller;

class SentryWeb : public HttpDaemon
{
    Q_OBJECT
public:
    explicit SentryWeb(Controller *controller);

private:
    void processHTTPRequest(HTTPRequest &request, HTTPReply &reply);

    Controller *m_controller;
    QByteArray m_imageData;

private slots:
    void onNewOpenCVFrame(cv::Mat image);

signals:
    void newFrameReady();
};

#endif // WEBUI_H
