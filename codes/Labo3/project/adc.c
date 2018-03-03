#include <xc.h>


void adcTimerInit(void)
{
    AD1CON1bits.AD12B = 0;  // Convertisseur sur 10 bits
    AD1CON3bits.ADCS = 5;   // Clock de l'adc
    AD1CON1bits.ASAM = 1;   // auto sample activé
    AD1CSSLbits.CSS0 = 1;   // Le convertisseur doit scanner la patte AN0
    AD1PCFGLbits.PCFG0 = 0; // AN0 en mode analogique
    AD1CON1bits.SSRC = 2;   // ADC activé par le débordement du Timer3
    AD1CON1bits.ADON = 1;   // l'ADC est actif
}

void adcPollingStart(void)
{
    AD1CON1bits.SAMP = 0;
}


int adcConversionFinished(void)
{
    return (AD1CON1bits.DONE);
}


int adcRead(void)
{
    if (AD1CON1bits.DONE) {
        AD1CON1bits.DONE = 0;
        return ADC1BUF0;
    } else {
        return (-32768);
    }
}
