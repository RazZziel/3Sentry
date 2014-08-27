#include "parameterwidget.h"
#include "ui_parameterwidget.h"

#include <QDebug>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>

#include "parameter.h"
#include "util.h"

ParameterWidget::ParameterWidget(Parameter &detectorParameter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParameterWidget),
    m_parameter(detectorParameter)
{
    ui->setupUi(this);

    load();
}

ParameterWidget::~ParameterWidget()
{
    delete ui;
}

void ParameterWidget::load()
{
    Util::clearLayout(ui->horizontalLayout, 1);

    QWidget *widget = NULL;

    switch (m_parameter.m_type)
    {
    case Parameter::Unknown:
        break;

    case Parameter::Boolean:
    {
        QCheckBox *checkBox = new QCheckBox(this);
        checkBox->setChecked(m_parameter.m_value.toBool());
        connect(checkBox, SIGNAL(toggled(bool)), SLOT(save()));
        widget = checkBox;
    }
        break;

    case Parameter::Integer:
    {
        QSpinBox *spinBox = new QSpinBox(this);
        if (!m_parameter.m_minValue.isNull())
            spinBox->setMinimum(m_parameter.m_minValue.toInt());
        if (!m_parameter.m_maxValue.isNull())
            spinBox->setMaximum(m_parameter.m_maxValue.toInt());
        spinBox->setValue(m_parameter.m_value.toInt());
        connect(spinBox, SIGNAL(valueChanged(int)), SLOT(save()));
        widget = spinBox;
    }
        break;

    case Parameter::Real:
    {
        QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox(this);
        if (!m_parameter.m_minValue.isNull())
            doubleSpinBox->setMinimum(m_parameter.m_minValue.toInt());
        if (!m_parameter.m_maxValue.isNull())
            doubleSpinBox->setMaximum(m_parameter.m_maxValue.toInt());
        doubleSpinBox->setValue(m_parameter.m_value.toDouble());
        connect(doubleSpinBox, SIGNAL(valueChanged(double)), SLOT(save()));
        widget = doubleSpinBox;
    }
        break;

    case Parameter::String:
    {
        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setText(m_parameter.m_value.toString());
        connect(lineEdit, SIGNAL(textChanged(QString)), SLOT(save()));
        widget = lineEdit;
    }
        break;

    case Parameter::Color:
    {
        QPushButton *pushButton = new QPushButton(tr("Select"), this);
        connect(pushButton, SIGNAL(clicked()), SLOT(save()));
        widget = pushButton;
    }
        break;

    case Parameter::Selection:
    {
        QComboBox *comboBox = new QComboBox(this);
        foreach (const QString &key, m_parameter.m_options.keys())
        {
            comboBox->addItem(key, m_parameter.m_options.value(key));
        }
        if (!m_parameter.m_defaultValue.isNull())
        {
            comboBox->setCurrentIndex(comboBox->findData(m_parameter.m_value));
        }
        connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(save()));
        widget = comboBox;
    }
        break;
    }

    if (widget)
    {
        ui->horizontalLayout->addWidget(widget);
        adjustSize();
    }
}

void ParameterWidget::save()
{
    QWidget *widget = dynamic_cast<QWidget*>(sender());
    Q_ASSERT(widget != NULL);

    switch (m_parameter.m_type)
    {
    case Parameter::Unknown:
        break;

    case Parameter::Boolean:
    {
        QCheckBox *checkBox = dynamic_cast<QCheckBox*>(widget);
        Q_ASSERT(checkBox != NULL);

        m_parameter.m_value = checkBox->isChecked();
    }
        break;

    case Parameter::Integer:
    {
        QSpinBox *spinBox = dynamic_cast<QSpinBox*>(widget);
        Q_ASSERT(spinBox != NULL);

        m_parameter.m_value = spinBox->value();
    }
        break;

    case Parameter::Real:
    {
        QDoubleSpinBox *doubleSpinBox = dynamic_cast<QDoubleSpinBox*>(widget);
        Q_ASSERT(doubleSpinBox != NULL);

        m_parameter.m_value = doubleSpinBox->value();
    }
        break;

    case Parameter::String:
    {
        QLineEdit *lineEdit = dynamic_cast<QLineEdit*>(widget);
        Q_ASSERT(lineEdit != NULL);

        m_parameter.m_value = lineEdit->text();
    }
        break;

    case Parameter::Color:
    {
        QPushButton *pushButton = dynamic_cast<QPushButton*>(widget);
        Q_ASSERT(pushButton != NULL);

        QColorDialog *d = new QColorDialog(m_parameter.m_value.value<QColor>(), this);
        d->exec();
        m_parameter.m_value = d->selectedColor();
    }
        break;

    case Parameter::Selection:
    {
        QComboBox *comboBox = dynamic_cast<QComboBox*>(widget);
        Q_ASSERT(comboBox != NULL);

        m_parameter.m_value = comboBox->currentData();
    }
        break;
    }

    emit valueChanged(m_parameter.m_code, m_parameter.m_value);
}

void ParameterWidget::on_btnReset_clicked()
{
    m_parameter.m_value = m_parameter.m_defaultValue;
    load();
    emit valueChanged(m_parameter.m_code, m_parameter.m_value);
}
