#ifndef COLORIZERTHREAD_H
#define COLORIZERTHREAD_H

#include <QThread>
#include <QImage>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

#include "imagerthread.h"

#define COLOR_NUM (1<<8)

class ColorizerThread : public QThread
{
    Q_OBJECT

public:
    ColorizerThread(QObject *parent = 0);
    ~ColorizerThread();

    QColor& getColorJet(quint16 gray);
    void run();

public slots:
    void colorize(QByteArray rawImage, int mode);

signals:
    void signalImageDone(QImage qImg);

private:
    bool quit;
//    QQueue<QByteArray> queueRawImage;
//    QQueue<int> queueDisplayMode;
    QByteArray pRawImage;
    int pDisplayMode;

    QMutex mutex;
    QWaitCondition cond;
    bool cond_notified;

    QColor* colorVec;
    void colorVecInit();
    void do_colormap(QByteArray &rawImg, QImage &qImg);
};

#endif // COLORIZERTHREAD_H
