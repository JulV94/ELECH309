#ifndef CONFIG_H
#define	CONFIG_H

#define FILTER_STAGE_COUNT 4
#define FILTER_COUNT 2
#define MAX_WINDOW_SIZE 12
#define THRESHOLD 512

// Multiplier for float to int32_t
#define SHIFT 14
#define M (int32_t)(1 << SHIFT)

#define DEBUG2
//#define DEBUG3

#define INPUT_FREQ 1100.0
#define MAX_SAMPLE 1500
#define SAMPLE_FREQ 15000.0
#define ADC_RESOLUTION 10

#endif	/* CONFIG_H */