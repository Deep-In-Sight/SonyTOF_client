#include "filterthread.h"
#include "imagerthread.h"
#include "profile.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define MAX_FILTER 10
#define NUMPIX (640*480)
uint16_t filter_tmp[MAX_FILTER][NUMPIX];

FilterThread::FilterThread(QObject* parent)
    :QThread(parent), quit(false), cond_notified(false)
{
}

FilterThread::~FilterThread() {
    quit = true;
    mutex.lock();
    cond_notified = true;
    cond.wakeOne();
    mutex.unlock();
    wait();
}

void FilterThread::filter(const QByteArray rawImage, int mode) {
//    qDebug() << "new frame comming " << &rawImage;
    QMutexLocker locker(&mutex);
//    queueRawImage.enqueue(rawImage);
//    queueImageMode.enqueue(mode);
    pRawImage = rawImage;
    pDisplayMode = mode;

    if (!isRunning()) {
        start();
    } else {
        cond_notified = true;
        cond.wakeOne();
    }
}

void FilterThread::run() {
    QByteArray filteredRaw;
    QSize imgSize;
    cv::Mat grayMat, grayBlurMat;
    QByteArray rawImage;
    DISPLAY_MODE displayMode;

    while (!quit) {
        mutex.lock();
//        qDebug() << "FIlter queue: " << queueRawImage.count();
//        QByteArray rawImage = queueRawImage.dequeue();
//        DISPLAY_MODE displayMode = (DISPLAY_MODE) queueImageMode.dequeue();
        rawImage = pRawImage;
        displayMode = (DISPLAY_MODE) pDisplayMode;
        mutex.unlock();

        if (displayMode == DCS_MODE) {
            imgSize = QSize(640, 480*4);
        } else {
            imgSize = QSize(640, 480);
        }

        __TIC__(MEDIAN_FILTER);
//        do_medianFilter(rawImage, filteredRaw, imgSize, 2);
        grayMat = cv::Mat(imgSize.height(), imgSize.width(), CV_16UC1, rawImage.data());
        cv::medianBlur(grayMat, grayBlurMat, 3);
        filteredRaw = QByteArray::fromRawData((char*)grayBlurMat.data, grayBlurMat.rows*grayBlurMat.step);
        __TOC__(MEDIAN_FILTER);

//        qDebug() << "colorizing done" ;
        emit signalFilterDone(filteredRaw, displayMode);

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

void FilterThread::insertionSort(uint16_t* arr, int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

void FilterThread::do_medianFilter(QByteArray &rawImg, QByteArray &rawFiltered, QSize &size, int nLoop) {
    uint16_t window[9];
    uint16_t* pInput = NULL;
    uint16_t* pOutput = NULL;
    int H = size.height();
    int W = size.width();

    for (int i = 0; i < nLoop; i++) {
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                int pos = y*W+x;
                pOutput = filter_tmp[i];
                if (i == 0)
                    pInput = (uint16_t*) rawImg.data();
                else
                    pInput = filter_tmp[i-1];

                if (x == 0 || y == 0 || x == W-1 || y == H-1) {
                    pOutput[pos] = pInput[pos];
                } else {
                    for (int ky = 0; ky < 3; ky++){
                        for (int kx = 0; kx < 3; kx++) {
                             int wpos = ky*3 + kx;
                             window[wpos] = pInput[(y+ky-1)*W+x+kx-1];
                        }
                    }
                    insertionSort(window,9);
                    pOutput[pos] = window[4];
                }
            }
        }
    }

    rawFiltered = QByteArray((char*)pOutput, H*W*sizeof(uint16_t));
}

