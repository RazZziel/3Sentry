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

private:
    Ui::SentryUI *ui;
    Controller *m_controller;

private slots:
    void onNewOpenCVFrame(cv::Mat image);
    void onCaptureDeviceChanged(int index);
    void onOpenCvViewClicked(Qt::MouseButton button, QPoint pos);
    void updateDetectorParameters();
    void on_btnStartCalibration_clicked();
    void on_btnAbortCalibration_clicked();
};

#endif // SENTRYUI_H
