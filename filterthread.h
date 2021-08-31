#ifndef FILTERTHREAD_H
#define FILTERTHREAD_H


#include <QThread>
#include <QImage>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

#include "filterthread.h"

class FilterThread : public QThread
{
    Q_OBJECT

public:
    FilterThread(QObject *parent = 0);
    ~FilterThread();

    void run();

public slots:
    void filter(QByteArray rawImage, int mode);

signals:
    void signalFilterDone(QByteArray filteredRawImage, int mode);

private:
    bool quit;
//    QQueue<QByteArray> queueRawImage;
//    QQueue<int> queueImageMode;
    QByteArray pRawImage;
    int pDisplayMode;

    QMutex mutex;
    QWaitCondition cond;
    bool cond_notified;

    void insertionSort(uint16_t* arr, int n);
    void do_medianFilter(QByteArray &rawImg, QByteArray &rawFiltered, QSize &size, int nLoop);
};

#endif // FILTERTHREAD_H
