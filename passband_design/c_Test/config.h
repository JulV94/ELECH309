#ifndef CONFIG_H
#define	CONFIG_H

#define FILTER_STAGE_COUNT 4
#define FILTER_COUNT 2
#define MAX_WINDOW_SIZE 12
#define THRESHOLD 200

// Multiplier for float to int32_t
#define SHIFT 13
#define M (int32_t)(1 << SHIFT)

#define DEBUG2
//#define DEBUG3

#define INPUT_FREQ 900.0
#define MAX_SAMPLE 20000
#define SAMPLE_FREQ 15000.0
#define ADC_RESOLUTION 10

#endif	/* CONFIG_H */
