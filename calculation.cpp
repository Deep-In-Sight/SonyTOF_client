#include "fp_atan2_lut.h"
#include "evalkit_constants.h"
#include <math.h>
#include <QDebug>
#include <QDateTime>

#include "calculation.h"

typedef unsigned short uint16_t;

#define MAX_FILTER 10

uint16_t distance[640*480];
uint16_t amplitude[640*480];
uint16_t filter_tmp[MAX_FILTER][640*480];
uint16_t offset = 0;
double amplitude_tmp[640*480];

QDateTime* dtime = new QDateTime();

void filterImage(uint16_t* pImage, int nLoop, uint16_t **pFiltered);

void insertionSort(uint16_t* arr, int n)
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

//should be called getPhase()
void getDistance(uint16_t* pDCS, uint16_t** pDistance, bool filter) {
    int numPix = 640*480;
    int Q, I;

    qint64 start = dtime->currentMSecsSinceEpoch();

    for (int i = 0; i < numPix; i++) {
        int16_t dcs0 = pDCS[i] << 4;
        int16_t dcs2 = pDCS[i + numPix] << 4;
        int16_t dcs1 = pDCS[i + 2*numPix] << 4;
        int16_t dcs3 = pDCS[i + 3*numPix] << 4;

        Q = dcs3 - dcs1;
        I = dcs2 - dcs0;

//        distance[i] = (int16_t)((atan2_lut(Q, I) + MODULO_SHIFT_PI) % MAX_DIST_VALUE); //atan2 from lookup table
        distance[i] = (uint16_t) ((atan2(Q,I) + M_PI) / (2*M_PI) * ((1<<16) - 1));
        distance[i] += offset;
    }

    if (filter) {
        filterImage(distance, 3, pDistance);
    } else {
        *pDistance = distance;
    }

    qint64 end = dtime->currentMSecsSinceEpoch();
    qDebug() << __FUNCTION__ << " elapsed time: " << end - start << " ms";
}

void getAmplitude(uint16_t* pDCS, uint16_t** pAmplitude) {
    int numPix = 640*480;
    int Q, I;
    double A, maxA = 0;

    qint64 start = dtime->currentMSecsSinceEpoch();

    int shift = 4;
    for (int i = 0; i < numPix; i++) {
        //sensor data is 12bit signed integer =>> shift left 4 bit to expand to 16bit signed
        int16_t dcs0 = pDCS[i] << shift;
        int16_t dcs2 = pDCS[i + numPix] << shift;
        int16_t dcs1 = pDCS[i + 2*numPix] << shift;
        int16_t dcs3 = pDCS[i + 3*numPix] << shift;

        Q = dcs3 - dcs1;
        I = dcs2 - dcs0;
        A = 0.5 * sqrt (Q*Q + I*I);

        if (A < 256)
            amplitude[i] = A;
        else
            amplitude[i] = 255;
//        amplitude_tmp[i] = A;
//        if (A > maxA) maxA = A;
    }

//    for (int i = 0; i < numPix; i++) {
//        amplitude[i] = (uint16_t) (amplitude_tmp[i]/maxA * 255);
//    }
//    qDebug() << "Amplitude scale " << scale << " bits";
    *pAmplitude = amplitude;

    qint64 end = dtime->currentMSecsSinceEpoch();
    qDebug() << __FUNCTION__ << " elapsed time: " << end - start << " ms";
}

void filterImage(uint16_t* pImage, int nLoop, uint16_t **pFiltered) {
    uint16_t window[9];
    uint16_t* pInput = NULL;
    uint16_t* pOutput = NULL;

    for (int i = 0; i < nLoop; i++) {
        for (int y = 0; y < 480; y++) {
            for (int x = 0; x < 640; x++) {
                int pos = y*640+x;
                pOutput = filter_tmp[i];
                if (i == 0)
                    pInput = pImage;
                else
                    pInput = filter_tmp[i-1];

                if (x == 0 || y == 0 || x == 640-1 || y == 480-1) {
                    pOutput[pos] = pInput[pos];
                } else {
                    for (int ky = 0; ky < 3; ky++){
                        for (int kx = 0; kx < 3; kx++) {
                             int wpos = ky*3 + kx;
                             window[wpos] = pInput[(y+ky-1)*640+x+kx-1];
                        }
                    }
                    insertionSort(window,9);
                    pOutput[pos] = window[4];
                }
            }
        }
    }

    *pFiltered = pOutput;
}

void setPhaseOffset(uint16_t phaseOffset) {
    offset = phaseOffset;
}
