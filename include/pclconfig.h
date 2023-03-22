#ifndef PCLCONFIG_H
#define PCLCONFIG_H

#include <QObject>
#include <QWidget>


namespace Ui {
class pclconfig_widget;
}

class PCLViewer;

class pclconfig : public QWidget
{
    Q_OBJECT
public:
    explicit pclconfig(QWidget *parent = nullptr, PCLViewer* viewer = nullptr);
    ~pclconfig();

signals:
    void downsamplerateChanged(int& factor);
    void viewPositionChanged(float& x, float& y, float& z);
    void resetView();

private slots:
    void on_pushButton_resetView_clicked();

    void on_spinBox_point_downsampling_valueChanged(int arg1);

private:
    Ui::pclconfig_widget* ui;
};

#endif // PCLCONFIG_H
