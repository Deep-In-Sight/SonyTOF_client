#ifndef CALCULATION_H
#define CALCULATION_H

void getDistance(uint16_t* pDCS, uint16_t** pDistance, bool filter);
void getAmplitude(uint16_t* pDCS, uint16_t** pAmplitude);
void setPhaseOffset(uint16_t phaseOffset);

#endif // CALCULATION_H
