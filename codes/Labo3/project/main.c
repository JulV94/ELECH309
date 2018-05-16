#include <xc.h>
#include <libpic30.h>
#include <string.h>    // For memset
#include "config.h"
#include "init.h"
#include "adc.h"
#include "structs.h"
#include "FskDetector.h"

void timer3Init()
{
    PR3 = 2667; // Freq 15kHz (40MHz PIC timer at every 2667 instructions -> 15kHz)
    T3CONbits.TCKPS = 0; // Prescaler at 1
    T3CONbits.TON = 1; // Activation
}

void UART1Init()
{
    U1BRG = 21;  // baudrate of 115200
    U1MODEbits.PDSEL = 0; // 8 bits no parity
    U1MODEbits.STSEL = 0; // 1 stop bit
    U1MODEbits.URXINV = 0; // Idle state is high state
    U1MODEbits.BRGH = 0; // Standard mode (not high baudrate)
    U1MODEbits.UARTEN = 1; // Activate UART
    U1STAbits.UTXEN = 1;  // activate transmission
}

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

int main(void)
{
    int i, message; // Iterator variable
	oscillatorInit();
    AD1PCFGL = 0xFFFF;

    // Init ADC1 on AN0
    timer3Init();
    adcTimerInit();

    maxCircBuffer_s maxStructs[FILTER_COUNT];
    for (i=0; i<FILTER_COUNT; i++)
    {
        memset(maxStructs[i].window, 0, sizeof(maxStructs[i].window));
        maxStructs[i].max = 0;
        maxStructs[i].index = 0;
    }

    // Init UART1
    UART1Init();
    RPINR18bits.U1RXR = 6; // Configure RP6 as UART1 Rx
    RPOR3bits.RP7R = 3;  // Configure RP7 as UART1 Tx

#ifdef DEBUG
    TRISAbits.TRISA1 = 0;
#endif /* DEBUG */

    // Init the passband filters [0]:900 / [1]:1100
    int32_t input;
    int32_t outputs[FILTER_COUNT];
    filterStageData_s stages[FILTER_COUNT][FILTER_STAGE_COUNT] = {
        { {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.8492328819953079*M,0.99430893265693898*M}, 0.0073111832960681021*M},
          {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.8593573389143425*M,0.99449985838090327*M}, 0.0073111832960681021*M},
          {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.8449166157236199*M,0.98644899951001819*M}, 0.0072825057377408743*M},
          {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.8491968712651323*M,0.98663848354046868*M}, 0.0072825057377408743*M} },

        { {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.7777782573919603*M,0.99304925110422138*M}, 0.0089333802848647476*M},
          {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.7926618979622799*M,0.99327660434585519*M}, 0.0089333802848647476*M},
          {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.7736040019922288*M,0.98345970421132356*M}, 0.0088906476834830581*M},
          {{0,0}, {1*M,0*M,-1*M}, {1*M,-1.7798575052955288*M,0.98368495232171638*M}, 0.0088906476834830581*M} }
    };

	while(1) {
        if (adcConversionFinished())
        {
#ifdef DEBUG
            LATAbits.LATA1 = 1;
#endif /* DEBUG */
            // signal needs to be processed
            input = (int32_t)adcRead();
            // Send signal to each passband filter
            for (i=0; i<FILTER_COUNT;i++)
            {
                outputs[i] = passband(input, stages[i]);
                pushToCircBuffer(outputs[i], &maxStructs[i]);
                updateCircBufferMax(&maxStructs[i]);
            }
            message = fskDetector(toBinary(maxStructs[0].max), toBinary(maxStructs[1].max));
            if (message != 0)
            {
                for (i=0; i<OSR; i++)
                {
                    fskDetector(0, 0);
                }
                U1TXREG = (char)(message >> 8);  // Order
                U1TXREG = (char)(message & 0b11111111);   // parameter
            }
#ifdef DEBUG
            LATAbits.LATA1 = 0;
#endif /* DEBUG */
        }
	}
}
