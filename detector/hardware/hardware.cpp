#include "hardware.h"
#include <QMatrix3x3>
#include <QMatrix>
#include <cv.h>

Hardware::Hardware(QObject *parent) :
    QObject(parent),
    m_parameterManager(new ParameterManager(this, this))
{
    qRegisterMetaType<Pantilt>("Pantilt");
}

void Hardware::init()
{
    m_parameterManager->init();
}

QString Hardware::settingsGroup()
{
    return QString("Hardware/%1/").arg(metaObject()->className());
}

ParameterList Hardware::createParameters() const
{
    ParameterList list;
    list << Parameter("calibration/Eye", tr("Eye calibration matrix"), Parameter::String, "")
         << Parameter("calibration/Body", tr("Body calibration matrix"), Parameter::String, "");
    return list;
}

ParameterManager *Hardware::parameterManager()
{
    return m_parameterManager;
}

void Hardware::onParametersChanged()
{

}

bool Hardware::center(Pantilt pantilt)
{
    int minX, maxX, minY, maxY;
    if (!getLimits(pantilt, minX, maxX, minY, maxY))
    {
        qWarning() << "Could not get limits";
        return false;
    }

    return targetAbsolute(pantilt, (minX+maxX)/2, (minY+maxY)/2, false);
}

bool Hardware::setCalibrationData(Pantilt pantilt, CalibrationData calibrationData)
{
    // XY = xyOnHardware
    // XY' = xyOnScreen

#if 1 // OpenCV, N points
    int nPoints = calibrationData.length();
    float cvAdata[nPoints*3];
    float cvACdata[nPoints*2];
    int row=0;
    foreach (const PointPair &pair, calibrationData)
    {
        // A = (X'1 Y'1 1,
        //      X'2 Y'2 1,
        //      X'3 Y'3 1)
        cvAdata[row*3+0] = pair.first.x();
        cvAdata[row*3+1] = pair.first.y();
        cvAdata[row*3+2] = 1;

        // (ax bx cx)t = 1/A * (X1 X2 X3)t
        cvACdata[row*2+0] = pair.second.x();

        // (ay by cy)t = 1/A * (Y1 Y2 Y3)t
        cvACdata[row*2+1] = pair.second.y();

        ++row;
    }
    cv::Mat A(nPoints,3,CV_32F,cvAdata);
    cv::Mat C(nPoints,2,CV_32F,cvACdata);
    std::cout << "A " << A << std::endl;
    std::cout << "C " << C << std::endl;
    std::cout << std::endl;
    //std::cout << "A^(-1) " << A.inv() << std::endl;
    //std::cout << "A^(-1) * C " << A.inv() * C << std::endl;
    //std::cout << std::endl;
    cv::Mat At = A.t();
    std::cout << "(A^t * A)^(-1) * A^t * C " << (At * A).inv() * At * C << std::endl;
    std::cout << std::endl;

    //cv::Mat coef = A.inv() * C; // Specific form for 3 calibration points
    cv::Mat coef = (At * A).inv() * At * C; // Generic form for N calibration points
    float *coefData = (float*) coef.data;
    m_calibrationMatrix[pantilt] = QTransform(coefData[0], coefData[1],
            coefData[2], coefData[3],
            coefData[4], coefData[5]);
#else // Qt, shortcut, 3 or 4 points
    QPolygonF one;
    QPolygonF two;

    for (int i=0; i<4; ++i)
    {
        if (i < calibrationData.length())
        {
            one << calibrationData.at(i).first;
            two << calibrationData.at(i).second;
        }
        else
        {
            one << QPoint(0,0);
            two << QPoint(0,0);
        }
    }

    if (!QTransform::quadToQuad(one, two, m_calibrationMatrix[pantilt]))
    {
        qWarning() << "Could not calculate calibration matrix";
        return false;
    }
#endif

    m_calibrationMatrixInverted[pantilt] = m_calibrationMatrix[pantilt].inverted();

    qDebug() << "Calibration matrix:" << m_calibrationMatrix[pantilt];
    qDebug() << "Inverted calibration matrix:" << m_calibrationMatrixInverted[pantilt];

    return true;
}

QTransform Hardware::calibrationMatrix(Pantilt pantilt)
{
    return m_calibrationMatrix[pantilt];
}

QPoint Hardware::screen2hardware(Pantilt pantilt, QPoint xyOnScreen) const
{
    return m_calibrationMatrixInverted[pantilt].map(xyOnScreen);
}

QPoint Hardware::hardware2screen(Pantilt pantilt, QPoint xyOnHardware) const
{
    return m_calibrationMatrix[pantilt].map(xyOnHardware);
}
