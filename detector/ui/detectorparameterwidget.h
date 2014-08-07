#ifndef DETECTORPARAMETERWIDGET_H
#define DETECTORPARAMETERWIDGET_H

#include <QWidget>

class DetectorParameter;

namespace Ui {
class DetectorParameterWidget;
}

class DetectorParameterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DetectorParameterWidget(DetectorParameter &detectorParameter, QWidget *parent = 0);
    ~DetectorParameterWidget();

private:
    Ui::DetectorParameterWidget *ui;
    DetectorParameter &m_detectorParameter;

private slots:
    void save();

signals:
    void dataChanged();
};

#endif // DETECTORPARAMETERWIDGET_H
