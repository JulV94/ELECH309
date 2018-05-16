#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>    // For memset
#include "config.h"
#include "structs.h"
#include "FskDetector.h"

void processStage(filterStageData_s* stage, int32_t* input, int32_t* newMemory, int32_t* acc)
{
    *newMemory = stage->denCoeff[0] * *input - stage->denCoeff[1] * stage->memory[0] - stage->denCoeff[2] * stage->memory[1];
    *newMemory = *newMemory >> SHIFT;
    *acc = stage->numCoeff[0] * *newMemory + stage->numCoeff[1] * stage->memory[0] + stage->numCoeff[2] * stage->memory[1];
    // Apply output in input of next stage
    *acc = *acc >> SHIFT;
    *input = stage->gain * *acc;
    *input = *input >> SHIFT;
    // Shift the memory
    stage->memory[1] = stage->memory[0];
    stage->memory[0] = *newMemory;
}

int32_t passband(int32_t input, filterStageData_s stages[FILTER_STAGE_COUNT])
{
    int i; // Iterator variables
    int32_t newMemory, acc; // Accumulator
    for (i=0; i<FILTER_STAGE_COUNT; i++)
    {
        processStage(&stages[i], &input, &newMemory, &acc);
    }
    return input;
}

void pushToCircBuffer(int32_t value, maxCircBuffer_s *buffer)
{
    buffer->window[buffer->index] = value;
    if (buffer->index < MAX_WINDOW_SIZE)
    {
        buffer->index++;
    }
    else
    {
        buffer->index = 0;
    }
}

void updateCircBufferMax(maxCircBuffer_s *buffer)
{
    int i;
    buffer->max = 0;
    for (i=0; i<MAX_WINDOW_SIZE; i++)
    {
        if (buffer->window[i] > buffer->max)
        {
            buffer->max = buffer->window[i];
        }
    }
}

int toBinary(int32_t value)
{
    return (value > THRESHOLD);
}

int toFreq(int bit)
{
    if (bit)
    {
        return 1100;
    }
    return 900;
}

int main()
{
    int i, message, sample, bitCount=0, osrCount=0; // Iterator variable

    int frame[13] = {0,1,0,0,1,0,1,0,0,0,0,1,0};  // 0x250

    maxCircBuffer_s maxStructs[FILTER_COUNT];
    for (i=0; i<FILTER_COUNT; i++)
    {
        memset(maxStructs[i].window, 0, sizeof(maxStructs[i].window));
        maxStructs[i].index = 0;
    }

    // Init the passband filters [0]:900 / [1]:1100
    int32_t input, maxInput = 0;
    int32_t outputs[FILTER_COUNT];
    filterStageData_s stages[FILTER_COUNT][FILTER_STAGE_COUNT] = {
        { {{0}, {1*M,0*M,-1*M}, {1*M,-1.8492328819953079*M,0.99430893265693898*M}, 0.0073111832960681021*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.8593573389143425*M,0.99449985838090327*M}, 0.0073111832960681021*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.8449166157236199*M,0.98644899951001819*M}, 0.0072825057377408743*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.8491968712651323*M,0.98663848354046868*M}, 0.0072825057377408743*M} },

        { {{0}, {1*M,0*M,-1*M}, {1*M,-1.7777782573919603*M,0.99304925110422138*M}, 0.0089333802848647476*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7926618979622799*M,0.99327660434585519*M}, 0.0089333802848647476*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7736040019922288*M,0.98345970421132356*M}, 0.0088906476834830581*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7798575052955288*M,0.98368495232171638*M}, 0.0088906476834830581*M} }
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
        if (osrCount < OSR)
        {
            osrCount++;
        }
        else
        {
            osrCount = 0;
            bitCount++;
        }
        if (bitCount > 12)
        {
            bitCount = 12;
        }
        input = floor(((1 << ADC_RESOLUTION)-1) * (sin(2*M_PI*toFreq(frame[bitCount])*sample/SAMPLE_FREQ)+1)/2);

        for (i=0; i<FILTER_COUNT;i++)
        {
            outputs[i] = passband(input, stages[i]);
            pushToCircBuffer(outputs[i], &maxStructs[i]);
            updateCircBufferMax(&maxStructs[i]);
        }
        //printf("900 max bit %d\n 1100 max bit %d\n", toBinary(maxStructs[0].max), toBinary(maxStructs[1].max));
        message = fskDetector(toBinary(maxStructs[0].max), toBinary(maxStructs[1].max));
        //printf("message is %d\n", message);
        if (message != 0)
        {
            for (i=0; i<OSR; i++)
            {
                fskDetector(0, 0);
            }
                printf("message %#X is sent\n", message);
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

        if (maxInput < input)
        {
            maxInput = input;
        }
    }
    printf("Max input:\t%d\nMax out 1:\t%d\nMax out 2:\t%d\n\n", maxInput, maxStructs[0].max, maxStructs[1].max);
    fclose(fi);
    fclose(f1);
    fclose(f2);

    system("python /home/julien/git/ELECH309/passband_design/c_Test/plot.py");

    return 0;
}
