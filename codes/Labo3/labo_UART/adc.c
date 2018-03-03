#include <xc.h>



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
