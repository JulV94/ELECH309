#include "init.h"
#include "adc.h"
#include <xc.h>
#include <string.h> // for memmove()

#define FILTER_STAGE_COUNT 4
#define FILTER_STAGE_ORDER 2
#define FILTER_COUNT 2

// Multiplier for float to int32_t
#define SHIFT 8
#define M (int32_t)(1 << SHIFT)

#define DEBUG
#define DEBUG2
//#define DEBUG3
#include <libpic30.h>

#ifdef DEBUG
    int32_t d_out[2][14];
    int32_t d_max[2];
    int d_i;
#endif /* DEBUG */

typedef struct filterStageData_s {
    int32_t memory[FILTER_STAGE_ORDER];
    int32_t numCoeff[FILTER_STAGE_ORDER + 1];
    int32_t denCoeff[FILTER_STAGE_ORDER + 1];
    int32_t gain;
} filterStageData_s;

void timer3Init()
{
    PR3 = 3076; // Freq 13kHz (40MHz PIC timer at every 3076 instructions -> 13kHz)
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
        input = input >> SHIFT;
        // Shift the memory
        for (j=FILTER_STAGE_ORDER-1;j>0; j--)
        {
            stages[i].memory[j] = stages[i].memory[j-1];
        }
        stages[i].memory[0] = newMemory;
    }
    *output = input;
}

int main(void)
{
    int i; // Iterator variable
	oscillatorInit();
    AD1PCFGL = 0xFFFF;
    
    // Init ADC1 on AN0
    timer3Init();
    adcTimerInit();
    
#ifdef DEBUG2
    TRISAbits.TRISA1 = 0;
#endif /* DEBUG2 */
    
#ifdef DEBUG3    
    // Init UART1
    UART1Init();
    RPINR18bits.U1RXR = 6; // Configure RP6 as UART1 Rx
    RPOR3bits.RP7R = 3;  // Configure RP7 as UART1 Tx
    char rxReg;
    int osc = 0;
    int oscCountSuper = 0;
    int dataOsc[3][1000];
    int iosc;
#endif /* DEBUG3 */
    
    // Init the passband filters [0]:900 / [1]:1100
    int32_t input;
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
	
	while(1) {
#ifdef DEBUG3
        if (U1STAbits.URXDA)  // A message is in the receiver buffer
        {
            rxReg = U1RXREG;
            if (rxReg == 's')
            {
                // send the samples to oscilloscope
                osc = 1;
            }
            else
            {
                U1TXREG = rxReg; // Echo test
            }
        }
#endif /* DEBUG3 */
        if (adcConversionFinished())
        {
#ifdef DEBUG2
            LATAbits.LATA1 = 1;
#endif /* DEBUG2 */
            // signal needs to be processed
            input = (int32_t)adcRead();
            // Send signal to each passband filter
            for (i=0; i<FILTER_COUNT;i++)
            {
                passband(input, &outputs[i], stages[i]);
            }
#ifdef DEBUG3
            if (osc == 1)
            {
                dataOsc[0][oscCountSuper] = input;
                dataOsc[1][oscCountSuper] = outputs[0];
                dataOsc[2][oscCountSuper] = outputs[1];
                oscCountSuper++;
                if (oscCountSuper >= 1000)
                {
                    osc = 2;
                }
            }
            if (osc == 2)
            {
                for (iosc=0; iosc<1000; iosc++)
                {
                    U1TXREG = dataOsc[0][iosc]/4;
                    U1TXREG = dataOsc[1][iosc]/4;
                    U1TXREG = dataOsc[2][iosc]/4;
                    __delay_ms(10);
                }
            }
#endif /* DEBUG3 */
#ifdef DEBUG
            /*memmove(&d_out[0][1], &d_out[0][0], 13 * sizeof(int32_t));
            //memmove(&d_out[1][1], &d_out[0][0], 19 * sizeof(int32_t));
            d_out[0][0] = outputs[0];
            //d_out[1][0] = outputs[1];
            d_max[0] = 0;
            //d_max[1] = 0;
            for (d_i=0;d_i<14; d_i++)
            {
                if (d_out[0][d_i] > d_max[0])
                {
                    d_max[0] = d_out[0][d_i];
                }
                /*if (d_out[1][d_i] > d_max[1])
                {
                    d_max[1] = d_out[1][d_i];
                }*/
            //}
            if (d_max[0] < outputs[0])
            {
                d_max[0] = outputs[0];
            }
#endif /* DEBUG */
#ifdef DEBUG2
            LATAbits.LATA1 = 0;
#endif /* DEBUG2 */
        }
	}
}
