#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FILTER_STAGE_COUNT 4
#define FILTER_STAGE_ORDER 2
#define FILTER_COUNT 2

// Multiplier for float to int32_t
#define SHIFT 7
#define M (int32_t)(1 << SHIFT)

#define INPUT_FREQ 900.0
#define MAX_SAMPLE 1000
#define SAMPLE_FREQ 13000.0
#define ADC_RESOLUTION 10

typedef struct filterStageData_s {
    int32_t memory[FILTER_STAGE_ORDER];
    int32_t numCoeff[FILTER_STAGE_ORDER + 1];
    int32_t denCoeff[FILTER_STAGE_ORDER + 1];
    int32_t gain;
} filterStageData_s;

void passband(int32_t input, int32_t* output, filterStageData_s stages[FILTER_STAGE_COUNT])
{
    int i,j; // Iterator variables
    int32_t newMemory, acc; // Accumulator
    for (i=0; i<FILTER_STAGE_COUNT; i++)
    {
        newMemory = stages[i].denCoeff[0] * input * M;
        for (j=0;j<FILTER_STAGE_ORDER; j++)
        {
            newMemory -= stages[i].denCoeff[j+1] * stages[i].memory[j];
        }

        newMemory = newMemory >> SHIFT;
        acc = stages[i].numCoeff[0] * newMemory;
        for (j=0;j<FILTER_STAGE_ORDER; j++)
        {
            acc += stages[i].numCoeff[j+1] * stages[i].memory[j];
        }
        // Apply output in input of next stage
        acc = acc >> 2*SHIFT;
        input = stages[i].gain * acc;
        //printf("Before shift : %#04x\n", input);
        input = input >> SHIFT;
        //printf("After shift : %#04x\n\n", input);
        // Shift the memory
        for (j=FILTER_STAGE_ORDER-1;j>0; j--)
        {
            stages[i].memory[j] = stages[i].memory[j-1];
        }
        stages[i].memory[0] = newMemory;
    }
    *output = input;
}

int main()
{
    int i, sample; // Iterator variable

    // Init the passband filters [0]:900 / [1]:1100
    int32_t input, max[3] = {0};
    int32_t outputs[FILTER_COUNT];
    filterStageData_s stages[FILTER_COUNT][FILTER_STAGE_COUNT] = {
        { {{0}, {1*M,0*M,-1*M}, {1*M,-1.8011571239155957*M,0.99343663965814499*M}, 0.0084342754131484891*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.8144890622013676*M,0.99365313679164347*M}, 0.0084342754131484891*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7968934937505234*M,0.98437870027614971*M}, 0.0083961616899833683*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.802504614864014*M,0.98459330636028497*M}, 0.0083961616899833683*M} },

        { {{0}, {1*M,0*M,-1*M}, {1*M,-1.7072742575792748*M,0.99198489677340751*M}, 0.010305881103809989*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7268035972665099*M,0.9922402806273074*M}, 0.010305881103809989*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7035572899577904*M,0.98093595942427547*M}, 0.01024910202670197*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7117294492219726*M,0.98118861143834546*M}, 0.01024910202670197*M} }
    };

    FILE* fi = fopen("filter_input.json", "w");
    FILE* f1 = fopen("filter_1.json", "w");
    FILE* f2 = fopen("filter_2.json", "w");
    if (fi == NULL || f1 == NULL || f2 == NULL)
    {
        printf("Cannot access specified file!\nExiting...\n");
        exit(1);
    }
    fprintf(fi, "[");
    fprintf(f1, "[");
    fprintf(f2, "[");

    for (sample=0; sample<MAX_SAMPLE; sample++)
    {
        input = floor(((1 << ADC_RESOLUTION)-1) * (sin(2*M_PI*INPUT_FREQ*sample/SAMPLE_FREQ)+1)/2);

        for (i=0; i<FILTER_COUNT;i++)
        {
            passband(input, &outputs[i], stages[i]);
        }

        if (sample == MAX_SAMPLE-1)
        {
            fprintf(fi, "%d]", input);
            fprintf(f1, "%d]", outputs[0]);
            fprintf(f2, "%d]", outputs[1]);
        }
        else
        {
            fprintf(fi, "%d,", input);
            fprintf(f1, "%d,", outputs[0]);
            fprintf(f2, "%d,", outputs[1]);
        }

        if (max[0] < input)
        {
            max[0] = input;
        }
        if (max[1] < outputs[0])
        {
            max[1] = outputs[0];
        }
        if (max[2] < outputs[1])
        {
            max[2] = outputs[1];
        }
    }
    printf("Max input:\t%d\nMax out 1:\t%d\nMax out 2:\t%d\n\n", max[0], max[1], max[2]);
    fclose(fi);
    fclose(f1);
    fclose(f2);

    system("python /home/julien/git/ELECH309/passband_design/c_Test/plot.py");

    return 0;
}
