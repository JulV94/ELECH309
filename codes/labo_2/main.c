#include "init.h"
#include "audio.h"
#include "lcd.h"
#include "clav.h"
#include "adc.h"
#include <xc.h>
#include <math.h>
#include <libpic30.h>

#define PI 3.14159

float wave[100];
int j=0;
float ampl=0;

void genWave(int ampl)
{
    int i;
    for (i=0; i<100; i++)
    {
        wave[i] = ampl*sin(i*2*PI/100);
    }
}

void _ISR _T1Interrupt(void) // Iterruption of timer 3 (timer 2 & 3 for us)
{
    IFS0bits.T1IF = 0;
    audioWrite((ampl/2000.0f)*wave[j]+2048);
    j++;
    if (j>100)
    {
        j=0;
    }
}

void adcTimerInit()
{
    PR3 = 40000; // Freq 1kHz (40MHz PIC timer at every 40000 instructions -> 1kHz)
    T3CONbits.TCKPS = 0; // Prescaler at 1
    T3CONbits.TON = 1; // Activation
    
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
    
    genWave(2000);
    audioInit();
    clavInit();
    
    PR1 = 2000; // Freq 20kHz (40MHz PIC timer at every 2000 instructions -> 20kHz)
    T1CONbits.TCKPS = 0; // Prescaler at 1
    IEC0bits.T1IE = 1;  // Activate interrupt on timer 1
    T1CONbits.TON = 1; // Activation
    
	while(1) {
        if (adcConversionFinished())
        {
            // Define amplitude
            ampl = adcRead()+1024;
        }
        if (readClav() == 'A')
        {
            T1CONbits.TON = !T1CONbits.TON;
            __delay_ms(300);
        }
	}
}
