#include <QString>
#include <QtTest>
#include "hardwareemulator.h"

class Tst_HardwareEmulator : public HardwareEmulator
{
    Q_OBJECT

public:
    Tst_HardwareEmulator();

private Q_SLOTS:
    void calibration();
};

Tst_HardwareEmulator::Tst_HardwareEmulator()
{
}

void Tst_HardwareEmulator::calibration()
{
    Hardware::CalibrationData calibrationData;
    calibrationData << Hardware::PointPair(QPoint(0,0), QPoint(50,50))
                    << Hardware::PointPair(QPoint(0,50), QPoint(50,100))
                    << Hardware::PointPair(QPoint(50,50), QPoint(100,100));

    setCalibrationData(Hardware::Body, calibrationData);

    QPoint xyOnScreen;
    QPoint xyOnHardware;

    xyOnHardware = screen2hardware(QPoint(50,50));
    QCOMPARE(xyOnHardware, QPoint(0,0));

    xyOnScreen = hardware2screen(QPoint(0,0));
    QCOMPARE(xyOnScreen, QPoint(50,50));
}

QTEST_APPLESS_MAIN(Tst_HardwareEmulator)

#include "tst_hardwareemulator.moc"
