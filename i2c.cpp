#include "i2c.h"

i2c::i2c(QTcpSocket* socket, QString host, int port)
{
    this->socket = socket;
    this->host = host;
    this->port = port;
}

void i2c::readReg(i2cReg& r) {
//    char command[20];

//    snprintf(command, sizeof(command), "readRegister %04x", r.addr);
//    this->socket->write(QByteArray(command));
//    this->socket->flush();
}

void i2c::writeReg(i2cReg& r) {
    qDebug() << host << port;
    socket->connectToHost(host,port);
    if(!socket->waitForConnected(30000))
    {
        qDebug() << "Connection failed!";
        return;
    }
    QString cmd = QString("writeRegister %1 %2").arg(r.addr, 0, 16).arg(r.val, 0, 16);
    QByteArray ba = cmd.toUtf8();

    this->socket->write(ba);
    this->socket->flush();
}
