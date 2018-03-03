#include <xc.h>


void audioInit(void)
{
    TRISBbits.TRISB2 = 0;
    LATBbits.LATB2 = 1;
    SPI1CON1bits.MODE16 = 1;        // mots de 16 bits
    SPI1CON1bits.PPRE = 1;          // primary prescaler = 16:1
    SPI1CON1bits.SPRE = 7;          // secondary prescaler = 1:1
    SPI1CON1bits.MSTEN = 1;         // mode master
    SPI1CON1bits.CKP = 1;           // on veut le mode (1,1), ...
    SPI1CON1bits.CKE = 0;           //  mais (CKE est l'inverse de CPHA)
    SPI1STATbits.SPIEN = 1;         // active le périphérique
}


void audioWrite(int sig)
{
    if (sig > 4095) {               // on sature la donnée
        sig = 4095;
    } else if (sig < 0) {
        sig = 0;
    }
    IFS0bits.SPI1IF = 0;            // on met le flag à 0 car on l'utilise dans la fonction
    LATBbits.LATB2 = 0;             // on active le Chip Select du DAC
    SPI1BUF = (0x3000|sig);         // on en voie la donnée
    while(!IFS0bits.SPI1IF);        // on attend que le transfert soit fini
    LATBbits.LATB2 = 1;             // on désactive le Chip Select
    SPI1BUF;                        // on lit la donnée "reçue", pour éviter une ereeur d'overrun.
}
