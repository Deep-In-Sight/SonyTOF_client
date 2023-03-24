#include "sliderwidget.h"
#include <QDebug>
SliderWidget::SliderWidget(QWidget* parent,
                          QString label,
                          int id,
                          int min,
                          int max,
                          float step) {
    layout = new QHBoxLayout(this);
    nameLabel = new QLabel(label, this);
    valueLabel = new QLabel(this);
    slider = new QSlider(Qt::Horizontal, this);
    layout->addWidget(nameLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(slider);
    slider->setRange(int(min/step), int(max/step));
    slider->setSingleStep(1);
    slider->setValue(0);
    slider->setTracking(false);
    m_step = step;
    m_id = id;
    connect(slider, &QSlider::valueChanged, this, &SliderWidget::updateValue);
}

void SliderWidget::updateValue(int val) {
    qDebug() << "slided";
    float float_val = val*m_step;
    valueLabel->setText(QString::number(val*m_step));
    emit newValue(float_val, m_id);
}

void SliderWidget::setFloatValue(float val) {
    int ival = val / m_step;
    this->slider->setValue(ival);
}
