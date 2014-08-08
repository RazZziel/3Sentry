#include <unistd.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include "sentryui.h"
#include "ui_sentryui.h"
#include "controller.h"
#include "audio.h"
#include "detector/detector.h"
#include "detector/detectorparameter.h"
#include "ui/detectorparameterwidget.h"

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
    {
        m_controller->setCaptureDevice(QApplication::instance()->arguments().value(1));
    }

    foreach (Detector *detector, m_controller->detectors())
    {
        ui->lstDetectors->addItem(detector->name());
    }
    connect(ui->lstDetectors, SIGNAL(activated(QModelIndex)), SLOT(updateDetectorParameters()));
    updateDetectorParameters();
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
    hide();
    m_controller->stopProcessing();

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

void SentryUI::updateDetectorParameters()
{
    clearLayout(ui->formLayout_detectorProperties);

    if (ui->lstDetectors->currentRow() == -1)
        return;

    Detector *detector = m_controller->detectors().value(ui->lstDetectors->currentRow());
    DetectorParameterMap &parameters = detector->getParameters();

    foreach (const QString &key, parameters.keys())
    {
        DetectorParameter &parameter = parameters[key];
        DetectorParameterWidget *w = new DetectorParameterWidget(parameter, this);
        ui->formLayout_detectorProperties->addRow(parameter.m_name, w);

        // TODO: Don't save every single value, only the one modified
        connect(w, SIGNAL(dataChanged()), detector, SLOT(saveParameterValues()));
    }
}

void SentryUI::clearLayout(QLayout *layout, int start)
{
    int items = layout->count();
    for (int i=start; i<items; ++i)
    {
        QLayoutItem *child = layout->takeAt(start);
        child->widget()->close();
        delete child->widget();
        delete child;
    }
}
