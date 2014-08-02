#ifndef SENTRYUI_H
#define SENTRYUI_H

#include <QMainWindow>
#include <cv.h>

class QCloseEvent;
class Controller;

namespace Ui {
class SentryUI;
}

class SentryUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit SentryUI(Controller *controller, QWidget *parent = 0);
    ~SentryUI();

    void closeEvent(QCloseEvent *event);

public slots:
    void onNewOpenCVFrame(cv::Mat image);

private:
    Ui::SentryUI *ui;
    Controller *m_controller;
};

#endif // SENTRYUI_H
