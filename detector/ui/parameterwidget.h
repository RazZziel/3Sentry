#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H

#include <QWidget>

class Parameter;

namespace Ui {
class ParameterWidget;
}

class ParameterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterWidget(Parameter &detectorParameter, QWidget *parent = 0);
    ~ParameterWidget();

private:
    Ui::ParameterWidget *ui;
    Parameter &m_detectorParameter;

private slots:
    void load();
    void save();

    void on_btnReset_clicked();

signals:
    void dataChanged();
};

#endif // PARAMETERWIDGET_H
