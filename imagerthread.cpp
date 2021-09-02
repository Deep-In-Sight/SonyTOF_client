#include <QtNetwork>
#include <QDebug>

#include "imagerthread.h"
#include "profile.h"

ImagerThread::ImagerThread(QObject *parent, QString hostName, qint16 port)
    :QThread(parent)
{
    this->hostName = hostName;
    this->port = port;
    this->quit = false;
    this->videoRunning = false;
    this->cond_notified = false;
    this->videoResumeFlag = false;
    this->display_mode = DISTANCE_MODE;
    this->filterEn = false;
    this->modFreq = 24;
    this->phaseOffset = 0;
}

ImagerThread::~ImagerThread()
{
    quit = true;
    if (videoRunning) {
        videoRunning = false;
    }
    mutex.lock();
    cond_notified = true;
    cond.wakeOne();
    mutex.unlock();
    wait();
}

bool ImagerThread::isVideoRunning() {
    return videoRunning;
}

void ImagerThread::getFrame()
{
    videoRunning = false;
    if (!isRunning()) {
        start();
    } else {
        mutex.lock();
        cond_notified = true;
        cond.wakeOne();
        mutex.unlock();
    }
}

void ImagerThread::startVideo()
{
    if (!videoRunning) {
        videoRunning = true;

        if (!isRunning()) {
            start();
        } else {
            mutex.lock();
            cond_notified = true;
            cond.wakeOne();
            mutex.unlock();
        }
    } else {
        emit signalError(-1, "video already running");
    }
}

void ImagerThread::stopVideo()
{
    if (videoRunning)
        videoRunning = false;
    else
        emit signalError(-1, "video not runnign");
}

void ImagerThread::enableMedianFilter(bool enable) {
    filterEn = enable;

}

void ImagerThread::changeDisplayMode(DISPLAY_MODE mode) {
    QStringList cmds;

    qDebug() << "change display mode to " << mode;

    display_mode = mode;

    switch (mode) {
    case DCS_MODE:
        cmds.append("setMode 0");
        cmds.append("setAmplitudeScale 0");
        break;
    case AMPLITUDE_MODE_CLAMP:
        cmds.append("setMode 1");
        cmds.append("setAmplitudeScale 0");
        break;
    case AMPLITUDE_MODE_SCALE:
        cmds.append("setMode 1");
        cmds.append("setAmplitudeScale 1");
        break;
    case DISTANCE_MODE:
        cmds.append("setMode 2");
        cmds.append("setAmplitudeScale 0");
        break;
    default:
        break;
    }

    executeCmd(cmds);
}

void ImagerThread::changeFmod(int freq) {
    QList<i2cReg> regList;
    fmodData.getSetting(freq, regList);
    QStringList cmds;

    modFreq = freq;

    for (i2cReg reg : regList) {
        QString cmd = QString("w %1 %2\0").arg(reg.addr, 0, 16).arg(reg.val, 0, 16);
        cmds.append(cmd);
    }

    executeCmd(cmds);
}

void ImagerThread::changeOffset(int offsetCm) {
    double range = 300.0/(2*modFreq);
    phaseOffset = (quint16) (offsetCm/100.0 /range * 65535);

    qDebug() << "Offset = " << phaseOffset;

    QString cmd = QString("setPhaseOffset %1\0").arg(phaseOffset);
    QStringList cmds;
    cmds.append(cmd);
    executeCmd(cmds);
}

void ImagerThread::changeIntegrationTime(int timeus){
    QStringList cmds;

    int clk120;
    if (timeus) clk120 = (int)(timeus*1000/8.3);
    else clk120 = 30;

    //16 registers, starting from 0x2120
    for (int i = 0; i < 16; i++) {
        int byteshift = 3-(i%4);
        uint16_t a = 0x2120 + i;
        uint8_t v = (clk120 >> (byteshift*8)) & 0xFF;
        QString cmd = QString("w %1 %2\0").arg(a, 0, 16).arg(v, 0, 16);
        cmds.append(cmd);
    }

    executeCmd(cmds);
}

void ImagerThread::i2cReadWrite(bool read, int addr, int val) {
    QString cmd;

    if (read) {
        cmd = QString("r %1\0").arg(addr, 0, 16);
    } else {
        cmd = QString("w %1 %2\0").arg(addr, 0, 16).arg(val, 0, 16);
    }
    qDebug() << "i2c cmd: " << cmd;
    QStringList cmds;
    cmds.append(cmd);
    executeCmd(cmds);
}

//video thread
void ImagerThread::run()
{
    DISPLAY_MODE mode = display_mode;

    while (!quit) {
        //do something lengthy

        if (!videoRunning) {
            do_getFrame(mode, 1);
        } else {
            do_shortCmd("startVideo");
            while (videoRunning) {
                __TIC__(TCP);
                do_getFrame(mode, NFRAME);
                __TOC_FPS__(TCP, NFRAME);
            }
            do_shortCmd("stopVideo");
        }

        mutex.lock();
        //safeguard against lost wake up or spurious wake up
        while (!cond_notified)
            cond.wait(&mutex);
        cond_notified = false;

        //execute commands that was sent mid video
        if (videoResumeFlag) {
            videoRunning = true;
            videoResumeFlag = false;
        }
        while(!cmdQueue.empty()) {
            QString cmd = cmdQueue.dequeue();
            do_shortCmd(cmd);
        }

        mode = display_mode;
        mutex.unlock();
    }
}

void ImagerThread::do_shortCmd(const QString &cmd)
{
    QTcpSocket socket;
    QByteArray buffer;
    int Timeout = 5*1000;

    qDebug() << cmd;

    if (cmd == "getFrame") {
        qDebug() << "Not a short command";
        return;
    }

    socket.connectToHost(hostName, port);
    if(!socket.waitForConnected(Timeout))
    {
        emit signalError(socket.error(), socket.errorString());
        return;
    }
    socket.write(cmd.toStdString().c_str());
    socket.flush();

    while (socket.bytesAvailable() < (int)sizeof(qint16)) {
        if (!socket.waitForReadyRead(Timeout)) {
            emit signalError(socket.error(), socket.errorString());
            return;
        }
    }

    buffer = socket.readAll();
    qint16* val_int16 = (qint16*) buffer.data();

    if (cmd[0] == 'r') {
        qDebug() << "i2c register value = " << val_int16[0];
        emit signalI2CReadVal((qint8)val_int16[0]);
    } else {
        qDebug() << "Server response = " << val_int16[0];
        emit signalNewResponse(val_int16[0], "Server response value: ");
    }

    if (!socket.waitForDisconnected(100)){
        emit signalError(socket.error(), socket.errorString());
        socket.disconnectFromHost();
    }
    socket.close();
}

void ImagerThread::do_getFrame(DISPLAY_MODE mode, int nFrame) {
    QTcpSocket socket;
    QByteArray buffer;
    int Timeout = 5*1000;
    int frameSize;
    int frameCount = 0;

    QString cmd = "getFrame";

    if (mode == DCS_MODE) {
        frameSize = IMG_W * IMG_H * 2 * 4;
    } else {
        frameSize = IMG_W * IMG_H * 2;
    }

    socket.connectToHost(hostName, port);
    if(!socket.waitForConnected(Timeout))
    {
        emit signalError(socket.error(), socket.errorString());
        return;
    }
    socket.write(cmd.toStdString().c_str());
    socket.flush();

    while (frameCount < nFrame) {
        while (socket.bytesAvailable() < frameSize) {
            if (!socket.waitForReadyRead(Timeout)) {
                emit signalError(socket.error(), socket.errorString());
                return;
            }
        }
        buffer = socket.read(frameSize);

        emit signalNewFrame(buffer, (int)mode);
        frameCount++;
    }

    if (!socket.waitForDisconnected(100)){
        emit signalError(socket.error(), socket.errorString());
        socket.disconnectFromHost();
    }
    socket.close();
}

void ImagerThread::executeCmd(const QStringList &cmds)
{
    if (!isRunning()) {
        //execute command immediately
        for(auto &cmd: cmds) {
            do_shortCmd(cmd);
        }
    } else {
        if (videoRunning) {
            qDebug() << "interrupting video";
            videoResumeFlag = true;
            videoRunning = false;
        }

        //defer the commands to run later
        mutex.lock();
        qDebug() << "gonna execute " << cmdQueue.count() << " cmds";
        for (auto &cmd : cmds) {
            cmdQueue.enqueue(cmd);
        }
        cond_notified = true;
        cond.wakeOne();
        mutex.unlock();
    }
}
