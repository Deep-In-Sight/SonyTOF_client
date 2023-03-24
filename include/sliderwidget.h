#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

class SliderWidget : public QWidget {
    Q_OBJECT
public:
    explicit SliderWidget(QWidget* parent = nullptr,
                          QString label=QString(""),
                          int id = 0,
                          int min=-10,
                          int max=10,
                          float step = 0.1);
    void setFloatValue(float val);

public slots:
    void updateValue(int val);


signals:
    void newValue(float val, int id);

private:
    int m_id;

    QHBoxLayout* layout;
    QLabel* nameLabel;
    QLabel* valueLabel;
    QSlider* slider;

    float m_step;
};

#endif // SLIDERWIDGET_H
