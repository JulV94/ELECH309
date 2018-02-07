#include "init.h"
#include "audio.h"
#include "lcd.h"
#include "clav.h"
#include "adc.h"
#include <xc.h>


int main(void)
{
	oscillatorInit();
    
    TRISDbits.TRISD7 = 1; //bouton RD7 en input
    TRISAbits.TRISA0 = 0; 

	while(1) {	
        if (!PORTDbits.RD7) //bouton appuyé (logique inversée)
		{
			LATAbits.LATA0 = 1; //led allumée
		}
		else
		{
			LATAbits.LATA0 = 0;
		}
	}
}
