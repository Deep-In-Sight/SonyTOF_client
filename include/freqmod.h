#ifndef FREQMOD_H
#define FREQMOD_H

#include <QFile>
#include <QDebug>
#include <string.h>

class i2cReg {
public:
    uint16_t addr;
    uint8_t val;
};

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
        qDebug() << "Building fmod\n";
        buildSettings();
    }

    ~modSetting() {
        qDebug() << "Dumping fmod\n";
        dump_modSettings();
        delete modSettings;
    }

    //return the setting for a modulation frequency
    void getSetting(int freq, QList<i2cReg> &regList) {
        if (freq < minFreq || freq > maxFreq) {
            return;
        } else {
            freqmod* set = &modSettings[maxFreq-freq];
            printFreqMod(set);
            regList.clear();
            regList.append(set->EXCLK_FREQ_MSB);
            regList.append(set->EXCLK_FREQ_LSB);
            regList.append(set->PL_RC_VT);
            regList.append(set->PL_RC_OP);
            regList.append(set->PL_FC_MX_MSB);
            regList.append(set->PL_FC_MX_LSB);
            regList.append(set->PL_RC_MX);
            regList.append(set->PL_RES_MX);
            regList.append(set->DIVSELPRE);
            regList.append(set->DIVSEL);
        }
    }

private:
    const int minFreq = 4;
    const int maxFreq = 100;
    const int numRegs = 10;
    const int numSettings = maxFreq - minFreq + 1;
    freqmod* modSettings;

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

        QFile fmodData(":/data/fmodData.txt");
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
                    qDebug() << "Fmod reg address = 0x" << QString::number(addr, 16);
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

    void printReg(i2cReg& r) {
        qDebug() << QString::number(r.addr, 16) <<
                    " " << QString::number(r.val, 16);
    }

    void printFreqMod(freqmod* set) {
        printReg(set->EXCLK_FREQ_MSB);
        printReg(set->EXCLK_FREQ_LSB);
        printReg(set->PL_RC_VT);
        printReg(set->PL_RC_OP);
        printReg(set->PL_FC_MX_MSB);
        printReg(set->PL_FC_MX_LSB);
        printReg(set->PL_RC_MX);
        printReg(set->PL_RES_MX);
        printReg(set->DIVSELPRE);
        printReg(set->DIVSEL);
    }

    void dump_modSettings() {
        QFile file("freqmod.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);

        for (int i = 0; i < numSettings; i++) {
            freqmod* set = &modSettings[i];

            out << set->EXCLK_FREQ_MSB.addr << " " << set->EXCLK_FREQ_MSB.val << "\n";

            out << set->EXCLK_FREQ_LSB.addr << " " << set->EXCLK_FREQ_LSB.val << "\n";

            out << set->PL_RC_VT.addr << " " << set->PL_RC_VT.val << "\n";

            out << set->PL_RC_OP.addr << " " << set->PL_RC_OP.val << "\n";

            out << set->PL_FC_MX_MSB.addr << " " << set->PL_FC_MX_MSB.val << "\n";

            out << set->PL_FC_MX_LSB.addr << " " << set->PL_FC_MX_LSB.val << "\n";

            out << set->PL_RC_MX.addr << " " << set->PL_RC_MX.val << "\n";

            out << set->PL_RES_MX.addr << " " << set->PL_RES_MX.val << "\n";

            out << set->DIVSELPRE.addr << " " << set->DIVSELPRE.val << "\n";

            out << set->DIVSEL.addr << " " << set->DIVSEL.val << "\n";
        }
    }
};

#endif // FREQMOD_H
