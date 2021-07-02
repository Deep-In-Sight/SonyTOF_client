#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fp_atan2_lut.h"
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QFile>

#define DCS_SIZE (640*480*4*2)
#define COLOR_NUM (1<<16)

void getDistance(uint16_t* pDCS, uint16_t** pDistance, bool filter);
void getAmplitude(uint16_t* pDCS, uint16_t** pDistance);
bool colorMapEnable = true;
bool filterEnable = true;
QVector<QColor> colorVec;
int videoRunning = 0;
int frameFinish = 1;

QDateTime* dt = new QDateTime();
qint64 tlast = dt->currentMSecsSinceEpoch();
qint64 tbegin = dt->currentMSecsSinceEpoch();
int frameCnt = 0;

QFile* videoFp = NULL;
QFile* pictureFp = NULL;

enum DISPLAY_MODE : int {
    DCS_MODE = 0,
    AMPLITUDE_MODE,
    DISTANCE_MODE
};

DISPLAY_MODE displayMode;

void colorMap(bool on, uint16_t lumi, int& r, int& g, int& b) {
    int STEP = COLOR_NUM/8;
    int COMP_HIGH = 256;

    if (!on) {
        r = g = b = lumi;
    }
    else if (lumi < STEP) {
        r = 0;
        g = 0;
        b = (lumi+STEP)*COMP_HIGH/(2*STEP);
    }
    else if (lumi < 3*STEP) {
        r = 0;
        g = (lumi - STEP)*COMP_HIGH/(2*STEP);
        b = COMP_HIGH-1;
    }
    else if (lumi < 5*STEP) {
        r = (lumi - 3*STEP)*COMP_HIGH/(2*STEP);
        g = COMP_HIGH-1;
        b = COMP_HIGH-1 - r;
    }
    else if (lumi < 7*STEP) {
        r = COMP_HIGH-1;
        g = COMP_HIGH-1 - (lumi - 5*STEP)*COMP_HIGH/(2*STEP);
        b = 0;
    }
    else {
        r = COMP_HIGH-1 - (lumi - 7*STEP)*COMP_HIGH/(2*STEP);
        g = 0;
        b = 0;
    }
}

void colorVecInit(int vecLen) {
    int r, g, b;

    for (int i = 0; i < vecLen; i++) {
        colorMap(true, i, r, g, b);
        colorVec.push_back(QColor(r, g, b));
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    width = 640;
    height = 480;
    buffer.clear();
    colorVecInit(COLOR_NUM);
    atan2lut_Init();

    colorMapEnable = !!(ui->colorMapCheckBox->checkState());
    filterEnable = !!(ui->medianCheckBox->checkState());

    socket = new QTcpSocket(this);
    socket->setReadBufferSize(0); //unlimited
    connect(socket, SIGNAL(readyRead()), this, SLOT(socket_Read_Data()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::socket_Read_Data()
{
//    if (socket->bytesAvailable() >= DCS_SIZE * 300) {
//        qint64 time = dt->currentMSecsSinceEpoch() - tlast;
//        qDebug() << "socket read time: " << time;
//        qDebug() << "bitrate = " << DCS_SIZE*300/(1<<20) /(time/1000.0) *8 << " Mbitps";
//        exit(0);
//    }
//    qDebug() << "read";
    if(socket->bytesAvailable() >= DCS_SIZE ) {
        frameFinish = 1;
        qDebug() << "socket read time: " << dt->currentMSecsSinceEpoch() - tlast;
        tlast = dt->currentMSecsSinceEpoch();

        buffer.clear();
        buffer.append(socket->readAll());
        imageShow(buffer);

        if (videoRunning) {
            videoFp->write(buffer.data(), DCS_SIZE);
        } else {
            if (videoFp) {
                videoFp->close();
                delete videoFp;
                videoFp = NULL;
            }
            if (pictureFp){
                pictureFp->write(buffer.data(), DCS_SIZE);
                pictureFp->close();
                delete pictureFp;
                pictureFp = NULL;
            }
        }

        qDebug() << ++frameCnt << "image process time: " << dt->currentMSecsSinceEpoch() - tlast;
        tlast = dt->currentMSecsSinceEpoch();

        if (videoRunning) {
            grabFrame();
        }
    }
}

void grayToImage(uint16_t* pData, QImage& img) {
    for(int i = 0 ; i < img.height(); i ++){
        for(int j = 0 ; j < img.width(); j++){
            uint16_t gray = pData[i*img.width()+j];
            img.setPixel(j,i, qRgb(gray, gray, gray));
        }
    }
}



void grayToColorMap(uint16_t* pData, QImage& img, bool colorEnable) {
    int max = 0;
    for(int i = 0 ; i < img.height(); i ++){
        for(int j = 0 ; j < img.width(); j++){
            uint16_t gray = pData[i*img.width()+j];
//            uint16_t gray = j*30000/640;
//            if (gray > max) max = gray;
            if (colorEnable)
                img.setPixelColor(j,i, colorVec.at(gray));
            else
                img.setPixel(j, i, qRgb(gray, gray, gray));
        }
    }
//    qDebug() << "max distance = " << max;
}


void histogram(uint16_t* pData, int length) {
    int bins[10];
    int step = 30000/10;
    memset(bins, 0, sizeof(bins));
    for(int i = 0; i < length; i++) {
        bins[pData[i]/step]++;
    }
    qDebug() << "Distance values histogram: ";
    for (int i = 0; i < 10; i++)
        qDebug() << bins[i] << " ";
}

void MainWindow::imageShow(QByteArray& data)
{
    qDebug()<<"show";

    uint16_t* pAmplitude = NULL;
    uint16_t* pDistance = NULL;
    uint16_t* pDCS = (uint16_t*) data.data();
    int width = 640, height = 480;

    switch (displayMode) {
    case DCS_MODE:
        height = 480*4;
        qimg = QImage(width,height,QImage::Format_RGB888);
        grayToImage(pDCS, qimg);
        break;
    case AMPLITUDE_MODE:
        qimg = QImage(width,height,QImage::Format_RGB888);
        getAmplitude(pDCS, &pAmplitude);
        grayToImage(pAmplitude, qimg);
        break;
    case DISTANCE_MODE:
        qimg = QImage(width,height,QImage::Format_RGB888);
        getDistance(pDCS, &pDistance, filterEnable);
        grayToColorMap(pDistance, qimg, colorMapEnable);
//        histogram(pDistance, 640*480);
        break;
    default:
        break;
    }


//    QPixmap pixmap = QPixmap::fromImage(qimg).scaled(width*1.5, height*1.5);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    scene->clear();
    scene->addPixmap(pixmap);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    qApp->processEvents();
}

void MainWindow::grabFrame() {
    socket->connectToHost(ui->IP->text(),ui->port->text().toInt());
    if(!socket->waitForConnected(30000))
    {
        qDebug() << "Connection failed!";
        return;
    }
//    qDebug() << "Connect successfully!";

    QString str = "getRawData";
    QByteArray sendData = str.toUtf8();
    socket->write(sendData);
    socket->flush();
}


void MainWindow::on_btn_Video_clicked()
{
    if (!videoRunning) {
        QString filename = QString("DCS_video") + dt->currentDateTime().toString("yyyyMMdd_hhmmss") + QString(".bin");
        videoFp = new QFile(filename);
        if (!videoFp->open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot save video";
            delete videoFp;
            videoFp = NULL;
        }
        videoRunning = 1;
        grabFrame();
    }
}

void MainWindow::on_btn_picture_clicked()
{
    if (videoRunning) {
        qDebug() << "stop video";
        videoRunning = 0;
        return;
    }
    QString filename = "DCS_frame_" + dt->currentDateTime().toString("yyyyMMdd_hhmmss") + QString(".bin");
    pictureFp = new QFile(filename);
    if (!pictureFp->open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot save frame";
        delete pictureFp;
        pictureFp = NULL;
    }
    grabFrame();
}


void MainWindow::on_medianCheckBox_stateChanged(int arg1)
{
    filterEnable = !!arg1;
}

void MainWindow::on_colorMapCheckBox_stateChanged(int arg1)
{
    colorMapEnable = !!arg1;
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    displayMode = (DISPLAY_MODE) index;
}
