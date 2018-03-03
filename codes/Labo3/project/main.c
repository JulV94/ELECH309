#include "init.h"
#include "adc.h"
#include <xc.h>

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

int main(void)
{
	oscillatorInit();
    AD1PCFGL = 0xFFFF;
    
    // Init ADC1 on AN0
    timer3Init();
    adcTimerInit();
    
    // Init UART1
    //UART1Init();
    //RPINR18bits.U1RXR = 6; // Configure RP6 as UART1 Rx
    //RPOR3bits.RP7R = 3;  // Configure RP7 as UART1 Tx
	
	while(1) {
        if (adcConversionFinished())
        {
            // signal needs to be processed
        }
	}
}
