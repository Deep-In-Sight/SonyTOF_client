#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fp_atan2_lut.h"
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QFile>
#include <QKeyEvent>

#define HW_ACCEL 1
#define DCS_SIZE (640*480*4*2)
#define IMG_SIZE (640*480*2)
#define COLOR_NUM (1<<16)
#define INT_STEP 100

void getDistance(uint16_t* pDCS, uint16_t** pDistance, bool filter);
void getAmplitude(uint16_t* pDCS, uint16_t** pDistance);
bool colorMapEnable = true;
bool filterEnable = true;
QColor* colorVec;
int videoRunning = 0;
int frameFinish = 1;

QDateTime* dt = new QDateTime();
qint64 tlast = dt->currentMSecsSinceEpoch();
qint64 tbegin = dt->currentMSecsSinceEpoch();
int frameCnt = 0;

QFile* videoFp = NULL;
QFile* pictureFp = NULL;

int i2cChanging = 0;

enum DISPLAY_MODE : int {
    DCS_MODE = 0,
    AMPLITUDE_MODE,
    DISTANCE_MODE
};
enum FMOD : int {
    FMOD_24MHz = 24,
    FMOD_20MHz = 20,
    FMOD_16MHz = 16,
    FMOD_12MHz = 12,
    FMOD_8MHz = 8,
    FMOD_4MHz = 4,
    FMOD_NUM
};
DISPLAY_MODE displayMode;
int modFreq;

void colorMap(bool on, int val, int& r, int& g, int& b) {
    int STEP = COLOR_NUM/8;
    int COLOR_MAX = 256;

    if (!on) {
        r = g = b = val;
    }
    else if (val < STEP) {
        r = 0;
        g = 0;
        b = (val+STEP)*COLOR_MAX/(2*STEP);
    }
    else if (val < 3*STEP) {
        r = 0;
        g = (val - STEP)*COLOR_MAX/(2*STEP);
        b = COLOR_MAX-1;
    }
    else if (val < 5*STEP) {
        r = (val - 3*STEP)*COLOR_MAX/(2*STEP);
        g = COLOR_MAX-1;
        b = COLOR_MAX-1 - r;
    }
    else if (val < 7*STEP) {
        r = COLOR_MAX-1;
        g = COLOR_MAX-1 - (val - 5*STEP)*COLOR_MAX/(2*STEP);
        b = 0;
    }
    else {
        r = COLOR_MAX-1 - (val - 7*STEP)*COLOR_MAX/(2*STEP);
        g = 0;
        b = 0;
    }
}

//map integer to color
//vecLen is
void colorVecInit() {
    int r, g, b;

    colorVec = new QColor[COLOR_NUM];

    for (int i = 0; i < COLOR_NUM; i++) {
        colorMap(true, i, r, g, b);
        colorVec[i] = QColor(r, g, b);
    }
}

void MainWindow::addColorBar() {
    QImage barImg = QImage(25,660,QImage::Format_RGB888);
    int H = barImg.height();
    QColor col[H];
    //zero to 2pi fixed point (3,13)
    for (int y = 0; y < H; y++) {
        int grayVal = (int) (y*COLOR_NUM/H);
        col[y] = colorVec[grayVal];
    }
    for (int x = 0; x < barImg.width(); x++) {
        for (int y = 0; y < H; y++) {
            barImg.setPixelColor(x, y, col[y]);
        }
    }
    QPixmap pixmap = QPixmap::fromImage(barImg);
    QGraphicsScene* barScene = new QGraphicsScene;
    barScene->clear();
    barScene->addPixmap(pixmap);
    ui->colorBarView->setScene(barScene);

    modFreq = ui->fmodBox->text().toInt();
    float range = 300.0/(2*modFreq);
    ui->label_minDistance->setText(QString::asprintf("%.02f m", 0.0));
    ui->label_maxDistance->setText(QString::asprintf("%.02f m", range));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    width = 640;
    height = 480;
    buffer.clear();
    buffer2 = new char[640*480*8];
    colorVecInit();
    atan2lut_Init();

    addColorBar();

    colorMapEnable = !!(ui->colorMapCheckBox->checkState());
    filterEnable = !!(ui->medianCheckBox->checkState());
    displayMode = (DISPLAY_MODE) ui->comboBox->currentIndex();

    socket = new QTcpSocket(this);
    socket->setReadBufferSize(0); //unlimited
    connect(socket, SIGNAL(readyRead()), this, SLOT(socket_Read_Data()));

    host = ui->IP->text();
    port = ui->port->text().toInt();
    i2c_client = new i2c(socket, host, port);
    modSet = new modSetting(i2c_client);
}

MainWindow::~MainWindow()
{
    delete socket;
//    delete i2c;
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
    if (i2cChanging) {
        buffer.clear();
        buffer.append(socket->readAll());
        qDebug() << "server response" << QString(buffer).toInt();
        return;
    }
    int datasz;
    if (displayMode == DCS_MODE) {
        datasz = DCS_SIZE;
    } else {
        datasz = IMG_SIZE;
    }
    if(socket->bytesAvailable() >= datasz ) {
        frameFinish = 1;
//        qDebug() << "socket read time: " << dt->currentMSecsSinceEpoch() - tlast;
//        tlast = dt->currentMSecsSinceEpoch();

        buffer.clear();
        buffer.append(socket->readAll());
//        socket->read(buffer2, datasz);
//        buffer = QByteArray::fromRawData(buffer2, datasz);

//        QThread::msleep(20);
//        if (!socket->waitForDisconnected(100)){
//            qDebug() << "tcp disconnect timeout";
//            socket->disconnectFromHost();
//        }
//        qApp->processEvents();
//        if (videoRunning) {
//            grabFrame();
//        }

        imageShow(buffer);

        if (videoRunning) {
            videoFp->write(buffer.data(), datasz);
        } else {
            if (videoFp) {
                videoFp->close();
                delete videoFp;
                videoFp = NULL;
            }
            if (pictureFp){
                pictureFp->write(buffer.data(), datasz);
                pictureFp->close();
                delete pictureFp;
                pictureFp = NULL;
            }
        }
//        qDebug() << "Image show time: " << dt->currentMSecsSinceEpoch() - tlast;
//        tlast = dt->currentMSecsSinceEpoch();

        frameCnt++;
        if (frameCnt % 100 == 0) {
            qDebug() << "Framerate: " << 100*1000.0/(dt->currentMSecsSinceEpoch() - tlast) << "fps";
            tlast = dt->currentMSecsSinceEpoch();
        }
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



void grayToColorMap(uint16_t* pData, QImage& img) {
//    int max = 0;
    for(int i = 0 ; i < img.height(); i ++){
        for(int j = 0 ; j < img.width(); j++){
            uint16_t gray = pData[i*img.width()+j];
            img.setPixelColor(j,i, colorVec[gray]);
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
//    qDebug()<<"show";

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
        if (colorMapEnable)
            grayToColorMap(pDistance, qimg);
        else
            grayToImage(pDistance, qimg);
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
    socket->connectToHost(host, port);
    if(!socket->waitForConnected(30000))
    {
        qDebug() << "Connection failed!";
        return;
    }
//    qDebug() << "Connect successfully!";

    QString str = "getFrame";
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
//        QString cmd = "startVideo";
//        executeCmd(cmd);

        grabFrame();
    } else {
        qDebug() << "stop video";
        videoRunning = 0;
        if (!socket->waitForDisconnected(100)){
            qDebug() << "tcp disconnect timeout";
            socket->disconnectFromHost();
        }
        socket->close();
//        QString cmd = "stopVideo";
//        executeCmd(cmd);
        return;
    }
}

void MainWindow::on_btn_picture_clicked()
{
    if (!videoRunning) {
        QString filename = "DCS_frame_" + dt->currentDateTime().toString("yyyyMMdd_hhmmss") + QString(".bin");
        pictureFp = new QFile(filename);
        if (!pictureFp->open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot save frame";
            delete pictureFp;
            pictureFp = NULL;
        }
        grabFrame();
    }
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
    QString cmd = QString("setMode %1\0").arg(displayMode);
    executeCmd(cmd);
}

void MainWindow::executeCmd(QString cmd) {
    socket->connectToHost(host,port);
    if(!socket->waitForConnected(30000))
    {
        qDebug() << "Connection failed!";
        return;
    }
    socket->write(cmd.toStdString().c_str());
    socket->flush();
    if (!socket->waitForDisconnected(100)){
        qDebug() << "tcp disconnect timeout";
        socket->disconnectFromHost();
    }
    socket->close();
}

void MainWindow::writeReg(i2cReg& r) {
//    qDebug() << host << port;
    QString cmd = QString("w %1 %2\0").arg(r.addr, 0, 16).arg(r.val, 0, 16);
    executeCmd(cmd);
}

void MainWindow::on_fmodBox_returnPressed()
{
    int freq = ui->fmodBox->text().toInt();
    i2cChanging = 1;
//    modSet->applySetting(freq);
    freqmod* setting = modSet->getSetting(freq);

    writeReg(setting->EXCLK_FREQ_MSB);
    writeReg(setting->EXCLK_FREQ_LSB);
    writeReg(setting->PL_RC_VT);
    writeReg(setting->PL_RC_OP);
    writeReg(setting->PL_FC_MX_MSB);
    writeReg(setting->PL_FC_MX_LSB);
    writeReg(setting->PL_RC_MX);
    writeReg(setting->PL_RES_MX);
    writeReg(setting->DIVSELPRE);
    writeReg(setting->DIVSEL);

    i2cChanging = 0;
    addColorBar();
}


void MainWindow::on_lineEdit_offset_returnPressed()
{
    int offsetCM = ui->lineEdit_offset->text().toInt();
    int freq = ui->fmodBox->text().toInt();
    double range = 300.0/(2*freq);
    uint16_t phaseOffset = (uint16_t) (offsetCM/100.0 /range * 65535);
    setPhaseOffset(phaseOffset);
    qDebug() << "Offset = " << phaseOffset;
    QString cmd = QString("setPhaseOffset %1\0").arg(phaseOffset);
    executeCmd(cmd);
}

void MainWindow::setIntegrationTime(int timeus){
    int clk120;
    if (timeus) clk120 = (int)(timeus*1000/8.3);
    else clk120 = 30;

    //16 registers, starting from 0x2120
    for (int i = 0; i < 16; i++) {
        int byteshift = 3-(i%4);
        uint16_t a = 0x2120 + i;
        uint8_t v = (clk120 >> (byteshift*8)) & 0xFF;
        i2cReg exarea_intg = {a,v};
        writeReg(exarea_intg);
    }
}

void MainWindow::on_lineEdit_intgtime_returnPressed()
{
    int time = ui->lineEdit_intgtime->text().toInt();
    setIntegrationTime(time);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lineEdit_intgtime && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (key->key() == Qt::Key_Up) {
            int time = ui->lineEdit_intgtime->text().toInt() + INT_STEP;
            ui->lineEdit_intgtime->setText(QString::number(time));
            setIntegrationTime(time);
        } else if (key->key() == Qt::Key_Down) {
            int time = ui->lineEdit_intgtime->text().toInt() - INT_STEP;
            ui->lineEdit_intgtime->setText(QString::number(time));
            setIntegrationTime(time);
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    qDebug() << "setAmplitude " << arg1;
    QString cmd = QString("setAmplitudeScale %1\0").arg(arg1);
    executeCmd(cmd);
}
