#ifndef CONFIG_H
#define	CONFIG_H

#define FILTER_STAGE_COUNT 4
#define FILTER_COUNT 2
#define MAX_WINDOW_SIZE 12
#define THRESHOLD 256   // signal between 0 and 512

// Multiplier for float to int32_t
#define SHIFT 14
#define M (int32_t)(1 << SHIFT)

#define DEBUG

#endif	/* CONFIG_H */