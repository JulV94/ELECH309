#include "init.h"
#include "audio.h"
#include "lcd.h"
#include "clav.h"
#include "adc.h"
#include <xc.h>

void _ISR _T3Interrupt(void) // Iterruption of timer 3 (timer 2 & 3 for us)
{
    IFS0bits.T3IF = 0;
    LATAbits.LATA1 = !LATAbits.LATA1;
}
int main(void)
{
	oscillatorInit();
    // Builtin LED config
    TRISDbits.TRISD7 = 1;
    TRISAbits.TRISA0 = 0;
    
    // External LED config
    TRISDbits.TRISD6 = 1;
    TRISCbits.TRISC4 = 0;
    
    // Timer 1 10kHz config
    PR1 = 2000; // Freq 20kHz (40MHz PIC timer at every 2000 instructions -> 20kHz)
    T1CONbits.TCKPS = 0; // Prescaler at 1
    T1CONbits.TON = 1; // Activation
    
    // Init RC1 as output
    TRISCbits.TRISC1 = 0;
    
    // Timer 2 & 3 (32 bits) at 1Hz
    PR2 = 20000000 & 0x0000FFFF;  // 16 last bits of 40e6 (mask)  (get 2Hz)
    PR3 = (20000000 & 0xFFFF0000) >> 16; // 16 first bits of 40e6 (mask) with 16 bits shift  (get 2Hz)
    T2CONbits.TCKPS = 0; // Prescaler at 1
    T3CONbits.TCKPS = 0; // Prescaler at 1
    T2CONbits.T32 = 1; // Activate 32 bits timer
    IEC0bits.T3IE = 1;  // Activation of interruption on timer 2 & 3
    T2CONbits.TON = 1; // Activation
    
    
    // Init RA1 as output
    TRISAbits.TRISA1 = 0;
    
	while(1) {
        // Builtin LED Toggle
        if (!PORTDbits.RD7)  // Inverted logic (button pressed)
        {
            LATAbits.LATA0 = 1; // LED on
        }
        else
        {
            LATAbits.LATA0 = 0; // LED off
        }
        
        // External LED Toggle
        LATCbits.LATC4 = !PORTDbits.RD6; // Inverted logic (LED on when button pressed)
        
        // Polling of timer 1
        if (IFS0bits.T1IF)
        {
            IFS0bits.T1IF = 0;
            LATCbits.LATC1 = !LATCbits.LATC1;
        }
        
        // Polling of timer 2 & 3 (32 bits)
        /*if (IFS0bits.T3IF)
        {
            IFS0bits.T3IF = 0;
            LATAbits.LATA1 = !LATAbits.LATA1;
        }*/
	}
}
