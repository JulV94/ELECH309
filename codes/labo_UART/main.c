#include "init.h"
#include "audio.h"
#include "lcd.h"
#include "clav.h"
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

int main(void)
{
	oscillatorInit();
    AD1PCFGL = 0xFFFF;
    
    // Init ADC1 on AN0
    adcTimerInit();
    
	while(1) {
        if (adcConversionFinished())
        {
            // store the point
        }
	}
}
