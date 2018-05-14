#ifndef STRUCTS_H
#define	STRUCTS_H

#include "config.h"

typedef struct filterStageData_s {
    int32_t memory[FILTER_STAGE_ORDER];
    int32_t numCoeff[FILTER_STAGE_ORDER + 1];
    int32_t denCoeff[FILTER_STAGE_ORDER + 1];
    int32_t gain;
} filterStageData_s;

typedef struct maxCircBuffer_s {
    int32_t window[MAX_WINDOW_SIZE];
    int32_t max;
    int index;
} maxCircBuffer_s;


#endif	/* STRUCTS_H */
