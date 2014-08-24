#include <unistd.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include <QKeyEvent>
#include "sentryui.h"
#include "ui_sentryui.h"
#include "controller.h"
#include "util.h"
#include "audio.h"
#include "detector/detector.h"
#include "ui/parameterwidget.h"

SentryUI::SentryUI(Controller *controller, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SentryUI),
    m_controller(controller)
{
    ui->setupUi(this);
    QApplication::instance()->installEventFilter(this);

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

    updateHardwareParameters();
    updateControllerParameters();

    connect(ui->wOpenCV, SIGNAL(clicked(Qt::MouseButton,QPoint)), SLOT(onOpenCvViewClicked(Qt::MouseButton,QPoint)));

    ui->stackedWidget->setCurrentWidget(m_controller->isCalibrating()
                                        ? ui->page_abortCalibration
                                        : ui->page_startCalibration);
}

SentryUI::~SentryUI()
{
    delete ui;
}

void SentryUI::onNewOpenCVFrame(cv::Mat image)
{
    ui->wOpenCV->setMinimumSize(image.cols, image.rows);
    ui->wOpenCV->setMaximumSize(image.cols, image.rows);
    adjustSize();
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

void SentryUI::onOpenCvViewClicked(Qt::MouseButton button, QPoint pos)
{
    m_controller->nextCalibrationPoint(button == Qt::LeftButton
                                       ? Hardware::Body
                                       : Hardware::Eye,
                                       pos);

    ui->stackedWidget->setCurrentWidget(m_controller->isCalibrating()
                                        ? ui->page_abortCalibration
                                        : ui->page_startCalibration);
}

void SentryUI::updateDetectorParameters()
{
    Util::clearLayout(ui->formLayout_detectorProperties);

    if (ui->lstDetectors->currentRow() == -1)
        return;

    Detector *detector = m_controller->detectors().value(ui->lstDetectors->currentRow());
    fillParameterForm(detector->parameterManager(), ui->formLayout_detectorProperties);
}

void SentryUI::updateHardwareParameters()
{
    Util::clearLayout(ui->formLayout_hardwareProperties);
    fillParameterForm(m_controller->hardware()->parameterManager(), ui->formLayout_hardwareProperties);
}

void SentryUI::updateControllerParameters()
{
    Util::clearLayout(ui->formLayout_controllerProperties);
    fillParameterForm(m_controller->parameterManager(), ui->formLayout_controllerProperties);
}

void SentryUI::fillParameterForm(ParameterManager *parameterManager, QFormLayout *layout)
{
    foreach (const QString &key, parameterManager->parameters().keys())
    {
        Parameter &parameter = parameterManager->parameters()[key];
        ParameterWidget *w = new ParameterWidget(parameter, this);
        layout->addRow(parameter.m_name, w);

        connect(w, SIGNAL(valueChanged(QString,QVariant)),
                parameterManager, SLOT(setParameter(QString,QVariant)));
    }
}


bool SentryUI::eventFilter(QObject *object, QEvent *event)
{
    if (object != this)
        return false;

    if (event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        m_keyState[(Qt::Key)keyEvent->key()] = (event->type() == QEvent::KeyPress);

        bool move = (keyEvent->key() == Qt::Key_Up ||
                     keyEvent->key() == Qt::Key_Down ||
                     keyEvent->key() == Qt::Key_Left ||
                     keyEvent->key() == Qt::Key_Right);

        if (move)
        {
            qreal dx = 0;
            qreal dy = 0;

            if (m_keyState[Qt::Key_Up])
                dy = 1;
            else if (m_keyState[Qt::Key_Down])
                dy = -1;
            else if (m_keyState[Qt::Key_Left])
                dx = -1;
            else if (m_keyState[Qt::Key_Right])
                dx = 1;

            m_controller->targetRelative(Hardware::Body, dx, dy);
        }
        else
        {
            if (event->type() == QEvent::KeyPress)
            {
                switch (keyEvent->key())
                {
                case Qt::Key_1:
                    m_controller->startFiring(Hardware::EyeLaser);
                    break;

                case Qt::Key_2:
                    m_controller->startFiring(Hardware::LeftGun);
                    break;

                case Qt::Key_3:
                    m_controller->startFiring(Hardware::RightGun);
                    break;
                }
            }
            else
            {
                switch (keyEvent->key())
                {
                case Qt::Key_1:
                    m_controller->stopFiring(Hardware::EyeLaser);
                    break;

                case Qt::Key_2:
                    m_controller->stopFiring(Hardware::LeftGun);
                    break;

                case Qt::Key_3:
                    m_controller->stopFiring(Hardware::RightGun);
                    break;
                }
            }
        }
    }

    return false;
 }

void SentryUI::on_btnStartCalibration_clicked()
{
    if (!m_controller->startCalibration())
    {
        QMessageBox::warning(this, tr("Error"), tr("Could not start calibration"));
    }

    ui->stackedWidget->setCurrentWidget(m_controller->isCalibrating()
                                        ? ui->page_abortCalibration
                                        : ui->page_startCalibration);
}

void SentryUI::on_btnAbortCalibration_clicked()
{
    m_controller->abortCallbration();

    ui->stackedWidget->setCurrentWidget(m_controller->isCalibrating()
                                        ? ui->page_abortCalibration
                                        : ui->page_startCalibration);
}
