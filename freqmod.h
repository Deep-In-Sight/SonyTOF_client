#ifndef FREQMOD_H
#define FREQMOD_H

#include <i2c.h>
#include <QFile>
#include <QDebug>
#include <string.h>

class freqmod {
public:
    i2cReg EXCLK_FREQ_MSB;
    i2cReg EXCLK_FREQ_LSB;
    i2cReg PL_RC_VT;
    i2cReg PL_RC_OP;
    i2cReg PL_FC_MX_MSB;
    i2cReg PL_FC_MX_LSB;
    i2cReg PL_RC_MX;
    i2cReg PL_RES_MX;
    i2cReg DIVSELPRE;
    i2cReg DIVSEL;
};

class modSetting {
public:
    modSetting (){
        buildSettings();
    }

    modSetting (i2c* pi2c) : i2c_client(pi2c) {
        buildSettings();
    }

    ~modSetting() {
        delete modSettings;
    }

    //return the setting for a modulation frequency
    freqmod* getSetting(int freq) {
        if (freq < minFreq || freq > maxFreq) {
            return NULL;
        } else {
            freqmod* set = &modSettings[maxFreq-freq];
            printFreqMod(set);
            return set;
        }
    }

    //apply the setting for a modulation frequency
    int applySetting(int freq) {
        freqmod* set = getSetting(freq);
        if (!set) {
            return -1;
        }
        i2c_client->writeReg(set->EXCLK_FREQ_MSB);
        i2c_client->writeReg(set->EXCLK_FREQ_LSB);
        i2c_client->writeReg(set->PL_RC_VT);
        i2c_client->writeReg(set->PL_RC_OP);
        i2c_client->writeReg(set->PL_FC_MX_MSB);
        i2c_client->writeReg(set->PL_FC_MX_LSB);
        i2c_client->writeReg(set->PL_RC_MX);
        i2c_client->writeReg(set->PL_RES_MX);
        i2c_client->writeReg(set->DIVSELPRE);
        i2c_client->writeReg(set->DIVSEL);
        return 0;
    }
private:
    const int minFreq = 4;
    const int maxFreq = 100;
    const int numRegs = 10;
    const int numSettings = maxFreq - minFreq + 1;
    freqmod* modSettings;
    i2c* i2c_client;

    void readRegValues(QString line, uint8_t* values) {
        QStringList vals = line.split(" ");
        if (vals.size() != numSettings) {
            qDebug() << "invalid fmod data";
            exit(-1);
        }
        for (int i = 0; i < vals.size(); i++) {
            QStringList tokens = vals.at(i).split("h");
            if (tokens.size() != 2) {
                qDebug() << "invalid fmod number format";
                exit(-1);
            }
            bool ok;
            int regVal = tokens[1].toInt(&ok, 16);
            values[i] = regVal & 0xFF;
        }
    }
    void readRegValues_2(QString line, uint8_t* values_msb, uint8_t* values_lsb){
        QStringList vals = line.split(" ");
        if (vals.size() != numSettings) {
            qDebug() << "invalid fmod data";
            exit(-1);
        }
        for (int i = 0; i < vals.size(); i++) {
            QStringList tokens = vals.at(i).split("h");
            if (tokens.size() != 2) {
                qDebug() << "invalid fmod number format";
                exit(-1);
            }
            bool ok;
            int regVal = tokens[1].toInt(&ok, 16);
            values_msb[i] = (regVal >>8) & 0xFF;
            values_lsb[i] = regVal & 0xFF;
        }
    }
    void buildSettings() {
        int lineCnt = 0;
        int addressCnt = 0;
        uint16_t addresses[numRegs];
        uint8_t exck_freq_msb[numSettings];
        uint8_t exck_freq_lsb[numSettings];
        uint8_t pl_rc_vt[numSettings];
        uint8_t pl_rc_op[numSettings];
        uint8_t pl_fc_mx_msb[numSettings];
        uint8_t pl_fc_mx_lsb[numSettings];
        uint8_t pl_rc_mx[numSettings];
        uint8_t pl_res_mx[numSettings];
        uint8_t divselpre[numSettings];
        uint8_t divsel[numSettings];

        modSettings = new freqmod[numSettings];

        QFile fmodData("D:/fmodData.txt");
        if (!fmodData.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open fmod datasheet" << fmodData.errorString();
            exit(-1);
        }
        QTextStream in(&fmodData);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (lineCnt < 20) {
                //read address
                if (lineCnt == 2 || lineCnt == 3 || lineCnt == 7 ||
                        lineCnt == 10 || lineCnt == 12 || lineCnt == 13 ||
                        lineCnt == 15 || lineCnt > 16) {
                    int addr = strtol(line.toLocal8Bit().data(), NULL, 16);
                    qDebug() << "Fmod reg address = 0x" << hex << addr;
                    addresses[addressCnt++] = addr;
                }

            } else {
                //read values
                if (lineCnt == 21) {
                    readRegValues_2(line, exck_freq_msb, exck_freq_lsb);
                } else if (lineCnt == 24) {
                    readRegValues(line, pl_rc_vt);
                } else if (lineCnt == 27) {
                    readRegValues(line, pl_rc_op);
                } else if (lineCnt == 29) {
                    readRegValues_2(line, pl_fc_mx_msb, pl_fc_mx_lsb);
                } else if (lineCnt == 31) {
                    readRegValues(line, pl_rc_mx);
                } else if (lineCnt == 33) {
                    readRegValues(line, pl_res_mx);
                } else if (lineCnt == 34) {
                    readRegValues(line, divselpre);
                } else if (lineCnt == 35) {
                    readRegValues(line, divsel);
                }
            }
            lineCnt++;
        }

        if (lineCnt != 36) {
            qDebug() << "fmod datasheet corrupted";
            exit(-1);
        }

        for (int i = 0; i < numSettings; i++) {
            int regIdx = 0;
            freqmod* set = &modSettings[i];
            set->EXCLK_FREQ_MSB.addr = addresses[regIdx++];
            set->EXCLK_FREQ_MSB.val = exck_freq_msb[i];

            set->EXCLK_FREQ_LSB.addr = addresses[regIdx++];
            set->EXCLK_FREQ_LSB.val = exck_freq_lsb[i];

            set->PL_RC_VT.addr = addresses[regIdx++];
            set->PL_RC_VT.val = pl_rc_vt[i];

            set->PL_RC_OP.addr = addresses[regIdx++];
            set->PL_RC_OP.val = pl_rc_op[i];

            set->PL_FC_MX_MSB.addr = addresses[regIdx++];
            set->PL_FC_MX_MSB.val = pl_fc_mx_msb[i];

            set->PL_FC_MX_LSB.addr = addresses[regIdx++];
            set->PL_FC_MX_LSB.val = pl_fc_mx_lsb[i];

            set->PL_RC_MX.addr = addresses[regIdx++];
            set->PL_RC_MX.val = pl_rc_mx[i];

            set->PL_RES_MX.addr = addresses[regIdx++];
            set->PL_RES_MX.val = pl_res_mx[i];

            set->DIVSELPRE.addr = addresses[regIdx++];
            set->DIVSELPRE.val = divselpre[i];

            set->DIVSEL.addr = addresses[regIdx++];
            set->DIVSEL.val = divsel[i];
        }
    }
    void printFreqMod(freqmod* set) {
        qDebug()<< hex << set->EXCLK_FREQ_MSB.addr << " " << set->EXCLK_FREQ_MSB.val;
        qDebug()<< hex << set->EXCLK_FREQ_LSB.addr << " " << set->EXCLK_FREQ_LSB.val;
        qDebug()<< hex << set->PL_RC_VT.addr << " " << set->PL_RC_VT.val;
        qDebug()<< hex << set->PL_RC_OP.addr << " " << set->PL_RC_OP.val;
        qDebug()<< hex << set->PL_FC_MX_MSB.addr << " " << set->PL_FC_MX_MSB.val;
        qDebug()<< hex << set->PL_FC_MX_LSB.addr << " " << set->PL_FC_MX_LSB.val;
        qDebug()<< hex << set->PL_RC_MX.addr << " " << set->PL_RC_MX.val;
        qDebug()<< hex << set->PL_RES_MX.addr << " " << set->PL_RES_MX.val;
        qDebug()<< hex << set->DIVSELPRE.addr << " " << set->DIVSELPRE.val;
        qDebug()<< hex << set->DIVSEL.addr << " " << set->DIVSEL.val;
    }


};

#endif // FREQMOD_H
