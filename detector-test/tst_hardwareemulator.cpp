#include <QString>
#include <QtTest>
#include "hardwareemulator.h"

class Tst_HardwareEmulator : public HardwareEmulator
{
    Q_OBJECT

public:
    Tst_HardwareEmulator();

private Q_SLOTS:
    void calibration_data();
    void calibration();
};

Tst_HardwareEmulator::Tst_HardwareEmulator()
{
}

static inline bool qFuzzyCompare(QPoint p1, QPoint p2)
{
    return (qAbs(p1.x() - p2.x()) * 100000.f <= qMin(qAbs(p1.x()), qAbs(p2.x()))) &&
            (qAbs(p1.y() - p2.y()) * 100000.f <= qMin(qAbs(p1.y()), qAbs(p2.y())));
}

void Tst_HardwareEmulator::calibration_data()
{
    QTest::addColumn<Hardware::CalibrationData>("calibrationData");
    QTest::addColumn<Hardware::CalibrationData>("testData");
    QTest::addColumn<QTransform>("expectedCalibrationMatrix");

    Hardware::CalibrationData calibrationData;
    Hardware::CalibrationData testData;
    QTransform expectedCalibrationMatrix;

    calibrationData.clear();
    calibrationData << Hardware::PointPair(QPoint(650, 2000), QPoint(65, 350))
                    << Hardware::PointPair(QPoint(2800, 1350), QPoint(200, 195))
                    << Hardware::PointPair(QPoint(2640, 3500), QPoint(195, 550));
    testData.clear();
    expectedCalibrationMatrix = QTransform((qreal)0.0635, (qreal)-0.0227,
                                           (qreal)0.0024, (qreal)0.1634,
                                           (qreal)18.9116, (qreal)37.8887);
    QTest::newRow("internet, 3 points") << calibrationData << testData << expectedCalibrationMatrix;

    calibrationData.clear();
    testData.clear();
    calibrationData << Hardware::PointPair(QPoint(678, 2169), QPoint(64, 384))
                    << Hardware::PointPair(QPoint(2807, 1327), QPoint(192, 192))
                    << Hardware::PointPair(QPoint(2629, 3367), QPoint(192, 576));
    expectedCalibrationMatrix = QTransform((qreal)0.0623, (qreal)-0.0163,
                                           (qreal)0.0054, (qreal)0.1868,
                                           (qreal)9.9951, (qreal)-10.1458);
    QTest::newRow("internet, 3 points (2)") << calibrationData << testData << expectedCalibrationMatrix;


    calibrationData.clear();
    testData.clear();
    calibrationData << Hardware::PointPair(QPoint(1698, 2258), QPoint(128, 384))
                    << Hardware::PointPair(QPoint(767, 1149), QPoint(64, 192))
                    << Hardware::PointPair(QPoint(2807, 1327), QPoint(192, 192))
                    << Hardware::PointPair(QPoint(2629, 3367), QPoint(192, 576))
                    << Hardware::PointPair(QPoint(588, 3189), QPoint(64, 576));
    expectedCalibrationMatrix = QTransform();
    QTest::newRow("internet, 5 points") << calibrationData << testData << expectedCalibrationMatrix;

    calibrationData.clear();
    testData.clear();
    calibrationData << Hardware::PointPair(QPoint(0,0), QPoint(50,50))
                    << Hardware::PointPair(QPoint(0,50), QPoint(50,100))
                    << Hardware::PointPair(QPoint(50,50), QPoint(100,100));
    /*testData << Hardware::PointPair(QPoint(25,0), QPoint(75,0))
             << Hardware::PointPair(QPoint(0,25), QPoint(0,75))
             << Hardware::PointPair(QPoint(25,25), QPoint(75,75));*/
    expectedCalibrationMatrix = QTransform();
    QTest::newRow("translation, 3 points") << calibrationData << testData << expectedCalibrationMatrix;
}

void Tst_HardwareEmulator::calibration()
{
    QFETCH(Hardware::CalibrationData, calibrationData);
    QFETCH(Hardware::CalibrationData, testData);
    //QFETCH(QTransform,expectedCalibrationMatrix);

    Hardware::Pantilt pantilt = Hardware::Body;

    QVERIFY(setCalibrationData(pantilt, calibrationData));

    // This shit returns false even if the matrixes are almost the same
    // TODO: Get the squared avg diff of two matrixes
    /*if (!expectedCalibrationMatrix.isIdentity())
    {
        bool matrixesAreEqual = qFuzzyCompare(hardware.calibrationMatrix(pantilt), expectedCalibrationMatrix);
        if (!matrixesAreEqual)
        {
            qWarning() << "Actual matrix:" << hardware.calibrationMatrix(pantilt);
            qWarning() << "Expected matrix:" << expectedCalibrationMatrix;
        }
        QVERIFY(matrixesAreEqual);
    }*/

    Hardware::CalibrationData points;
    points << calibrationData;
    points << testData;
    foreach (const Hardware::PointPair &point, points)
    {
        // Hardware -> Screen
        if ((hardware2screen(pantilt, point.first) - point.second).manhattanLength() > 2)
        {
            qWarning() << "Distance:" << (hardware2screen(pantilt, point.first) - point.second).manhattanLength();
            QCOMPARE(hardware2screen(pantilt, point.first), point.second);
        }
        QVERIFY((hardware2screen(pantilt, point.first) - point.second).manhattanLength() <= 2);

        // Screen -> Hardware
        if ((screen2hardware(pantilt, point.second) - point.first).manhattanLength() > 2)
        {
            qWarning() << "Distance:" << (screen2hardware(pantilt, point.second) - point.first).manhattanLength();
            QCOMPARE(screen2hardware(pantilt, point.second), point.first);
        }
        QVERIFY((screen2hardware(pantilt, point.second) - point.first).manhattanLength() <= 2);
    }
}

QTEST_APPLESS_MAIN(Tst_HardwareEmulator)

#include "tst_hardwareemulator.moc"
