#include <unistd.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include "sentryui.h"
#include "ui_sentryui.h"
#include "controller.h"
#include "detector/detector.h"

SentryUI::SentryUI(Controller *controller, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SentryUI),
    m_controller(controller)
{
    ui->setupUi(this);

    connect(m_controller, SIGNAL(newOpenCVFrame(cv::Mat)), SLOT(onNewOpenCVFrame(cv::Mat)));

    ui->cmbCaptureDevice->addItem(tr("File..."), -1);
    ui->cmbCaptureDevice->insertSeparator(ui->cmbCaptureDevice->count());
    for (int i=0; i<m_controller->numCaptureDevices(); ++i)
    {
        ui->cmbCaptureDevice->addItem(tr("Capture #1"), i);
    }
    connect(ui->cmbCaptureDevice, SIGNAL(activated(int)), SLOT(onCaptureDeviceChanged(int)));

    if (QApplication::instance()->arguments().count() > 1)
        m_controller->setCaptureDevice(QApplication::instance()->arguments().value(1));

    foreach (Detector *detector, m_controller->detectors())
    {
        ui->lstDetectors->addItem(detector->name());
    }

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

void SentryUI::onCaptureDeviceChanged(int index)
{
    int captureDevice = ui->cmbCaptureDevice->itemData(index).toInt();
    if (captureDevice == -1)
    {
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

        QSettings settings;
        if (settings.contains("LastVideo"))
        {
            defaultPath = settings.value("LastVideo").toString();
        }

        QString filename = QFileDialog::getOpenFileName(this, tr("Load video"), defaultPath);
        if (!filename.isEmpty())
        {
            if (!m_controller->setCaptureDevice(filename))
            {
                QMessageBox::warning(this, tr("Error"), tr("Could not open video file"));
            }
            else
            {
                settings.setValue("LastVideo", filename);
            }
        }
    }
    else
    {
        if (!m_controller->setCaptureDevice(captureDevice))
        {
            QMessageBox::warning(this, tr("Error"), tr("Could not open capture device"));
        }
    }
}
