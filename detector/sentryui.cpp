#include <unistd.h>
#include <QCloseEvent>
#include "sentryui.h"
#include "ui_sentryui.h"
#include "controller.h"

SentryUI::SentryUI(Controller *controller, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SentryUI),
    m_controller(controller)
{
    ui->setupUi(this);

    connect(m_controller, SIGNAL(newOpenCVFrame(cv::Mat)), SLOT(onNewOpenCVFrame(cv::Mat)));

    m_controller->setCaptureDevice("../detector/caras.mp4");
    m_controller->startProcessing();
}

SentryUI::~SentryUI()
{
    delete ui;
}

void SentryUI::onNewOpenCVFrame(cv::Mat image)
{
    ui->wOpenCV->showImage(image);
}

void SentryUI::closeEvent(QCloseEvent *event)
{
    m_controller->stopProcessing();
    sleep(2); // OBNOXIOUS HACK: Wait for audio to play (should try m_player->isAudioAvailable())
    event->accept();
}
