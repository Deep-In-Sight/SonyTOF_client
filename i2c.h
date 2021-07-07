#ifndef I2C_H
#define I2C_H

#include <QTcpSocket>

class i2cReg {
public:
    uint16_t addr;
    uint8_t val;
};

class i2c
{
public:
    i2c(QTcpSocket* socket, QString host, int port);
    void writeReg(i2cReg& r);
    void readReg(i2cReg& r);
private:
    QTcpSocket* socket;
    QString host;
    int port;
};

#endif // I2C_H
