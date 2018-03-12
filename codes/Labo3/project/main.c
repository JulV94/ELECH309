#include "init.h"
#include "adc.h"
#include <xc.h>
#include <string.h> // for memmove()

#define FILTER_STAGE_COUNT 3
#define FILTER_STAGE_ORDER 2
#define FILTER_COUNT 2

// Multiplier for float to long
#define SHIFT 10
#define M (long)(1 << SHIFT)

//#define DEBUG
#define DEBUG2
#define DEBUG3
#include <libpic30.h>

#ifdef DEBUG
    long d_out[2][15];
    long d_max[2];
    int d_i;
#endif /* DEBUG */

typedef struct filterStageData_s {
    long memory[FILTER_STAGE_ORDER];
    long numCoeff[FILTER_STAGE_ORDER + 1];
    long denCoeff[FILTER_STAGE_ORDER + 1];
    long gain;
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

void passband(long input, long* output, filterStageData_s stages[FILTER_STAGE_COUNT])
{
    int i,j; // Iterator variables
    long newMemory, acc; // Accumulator
    for (i=0; i<FILTER_STAGE_COUNT; i++)
    {
        newMemory = stages[i].denCoeff[0] * input;
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
        acc = acc/(M*M);
        input = stages[i].gain * acc;
        input = input/M;
        // Shift the memory
        memmove(&stages[i].memory[1], &stages[i].memory[0], (FILTER_STAGE_ORDER - 1) * sizeof(stages[i].memory[0]));
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
    long input;
    long outputs[FILTER_COUNT];
    filterStageData_s stages[FILTER_COUNT][FILTER_STAGE_COUNT] = {
        { {{0}, {1*M,0*M,-1*M}, {1*M,-1.7964920647337039*M,0.98943434670312946*M}, 0.010365843540149258*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.8119895806833997*M,0.98983485086720957*M}, 0.010365843540149258*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7950820896974879*M,0.9793743827745226*M}, 0.010312808612738712*M} },

        { {{0}, {1*M,0*M,-1*M}, {1*M,-1.7015520788691365*M,0.98710428574592357*M}, 0.012660500249294437*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.7241964095321776*M,0.98757631173112703*M}, 0.012660500249294437*M},
          {{0}, {1*M,0*M,-1*M}, {1*M,-1.702303529278439*M,0.97483677644146227*M}, 0.012581611779268901*M} }
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
            input = (long)adcRead();
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
            memmove(&d_out[0][1], &d_out[0][0], 14 * sizeof(long));
            //memmove(&d_out[1][1], &d_out[0][0], 19 * sizeof(long));
            d_out[0][0] = outputs[0];
            //d_out[1][0] = outputs[1];
            d_max[0] = 0;
            //d_max[1] = 0;
            for (d_i=0;d_i<15; d_i++)
            {
                if (d_out[0][d_i] > d_max[0])
                {
                    d_max[0] = d_out[0][d_i];
                }
                /*if (d_out[1][d_i] > d_max[1])
                {
                    d_max[1] = d_out[1][d_i];
                }*/
            }
#endif /* DEBUG */
#ifdef DEBUG2
            LATAbits.LATA1 = 0;
#endif /* DEBUG2 */
        }
	}
}
