#include <QTime>

#include "colorizerthread.h"
#include "imagerthread.h"
#include "profile.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

ColorizerThread::ColorizerThread(QObject *parent)
    :QThread(parent), quit(false), cond_notified(false)
{
}

ColorizerThread::~ColorizerThread() {
    quit = true;
    mutex.lock();
    cond_notified = true;
    cond.wakeOne();
    mutex.unlock();
    wait();

    delete colorVec;
}

QColor& ColorizerThread::getColorJet(quint16 gray) {
    return colorVec[gray];
}

void ColorizerThread::colorize(QByteArray rawImage, int mode) {
//    qDebug() << "new frame comming " << &rawImage;
    QMutexLocker locker(&mutex);
//    queueRawImage.enqueue(rawImage);
//    queueDisplayMode.enqueue(mode);
    pRawImage = rawImage;
    pDisplayMode = mode;

    if (!isRunning()) {
        start();
    } else {
        cond_notified = true;
        cond.wakeOne();
    }
}

void ColorizerThread::run() {
    QImage qImg;
    QSize imgSize;
    cv::Mat grayMat, grayMat8b, grayMat8b_inv, colorMat, colorMatMasked;
    QByteArray rawImage;
    DISPLAY_MODE displayMode;
    char* rawImageData;
    QDateTime datetime;

    while (!quit) {
        mutex.lock();

        rawImage = pRawImage;
        displayMode = (DISPLAY_MODE)pDisplayMode;
        rawImageData = rawImage.data();

        mutex.unlock();

        __TIC__(COLORIZE);

        if (displayMode == DCS_MODE) {
            imgSize = QSize(640, 480*4);
        } else {
            imgSize = QSize(640, 480);
        }

        if (displayMode == DISTANCE_MODE) {
            grayMat = cv::Mat(imgSize.height(), imgSize.width(), CV_16SC1, rawImageData);
            cv::Mat mask = (grayMat > 0);
            cv::cvtColor(mask, mask, cv::COLOR_GRAY2BGR);
            grayMat.convertTo(grayMat8b, CV_8UC1, 1/128.0);
//            cv::bitwise_not(grayMat8b, grayMat8b_inv);
            cv::applyColorMap(grayMat8b, colorMat, _colormap);
            cv::bitwise_and(colorMat, mask, colorMatMasked);
            cv::cvtColor(colorMatMasked, colorMatMasked, cv::COLOR_BGR2RGB);
            qImg = QImage(colorMatMasked.data, colorMat.cols, colorMat.rows, colorMat.step, QImage::Format_RGB888);
        } else if (displayMode == DCS_MODE) {
            grayMat = cv::Mat(imgSize.height(), imgSize.width(), CV_16SC1, rawImageData);
            if (_save_en) {
                QString filename = _save_path + QString("/") +QString::number(QDateTime::currentMSecsSinceEpoch())+".bin";
                QFile frameFile(filename);
                if (!frameFile.open(QIODevice::WriteOnly)){
                    qDebug() << "Cannot open " << filename << " to save DCS frame\r\n";
                }
                frameFile.write(rawImage);
                frameFile.close();
            }

//            grayMat = grayMat * 16; //convert 12bit signed integer to 16bit signed integer by shift right 4 bit

            grayMat.convertTo(grayMat8b, CV_8UC1, 1/16.0, 127.0);
            qImg = QImage(grayMat8b.data, grayMat8b.cols, grayMat8b.rows, grayMat8b.step, QImage::Format_Grayscale8);
        } else {
            grayMat = cv::Mat(imgSize.height(), imgSize.width(), CV_8UC1, rawImageData);
//            grayMat.convertTo(grayMat8b, CV_8UC1);
//            cv::imwrite("amplitude.jpg", grayMat8b);
            qImg = QImage(grayMat.data, grayMat.cols, grayMat.rows, grayMat.step, QImage::Format_Grayscale8);
        }

        __TOC__(COLORIZE);
//        qDebug() << "colorizing done" ;
        emit signalImageDone(qImg);

        mutex.lock();
        while (!cond_notified) {
//            qDebug() << "gonna sleep";
            cond.wait(&mutex);
        }
        cond_notified = false;
//        qDebug() << "awoken, queue has: " << queueRawImage.count();
        mutex.unlock();
    }
}

void ColorizerThread::enable_save(bool save_en, QString& path) {
    _save_en = save_en;
    _save_path = path;
}

void ColorizerThread::changeColormap(int colormap) {
    _colormap = colormap;
}
