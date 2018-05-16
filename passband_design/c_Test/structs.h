#ifndef STRUCTS_H
#define	STRUCTS_H

#include "config.h"

typedef struct filterStageData_s {
    int32_t memory[3];
    int32_t numCoeff[3];
    int32_t denCoeff[3];
    int32_t gain;
} filterStageData_s;

typedef struct maxCircBuffer_s {
    int32_t window[MAX_WINDOW_SIZE];
    int32_t max;
    int index;
} maxCircBuffer_s;


#endif	/* STRUCTS_H */
