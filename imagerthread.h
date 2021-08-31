#ifndef IMAGERTHREAD_H
#define IMAGERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QStringList>

#include "freqmod.h"

#define IMG_W 640
#define IMG_H 480
#define NFRAME 10

enum DISPLAY_MODE : int {
    DCS_MODE = 0,
    AMPLITUDE_MODE_CLAMP,
    AMPLITUDE_MODE_SCALE,
    DISTANCE_MODE
};

class ImagerThread : public QThread
{
    Q_OBJECT

public:
    ImagerThread(QObject *parent = 0, QString hostName = "localhost", qint16 port = 0);
    ~ImagerThread();

    bool isVideoRunning();
    void startVideo();
    void stopVideo();
    void getFrame();
    void enableMedianFilter(bool enable);
    void changeDisplayMode(DISPLAY_MODE mode);
    void changeFmod(int freq);
    void changeOffset(int offsetCm);
    void changeIntegrationTime(int timeus);
    void i2cReadWrite(bool read, int addr, int val = 0);

    void run();

signals:
    void signalNewFrame(const QByteArray rawFrame, int mode);
    void signalNewResponse(qint16 resp, const QString &message);
    void signalI2CReadVal(qint8 val);
    void signalError(int socketError, const QString &message);

private:
    QString hostName;
    qint16 port;
    QQueue<QString> cmdQueue;

    QMutex mutex;
    QWaitCondition cond;
    bool cond_notified;

    modSetting fmodData;
    int modFreq;
    quint16 phaseOffset;
    DISPLAY_MODE display_mode;
    bool filterEn;

    bool videoResumeFlag;
    bool videoRunning;
    bool quit;

    void do_shortCmd(const QString &cmd);
    void do_getFrame(DISPLAY_MODE mode, int nFrame);
    void executeCmd(const QStringList &cmds);
};

#endif // IMAGERTHREAD_H
