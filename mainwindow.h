#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QElapsedTimer>

#include "imagerthread.h"
#include "colorizerthread.h"
#include "filterthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButton_connect_clicked();
    void on_comboBox_displayMode_currentIndexChanged(int index);
    void on_pushButton_Video_clicked();
    void on_pushButton_picture_clicked();
    void on_checkbox_MedianEnable_stateChanged(int arg1);
    void on_lineEdit_fmod_returnPressed();
    void on_lineEdit_offset_returnPressed();
    void on_lineEdit_intgtime_returnPressed();
    void on_pushButton_i2c_read_clicked();
    void on_pushButton_i2c_write_clicked();

public slots:
    void showError(int error, const QString &message);
    void showResponse(qint16 val, const QString &message);
    void showI2CReadValue(qint8 val);
    void imageShow(QImage qImg);


private:
    void initializeUI();
    void changeIntegration(bool up);
    void fps_update();

    void addColorBar();

    Ui::MainWindow *ui;
    ImagerThread *imager;
    ColorizerThread *colorizer;
    FilterThread *filter;

    QString host;
    int port;

    QElapsedTimer etimer;

    QGraphicsScene* scene = new QGraphicsScene;
};
#endif // MAINWINDOW_H
