#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QFile>
#include <QKeyEvent>
#include <profile.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

#define INT_STEP 100

void MainWindow::addColorBar() {
    static bool first = true;

    if (first) {
        first = false;
        int W = 25, H = 660;
        cv::Mat grayBar(H, W, CV_8UC1);
        cv::Mat grayBarInv, colorBar;
        for (int x = 0; x < W; x++) {
            for (int y = 0; y < H; y++) {
                grayBar.at<uchar>(y, x) = (uchar) (y*256/H);
            }
        }
        cv::bitwise_not(grayBar, grayBarInv);
        cv::applyColorMap(grayBarInv, colorBar, cv::COLORMAP_JET);
        QImage qImg = QImage(colorBar.data, colorBar.cols, colorBar.rows, colorBar.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(qImg);
        QGraphicsScene* barScene = new QGraphicsScene;
        barScene->clear();
        barScene->addPixmap(pixmap);
        ui->colorBarView_3->setScene(barScene);
    }

    int modFreq = ui->lineEdit_fmod->text().toInt();
    float range = 300.0/(2*modFreq);
    ui->label_minDistance_3->setText(QString::asprintf("%.02f m", 0.0));
    ui->label_maxDistance_3->setText(QString::asprintf("%.02f m", range));
}

MainWindow::MainWindow(QWidget *parent, QString &host, int &port)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    this->host = host;
    this->port = port;

    initializeUI();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lineEdit_intgtime && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (key->key() == Qt::Key_Up || key->key() == Qt::Key_Down) {
            bool up = (key->key() == Qt::Key_Up);
            changeIntegration(up);
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::initializeUI() {

    colorizer = new ColorizerThread();
    filter = new FilterThread();

    ui->lineEdit_hostName->setText(host);
    ui->lineEdit_port->setText(QString::number(port));

    imager = new ImagerThread(this, host, port);

    connect(imager, &ImagerThread::signalError, this, &MainWindow::showError);
    connect(imager, &ImagerThread::signalNewResponse, this, &MainWindow::showResponse);
    connect(imager, &ImagerThread::signalI2CReadVal, this, &MainWindow::showI2CReadValue);
    connect(imager, &ImagerThread::signalNewFrame, colorizer, &ColorizerThread::colorize);
    connect(colorizer, &ColorizerThread::signalImageDone, this, &MainWindow::imageShow);

    DISPLAY_MODE mode = DISTANCE_MODE;
    ui->comboBox_displayMode->setCurrentIndex(mode);
    imager->changeDisplayMode(mode);

    int freq = 24;
    ui->lineEdit_fmod->setText(QString::number(freq));
    imager->changeFmod(freq);

    ui->checkbox_MedianEnable->setChecked(false);
    imager->enableMedianFilter(false);

    int offsetCm = 0;
    ui->lineEdit_offset->setText(QString::number(offsetCm));
    imager->changeOffset(offsetCm);

    int integration_us = 1000;
    ui->lineEdit_intgtime->setText(QString::number(integration_us));
    imager->changeIntegrationTime(integration_us);

    ui->lineEdit_i2c_addr->setText("");
    ui->lineEdit_i2c_val->setText("");

    addColorBar();
}

MainWindow::~MainWindow()
{
    delete imager;
    delete colorizer;
    delete filter;
    delete ui;
}

void MainWindow::imageShow(QImage qImg) {
//    qDebug() << "new frame to show " << qImg.width() << " " << qImg.height();
    QPixmap pixmap = QPixmap::fromImage(qImg);
    scene->clear();
    scene->addPixmap(pixmap);
    ui->graphicsView_Main->setScene(scene);
    ui->graphicsView_Main->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    fps_update();
}

void MainWindow::fps_update() {
    static int frameCnt = 0;

    if (frameCnt == 50) {
        frameCnt = 0;
        float fps = 50.0*1e9/etimer.nsecsElapsed();
        etimer.start();
        ui->labelStatus->setText(QString("FPS: ")+QString::number(fps));
    } else {
        frameCnt++;
    }
}

void MainWindow::on_pushButton_connect_clicked()
{
    host = ui->lineEdit_hostName->text();
    port = ui->lineEdit_port->text().toInt();
    delete imager;
    delete colorizer;
    delete filter;
    initializeUI();
}

void MainWindow::on_comboBox_displayMode_currentIndexChanged(int index)
{
    DISPLAY_MODE mode = (DISPLAY_MODE) index;
    imager->changeDisplayMode(mode);
}

void MainWindow::on_pushButton_Video_clicked()
{
    if (imager->isVideoRunning()) {
        imager->stopVideo();
    } else {
        imager->startVideo();
    }
}

void MainWindow::on_pushButton_picture_clicked()
{
    imager->getFrame();
}

void MainWindow::on_checkbox_MedianEnable_stateChanged(int arg1)
{
    bool enable = !!arg1;
    imager->enableMedianFilter(enable);
    if (enable) {
        disconnect(imager, &ImagerThread::signalNewFrame, colorizer, &ColorizerThread::colorize);
        connect(imager, &ImagerThread::signalNewFrame, filter, &FilterThread::filter);
        connect(filter, &FilterThread::signalFilterDone, colorizer, &ColorizerThread::colorize);
    } else {
        disconnect(imager, &ImagerThread::signalNewFrame, filter, &FilterThread::filter);
        disconnect(filter, &FilterThread::signalFilterDone, colorizer, &ColorizerThread::colorize);
        connect(imager, &ImagerThread::signalNewFrame, colorizer, &ColorizerThread::colorize);
    }
}

void MainWindow::on_lineEdit_fmod_returnPressed()
{
    bool okay;
    int freq = ui->lineEdit_fmod->text().toInt(&okay, 10);
    if (okay && freq >=4 && freq <= 100) {
        imager->changeFmod(freq);
        addColorBar();
    } else {
        showError(-1, "Invalid freqency value ");
    }
}

void MainWindow::on_lineEdit_offset_returnPressed()
{
    int offsetCM = ui->lineEdit_offset->text().toInt();
    imager->changeOffset(offsetCM);
}

void MainWindow::on_lineEdit_intgtime_returnPressed()
{
    int timeus = ui->lineEdit_intgtime->text().toInt();
    imager->changeIntegrationTime(timeus);
}

void MainWindow::on_pushButton_i2c_read_clicked()
{
    QString addrStr = ui->lineEdit_i2c_addr->text();
    bool okay;
    qint16 addr = addrStr.toUInt(&okay, 16);
    if (okay) {
        imager->i2cReadWrite(true, addr);
    } else {
        showError(-1, "Invalid I2C address");
    }
}

void MainWindow::on_pushButton_i2c_write_clicked()
{
    QString addrStr = ui->lineEdit_i2c_addr->text();
    QString valStr = ui->lineEdit_i2c_val->text();

    bool okay1, okay2;
    qint16 addr = addrStr.toUInt(&okay1, 16);
    qint8 val = valStr.toUInt(&okay2, 16);

    if (okay1 && okay2) {
        imager->i2cReadWrite(false, addr, val);
    } else {
        showError(-1, "Invalid i2c addr or val");
    }
}

void MainWindow::showError(int error, const QString &message) {
    QString text = message + ", errorCode = " + QString::number(error);
    ui->labelStatus->setText(text);
}

void MainWindow::showResponse(qint16 val, const QString &message) {
    QString text = message + QString::number(val);
    ui->labelStatus->setText(text);
}

void MainWindow::showI2CReadValue(qint8 val) {
    ui->lineEdit_i2c_val->setText(QString::number(val, 16));
}

void MainWindow::changeIntegration(bool up) {
    int timeus;
    if (up) {
        timeus = ui->lineEdit_intgtime->text().toInt() + INT_STEP;
    } else {
        timeus = ui->lineEdit_intgtime->text().toInt() - INT_STEP;
    }
    ui->lineEdit_intgtime->setText(QString::number(timeus));
    imager->changeIntegrationTime(timeus);
}




