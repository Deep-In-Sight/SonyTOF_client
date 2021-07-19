#include "fp_atan2_lut.h"
#include "evalkit_constants.h"
#include <math.h>
#include <QDebug>
#include <QDateTime>

#include "calculation.h"

typedef unsigned short uint16_t;

#define MAX_FILTER 10
#define NUMPIX (640*480)
uint16_t distance[NUMPIX];
uint16_t amplitude[NUMPIX];
uint16_t filter_tmp[MAX_FILTER][NUMPIX];
uint16_t offset = 0;
double amplitude_tmp[NUMPIX];

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
void getDistance(uint16_t* pData, uint16_t** pDistance, bool filter) {
//    qint64 start = dtime->currentMSecsSinceEpoch();

    if (filter) {
        filterImage(pData, 3, pDistance);
    } else {
        *pDistance = pData;
    }

//    qint64 end = dtime->currentMSecsSinceEpoch();
//    qDebug() << __FUNCTION__ << " elapsed time: " << end - start << " ms";
}

void getAmplitude(uint16_t* pData, uint16_t** pAmplitude) {
//    qint64 start = dtime->currentMSecsSinceEpoch();
    int max = 0;

    *pAmplitude = pData;

//    qint64 end = dtime->currentMSecsSinceEpoch();
//    qDebug() << __FUNCTION__ << " elapsed time: " << end - start << " ms";
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
