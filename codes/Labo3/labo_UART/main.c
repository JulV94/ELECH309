#include "init.h"
#include "adc.h"
#include <xc.h>

void adcTimerInit()
{
    PR3 = 8000; // Freq 5kHz (40MHz PIC timer at every 8000 instructions -> 5kHz)
    T3CONbits.TCKPS = 0; // Prescaler at 1
    //T3CONbits.TON = 1; // Activation
    
    AD1CON1bits.AD12B = 0;  // Convertisseur sur 10 bits
    AD1CON3bits.ADCS = 5;   // Clock de l'adc
    AD1CON1bits.ASAM = 1;   // auto sample activé
    AD1CSSLbits.CSS0 = 1;   // Le convertisseur doit scanner la patte AN0
    AD1PCFGLbits.PCFG0 = 0; // AN0 en mode analogique
    AD1CON1bits.SSRC = 2;   // ADC activé manuellement (en software) (use of the Timer 3)
    AD1CON1bits.ADON = 1;   // l'ADC est actif
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
    adcTimerInit();
    
    int i = 0;
    
    // Init UART1
    UART1Init();
    RPINR18bits.U1RXR = 6; // Configure RP6 as UART1 Rx
    RPOR3bits.RP7R = 3;  // Configure RP7 as UART1 Tx
    
    char rxReg;
    
	while(1) {
        if (U1STAbits.URXDA)  // A message is in the receiver buffer
        {
            rxReg = U1RXREG;
            if (rxReg == 's')
            {
                T3CONbits.TON = 1; // Activation of timer 3 -> start sampling
            }
            else
            {
                U1TXREG = rxReg; // Echo test
            }
        }
        if (adcConversionFinished())
        {
            U1TXREG = adcRead()/4;  // Store the sample
            i++;
            if (i>1000)
            {
                i=0;
                T3CONbits.TON = 0; // Deactivation of timer 3 -> stop sampling
            }
        }
	}
}
