#include "init.h"
#include "adc.h"
#include <xc.h>
#include <string.h> // for memmove()

#define FILTER_STAGE_COUNT 3
#define FILTER_STAGE_ORDER 2
#define FILTER_COUNT 2

#define DEBUG

typedef struct filterStageData_s {
    float memory[FILTER_STAGE_ORDER];
    float numCoeff[FILTER_STAGE_ORDER + 1];
    float denCoeff[FILTER_STAGE_ORDER + 1];
    float gain;
} filterStageData_s;

void timer3Init()
{
    PR3 = 3076; // Freq 13kHz (40MHz PIC timer at every 3076 instructions -> 13kHz)
    T3CONbits.TCKPS = 0; // Prescaler at 1
    //T3CONbits.TON = 1; // Activation
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

void passband(float input, float* output, filterStageData_s stages[FILTER_STAGE_COUNT])
{
    int i,j; // Iterator variables
    float newMemory, acc; // Accumulator
    for (i=0; i<FILTER_STAGE_COUNT; i++)
    {
        newMemory = -1 * stages[i].denCoeff[0] * input;
        for (j=0;j<FILTER_STAGE_ORDER; j++)
        {
            newMemory -= stages[i].denCoeff[j+1] * stages[i].memory[j];
        }
        acc = stages[i].numCoeff[0] * newMemory;
        for (j=0;j<FILTER_STAGE_ORDER; j++)
        {
            acc += stages[i].numCoeff[j+1] * stages[i].memory[j];
        }
        // Apply output in input of next stage
        input = stages[i].gain * acc;
        // Shift the memory
        memmove(&stages[i].memory[1], &stages[i].memory[0], (FILTER_STAGE_ORDER - 1) * sizeof(stages[i].memory[0]));
        stages[i].memory[FILTER_STAGE_ORDER - 1] = newMemory;
    }
    output = &input;
}

int main(void)
{
    int i; // Iterator variable
	oscillatorInit();
    AD1PCFGL = 0xFFFF;
    
    // Init ADC1 on AN0
    timer3Init();
    adcTimerInit();
    
#ifdef DEBUG
    float debugOut[2][10];
    float debugMeans[2];
    int debugi;
#endif /* DEBUG */
    
    // Init UART1
    //UART1Init();
    //RPINR18bits.U1RXR = 6; // Configure RP6 as UART1 Rx
    //RPOR3bits.RP7R = 3;  // Configure RP7 as UART1 Tx
    
    // Init the passband filters [0]:900 / [1]:1100
    float input;
    float outputs[FILTER_COUNT];
    filterStageData_s stages[FILTER_COUNT][FILTER_STAGE_COUNT] = {
        { {{0.0}, {1,0,-1}, {1,-1.7964920647337039,0.98943434670312946}, 0.010365843540149258},
          {{0.0}, {1,0,-1}, {1,-1.8119895806833997,0.98983485086720957}, 0.010365843540149258},
          {{0.0}, {1,0,-1}, {1,-1.7950820896974879,0.9793743827745226}, 0.010312808612738712} },
          
        { {{0.0}, {1,0,-1}, {1,-1.7015520788691365,0.98710428574592357}, 0.012660500249294437},
          {{0.0}, {1,0,-1}, {1,-1.7241964095321776,0.98757631173112703}, 0.012660500249294437},
          {{0.0}, {1,0,-1}, {1,-1.702303529278439,0.97483677644146227}, 0.012581611779268901} }
    };
	
	while(1) {
        if (adcConversionFinished())
        {
            // signal needs to be processed
            input = (float)adcRead();
            // Send signal to each passband filter
            for (i=0; i<FILTER_COUNT;i++)
            {
                passband(input, &outputs[i], stages[i]);
            }
#ifdef DEBUG
            memmove(&debugOut[0][1], &debugOut[0][0], 9 * sizeof(float));
            memmove(&debugOut[1][1], &debugOut[0][0], 9 * sizeof(float));
            debugOut[0][0] = outputs[0];
            debugOut[1][0] = outputs[1];
            debugMeans[0] = 0;
            debugMeans[1] = 0;
            for (debugi=0;debugi<10; debugi++)
            {
                debugMeans[0] += debugOut[0][debugi]*debugOut[0][debugi];
                debugMeans[1] += debugOut[1][debugi]*debugOut[1][debugi];
            }
#endif /* DEBUG */
        }
	}
}
