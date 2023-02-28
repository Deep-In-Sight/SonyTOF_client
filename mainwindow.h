#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QElapsedTimer>

#include "imagerthread.h"
#include "colorizerthread.h"
#include "filterthread.h"
#include "splashscreen.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent, QString &host, int &port, SplashScreen *splash);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButton_connect_clicked();
    void on_comboBox_displayMode_currentIndexChanged(int index);
    void on_pushButton_Video_clicked();
    void on_pushButton_picture_clicked();
    void on_lineEdit_fmod_returnPressed();
    void on_lineEdit_offset_returnPressed();
    void on_lineEdit_intgtime_returnPressed();
    void on_pushButton_i2c_read_clicked();
    void on_pushButton_i2c_write_clicked();

    void on_pushButton_reboot_clicked();

    void on_lineEdit_threshold_returnPressed();

    void on_hslider_threshold_valueChanged(int value);

    void on_hslider_threshold_sliderReleased();

    void on_comboBox_colormap_currentIndexChanged(int index);

    void on_checkBox_medianBlur_toggled(bool checked);

    void on_checkBox_gaussianBlur_toggled(bool checked);

    void on_checkBox_guidedFilter_toggled(bool checked);

    void on_spinBox_median_ksize_valueChanged(int arg1);

    void on_spinBox_gaussian_ksize_valueChanged(int arg1);

    void on_doubleSpinBox_gaussian_sigma_valueChanged(double arg1);

    void on_spinBox_guided_r_valueChanged(int arg1);

    void on_doubleSpinBox_guided_eps_valueChanged(double arg1);

    void on_pushButton_resetFilters_clicked();


    void on_checkBox_saveRaw_stateChanged(int arg1);

    void on_checkBox_hybridmedian_toggled(bool checked);

public slots:
    void showError(int error, const QString &message);
    void showResponse(qint16 val, const QString &message);
    void showI2CReadValue(qint8 val);
    void imageShow(QImage qImg);

signals:
    void splashMessage(const QString &msg, int alignment, const QColor &color);

private:
    void initializeUI();
    void changeIntegration(bool up);
    void fps_update();

    void addColorBar(int colormap);

    Ui::MainWindow *ui;
    SplashScreen* splash;
    ImagerThread *imager;
    ColorizerThread *colorizer;
    FilterThread *filter;

    QString host;
    int port;

    QElapsedTimer etimer;

    QGraphicsScene* scene = new QGraphicsScene;
};
#endif // MAINWINDOW_H
